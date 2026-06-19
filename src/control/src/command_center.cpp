#include "control/command_center.hpp"

namespace command_center
{
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_configure(const rclcpp_lifecycle::State &)
    {
        enable_b_ = get_node()->get_parameter("enable_b").as_string();
        panic_b_ = get_node()->get_parameter("panic_b").as_string();
        modes_state_machine_b_ = get_node()->get_parameter("state_machine_b").as_string();
        modes_manual_b_ = get_node()->get_parameter("manual_b").as_string();
        modes_autonomy_b_ = get_node()->get_parameter("autonomy_b").as_string();
        sm_idle_b_ = get_node()->get_parameter("idle_b").as_string();
        sm_travel_b_ = get_node()->get_parameter("travel_b").as_string();
        sm_excavate_b_ = get_node()->get_parameter("excavate_b").as_string();
        sm_deposit_b_ = get_node()->get_parameter("deposit_b").as_string();
        manual_la_extend_b_ = get_node()->get_parameter("la_extend_b").as_string();
        manual_la_retract_b_ = get_node()->get_parameter("la_retract_b").as_string();
        manual_latch_toggle_b_ = get_node()->get_parameter("latch_toggle_b").as_string();
        manual_vibe_toggle_b_ = get_node()->get_parameter("vibe_toggle_b").as_string();
        manual_excav_axis_ = get_node()->get_parameter("excav_axis").as_string();

        command_pub_ = this->create_publisher<interfaces::msg::Commands>("/commands", rclcpp::SystemDefaultsQoS());

        change_state_service_ = get_node()->create_service<interfaces::srv::ChangeState>(
            "/change_state",
            std::bind(&CommandCenter::change_state_callback, this, std::placeholders::_1, std::placeholders::_2));

        joy_sub_ = get_node()->create_subscription<sensor_msgs::msg::Joy>(
            "/joy", rclcpp::SensorDataQoS(),
            [this](const sensor_msgs::msg::Joy::SharedPtr msg) {
                joy_cmd_buffer_.writeFromNonRT(*msg);
            });
        
        // Psuedo realtime control/update function. Updates at 50Hz
        timer_ = this->create_wall_timer(50ms, std::bind(&CommandCenter::control_loop, this));

        RCLCPP_INFO(get_node()->get_logger(), "Configured CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_activate(const rclcpp_lifecycle::State &state)
    {
        current_state = STATE_IDLE;

        // Set everything to safe defaults
        msg_.toggle_latch = true;  // closed
        msg_.toggle_vibe = false;   // off
        msg_.excavate_cmd = 0.0;
        msg_.la_cmd = 0.0;

        command_pub_->on_activate();
        LifecycleNode::on_activate(state);

        RCLCPP_INFO(get_node()->get_logger(), "Activated CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_deactivate(const rclcpp_lifecycle::State &state)
    {
        current_state = STATE_IDLE;

        command_pub_->on_deactivate();
        LifecycleNode::on_deactivate(state);

        RCLCPP_INFO(get_node()->get_logger(), "Deactivated CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_cleanup(const rclcpp_lifecycle::State &)
    {
        command_pub_.reset();
        joy_cmd_buffer_.reset();
        timer_.reset();

        // Set everything to safe defaults
        msg_.toggle_latch = true;  // closed
        msg_.toggle_vibe = false;   // off
        msg_.excavate_cmd = 0.0;
        msg_.la_cmd = 0.0;

        RCLCPP_INFO(get_node()->get_logger(), "Cleaned up CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_shutdown(const rclcpp_lifecycle::State &state)
    {
        command_pub_.reset();
        joy_cmd_buffer_.reset();
        timer_.reset();

        // Set everything to safe defaults
        msg_.toggle_latch = true;  // closed
        msg_.toggle_vibe = false;   // off
        msg_.excavate_cmd = 0.0;
        msg_.la_cmd = 0.0;

        RCLCPP_INFO(get_node()->get_logger(), "Shut down CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    void CommandCenter::change_state_callback(
        const std::shared_ptr<interfaces::srv::ChangeState::Request> request,
        std::shared_ptr<interfaces::srv::ChangeState::Response> response)
    {
        // might be a good idea to make some sort of not safe to transition states logic?
        if (request->requested_state > current_state_[STATE_NR_ITEMS])
        {
            response->success = false;
            response->message = "Invalid state requested.";
            return;
        }

        // This is casting to an enum type rather than purely numerical
        current_state_.store(static_cast<State>(request->requested_state));

        response->success = true;
        response->message = "State successfully transitioned to %d", request->requested_state;
        RCLCPP_INFO(get_node()->get_logger(), "Service call received: Switched to state %d", request->requested_state);
    }

    void control_loop()
    {
        auto joy_msg = joy_cmd_buffer_.readFromRT();

        // TODO: Panic/start button logic
        // TODO: mode switch logic

        // TODO: Figure out how to do this lol
        switch (current_state_) {
            case State::STATE_MACHINE:
                return;
            case State::MANUAL:
                return;
            case State::AUTONOMY:
                return
            default:
                RCLCPP_ERROR(get_node()->get_logger(), "Unknown state.");
                return;
        }
    }

    int main(int argc, char *argv[])
    {
        setvbuf(stdout, NULL, _IONBF, BUFSIZ);
        rclcpp::init(argc, argv);
        rclcpp::executors::SingleThreadedExecutor exe;
        std::shared_ptr<CommandCenter> command_center_node =
            std::make_shared<CommandCenter>("command_center");
        exe.add_node(command_center_node->get_node_base_interface());
        exe.spin();
        rclcpp::shutdown();
        return 0;
    }
} // namespace command_center