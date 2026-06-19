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
            std::bind(&CommandCenter::change_state_callback, this, std::placeholders::_1, std::placeholders::_2)
        );

        RCLCPP_INFO(get_node()->get_logger(), "Configured CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_activate(const rclcpp_lifecycle::State &state)
    {
        current_state = STATE_IDLE;

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
        RCLCPP_INFO(get_node()->get_logger(), "Cleaned up CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
    CommandCenter::on_shutdown(const rclcpp_lifecycle::State &state)
    {
        command_pub_.reset();
        RCLCPP_INFO(get_node()->get_logger(), "Shut down CommandCenter.");
        return rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn::SUCCESS;
    }

    void CommandCenter::change_state_callback(
        const std::shared_ptr<interfaces::srv::ChangeState::Request> request,
        std::shared_ptr<interfaces::srv::ChangeState::Response> response)
    {
        // might be a good idea to make some sort of not safe to transition states logic?
        if (request->requested_state > 2) {
            response->success = false;
            response->message = "Invalid state requested.";
            return;
        }

        current_state_.store(request->requested_state);

        response->success = true;
        response->message = "State successfully transitioned to %d", request->requested_state;
        RCLCPP_INFO(get_node()->get_logger(), "Service call received: Switched to state %d", request->requested_state);
    }

    int main(int argc, char * argv[])
    {
        return 0;
    }
} // namespace command_center