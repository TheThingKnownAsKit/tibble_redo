#include "control/command_center.hpp"

using namespace std::chrono_literals; // lets things like ms register as milliseconds

namespace command_center
{

    int button_name_to_index(const std::string &name)
    {
        // Mappings can be found under the game_controller_node index map https://index.ros.org/p/joy/
        // This is specifically for XBox style controllers, but there are PS options
        static const std::unordered_map<std::string, int> map = {
            {"A", 0},
            {"B", 1},
            {"X", 2},
            {"Y", 3},
            {"BACK", 4},
            {"GUIDE", 5},
            {"START", 6},
            {"LEFTSTICK", 7},
            {"RIGHTSTICK", 8},
            {"LEFTSHOULDER", 9},
            {"RIGHTSHOULDER", 10},
            {"DPAD_UP", 11},
            {"DPAD_DOWN", 12},
            {"DPAD_LEFT", 13},
            {"DPAD_RIGHT", 14},
            {"MISC1", 15},
            {"PADDLE1", 16},
            {"PADDLE2", 17},
            {"PADDLE3", 18},
            {"PADDLE4", 19},
            {"TOUCHPAD", 20},
        };

        auto it = map.find(name);
        if (it != map.end())
            return it->second;

        throw std::runtime_error("Unknown button: " + name);
    }

    int axis_name_to_index(const std::string &name)
    {
        // Mappings can be found under the game_controller_node index map https://index.ros.org/p/joy/
        // This is specifically for XBox style controllers, but there are PS options
        static const std::unordered_map<std::string, int> map = {
            {"LEFTX", 0},
            {"LEFTY", 1},
            {"RIGHTX", 2},
            {"RIGHTY", 3},
            {"TRIGGERLEFT", 4},
            {"TRIGGERRIGHT", 5},
        };

        auto it = map.find(name);
        if (it != map.end())
            return it->second;

        throw std::runtime_error("Unknown axes: " + name);
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_configure(const rclcpp_lifecycle::State &)
    {
        // TODO: Map param strings to index numbers because of error
        modes_enable_b_ = button_name_to_index(this->get_parameter("enable_b").as_string());
        modes_panic_b_ = button_name_to_index(this->get_parameter("panic_b").as_string());
        modes_state_machine_b_ = button_name_to_index(this->get_parameter("state_machine_b").as_string());
        modes_manual_b_ = button_name_to_index(this->get_parameter("manual_b").as_string());
        modes_autonomy_b_ = button_name_to_index(this->get_parameter("autonomy_b").as_string());
        sm_idle_b_ = button_name_to_index(this->get_parameter("idle_b").as_string());
        sm_travel_b_ = button_name_to_index(this->get_parameter("travel_b").as_string());
        sm_excavate_b_ = button_name_to_index(this->get_parameter("excavate_b").as_string());
        sm_deposit_b_ = button_name_to_index(this->get_parameter("deposit_b").as_string());
        manual_la_extend_b_ = button_name_to_index(this->get_parameter("la_extend_b").as_string());
        manual_la_retract_b_ = button_name_to_index(this->get_parameter("la_retract_b").as_string());
        manual_latch_toggle_b_ = button_name_to_index(this->get_parameter("latch_toggle_b").as_string());
        manual_vibe_toggle_b_ = button_name_to_index(this->get_parameter("vibe_toggle_b").as_string());
        manual_excav_axis_ = axis_name_to_index(this->get_parameter("excav_axis").as_string());

        manual_command_pub_ = this->create_publisher<interfaces::msg::ManualCommands>("/manual_commands", rclcpp::SystemDefaultsQoS());
        sm_command_pub_ = this->create_publisher<interfaces::msg::StateMachineCommands>("/state_machine_commands", rclcpp::SystemDefaultsQoS());
        // Maybe one for autonomy pub????

        change_state_service_ = this->create_service<interfaces::srv::ChangeState>(
            "/change_state",
            std::bind(&CommandCenter::change_state_callback, this, std::placeholders::_1, std::placeholders::_2));

        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "/joy", rclcpp::SensorDataQoS(),
            [this](const sensor_msgs::msg::Joy::SharedPtr msg)
            {
                joy_cmd_buffer_.writeFromNonRT(*msg);
            });

        // Psuedo realtime control/update function. Updates at 50Hz
        timer_ = this->create_wall_timer(50ms, std::bind(&CommandCenter::control_loop, this));

        RCLCPP_INFO(this->get_logger(), "Configured CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_activate(const rclcpp_lifecycle::State &state)
    {
        current_state_.store(ControlState::PANIC);

        // Set everything to safe defaults
        manual_msg_.toggle_latch = true; // closed
        manual_msg_.toggle_vibe = false; // off
        manual_msg_.excavate_cmd = 0.0;
        manual_msg_.la_cmd = 0.0;
        sm_msg_.state = interfaces::msg::StateMachineCommands::IDLE;

        manual_command_pub_->on_activate();
        sm_command_pub_->on_activate();
        LifecycleNode::on_activate(state);

        RCLCPP_INFO(this->get_logger(), "Activated CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_deactivate(const rclcpp_lifecycle::State &state)
    {
        current_state_.store(ControlState::PANIC);

        manual_command_pub_->on_deactivate();
        sm_command_pub_->on_deactivate();
        LifecycleNode::on_deactivate(state);

        RCLCPP_INFO(this->get_logger(), "Deactivated CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_cleanup(const rclcpp_lifecycle::State &)
    {
        manual_command_pub_.reset();
        sm_command_pub_.reset();
        joy_cmd_buffer_.reset();
        timer_.reset();

        // Set everything to safe defaults
        manual_msg_.toggle_latch = true; // closed
        manual_msg_.toggle_vibe = false; // off
        manual_msg_.excavate_cmd = 0.0;
        manual_msg_.la_cmd = 0.0;
        sm_msg_.state = interfaces::msg::StateMachineCommands::IDLE;

        RCLCPP_INFO(this->get_logger(), "Cleaned up CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_shutdown(const rclcpp_lifecycle::State &state)
    {
        (void)state; // Silence unused param warning

        manual_command_pub_.reset();
        sm_command_pub_.reset();
        joy_cmd_buffer_.reset();
        timer_.reset();

        // Set everything to safe defaults
        manual_msg_.toggle_latch = true; // closed
        manual_msg_.toggle_vibe = false; // off
        manual_msg_.excavate_cmd = 0.0;
        manual_msg_.la_cmd = 0.0;
        sm_msg_.state = interfaces::msg::StateMachineCommands::IDLE;

        RCLCPP_INFO(this->get_logger(), "Shut down CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    // TODO: This needs to go in tibble_controller.cpp
    void CommandCenter::change_state_callback(
        const std::shared_ptr<interfaces::srv::ChangeState::Request> request,
        std::shared_ptr<interfaces::srv::ChangeState::Response> response)
    {
        // might be a good idea to make some sort of not safe to transition states logic?
        if (request->requested_state >= static_cast<uint8_t>(ControlState::STATE_NR_ITEMS))
        {
            response->success = false;
            response->message = "Invalid state requested.";
            return;
        }

        // This is casting to an enum type rather than purely numerical
        current_state_.store(static_cast<ControlState>(request->requested_state));

        response->success = true;
        response->message = "State successfully transitioned.";
        RCLCPP_INFO(this->get_logger(), "Service call received: Switched state.");
    }

    void CommandCenter::control_loop()
    {
        auto joy_msg = joy_cmd_buffer_.readFromRT();

        // Universal button logic (any control mode can access these at any time)
        if (joy_msg->buttons[modes_panic_b_] && current_state_.load() != ControlState::PANIC)
        {
            // TODO: state change request
            previous_state_.store(current_state_.load());
            current_state_.store(ControlState::PANIC);
        }
        else if (joy_msg->buttons[modes_enable_b_] && current_state_.load() == ControlState::PANIC) // Throw into manual if enabled after panic
        {
            // TODO: state change request
            previous_state_.store(current_state_.load());
            current_state_.store(ControlState::MANUAL);
        }
        else if (joy_msg->buttons[modes_manual_b_] && current_state_.load() != ControlState::MANUAL) // Redundant but easier to read
        {
            // TODO: state change request
            previous_state_.store(current_state_.load());
            current_state_.store(ControlState::MANUAL);
        }
        else if (joy_msg->buttons[modes_state_machine_b_] && current_state_.load() != ControlState::STATE_MACHINE)
        {
            // TODO: state change request
            previous_state_.store(current_state_.load());
            current_state_.store(ControlState::STATE_MACHINE);
        }
        else if (joy_msg->buttons[modes_autonomy_b_] && current_state_.load() != ControlState::AUTONOMY)
        {
            // TODO: state change request
            previous_state_.store(current_state_.load());
            current_state_.store(ControlState::AUTONOMY);
        }

        // TODO: Test and maybe implement float tolerance (rn its all digital and some buttons can produce analog like mode buttonss)
        switch (current_state_.load())
        {
        case ControlState::STATE_MACHINE:
            // Pressing more than one state button at once will result in no action (which is what the monster if statements check)

            // If ONLY the idle button is pressed
            if (joy_msg->buttons[sm_idle_b_] == 1 && joy_msg->buttons[sm_travel_b_] == 0 && joy_msg->buttons[sm_excavate_b_] == 0 && joy_msg->buttons[sm_deposit_b_] == 0)
            {
                sm_msg_.state = interfaces::msg::StateMachineCommands::IDLE;
            }
            // If ONLY the travel button is pressed
            else if (joy_msg->buttons[sm_idle_b_] == 0 && joy_msg->buttons[sm_travel_b_] == 1 && joy_msg->buttons[sm_excavate_b_] == 0 && joy_msg->buttons[sm_deposit_b_] == 0)
            {
                sm_msg_.state = interfaces::msg::StateMachineCommands::TRAVEL;
            }
            // If ONLY the excavate button is pressed
            else if (joy_msg->buttons[sm_idle_b_] == 0 && joy_msg->buttons[sm_travel_b_] == 0 && joy_msg->buttons[sm_excavate_b_] == 1 && joy_msg->buttons[sm_deposit_b_] == 0)
            {
                sm_msg_.state = interfaces::msg::StateMachineCommands::EXCAVATE;
            }
            // If ONLY the deposit button is pressed
            else if (joy_msg->buttons[sm_idle_b_] == 0 && joy_msg->buttons[sm_travel_b_] == 0 && joy_msg->buttons[sm_excavate_b_] == 0 && joy_msg->buttons[sm_deposit_b_] == 1)
            {
                sm_msg_.state = interfaces::msg::StateMachineCommands::DEPOSIT;
            }

            sm_command_pub_->publish(sm_msg_);
            return;
        case ControlState::MANUAL:

            // Toggle logic, if button is pressed swap it to whatever the opposite it is right now
            if (joy_msg->buttons[manual_latch_toggle_b_])
            {
                manual_msg_.toggle_latch = !manual_msg_.toggle_latch;
            }
            if (joy_msg->buttons[manual_vibe_toggle_b_])
            {
                manual_msg_.toggle_vibe = !manual_msg_.toggle_vibe;
            }

            // LA logic, if both or neither buttons are pressed send no movement command
            if (joy_msg->buttons[manual_la_extend_b_] == 1 && joy_msg->buttons[manual_la_retract_b_] == 0)
            {
                manual_msg_.la_cmd = joy_msg->buttons[manual_la_extend_b_];
            }
            else if (joy_msg->buttons[manual_la_extend_b_] == 0 && joy_msg->buttons[manual_la_retract_b_] == 1)
            {
                manual_msg_.la_cmd = joy_msg->buttons[manual_la_retract_b_];
            }
            else
            {
                manual_msg_.la_cmd = 0;
            }

            // No extra logic is needed, throw it directly in
            manual_msg_.excavate_cmd = joy_msg->axes[manual_excav_axis_];

            manual_command_pub_->publish(manual_msg_);
            return;
        case ControlState::AUTONOMY:
            return;               // This will remain unimplemented for now
        case ControlState::PANIC: // Do nothing
            return;
        default:
            RCLCPP_ERROR(this->get_logger(), "Unknown state.");
            return;
        }
    }
} // namespace command_center

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    rclcpp::init(argc, argv);
    rclcpp::executors::SingleThreadedExecutor exe;
    std::shared_ptr<command_center::CommandCenter> command_center_node =
        std::make_shared<command_center::CommandCenter>("command_center");
    exe.add_node(command_center_node->get_node_base_interface());
    exe.spin();
    rclcpp::shutdown();
    return 0;
}
