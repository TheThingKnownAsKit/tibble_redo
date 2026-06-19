#ifndef COMMAND_CENTER_HPP
#define COMMAND_CENTER_HPP

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <atomic>

#include "lifecycle_msgs/msg/transition.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/publisher.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "realtime_tools/realtime_buffer.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/joy.hpp"

#include "interfaces/srv/change_state.hpp"
#include "interfaces/msg/commands.hpp"

namespace command_center
{
    class CommandCenter : public rclcpp_lifecycle::LifecycleNode
    {
    public:
        explicit CommandCenter(const std::string &node_name, bool intra_process_comms = false)
            : rclcpp_lifecycle::LifecycleNode(node_name,
                                              rclcpp::NodeOptions().use_intra_process_comms(intra_process_comms))
        {
        }

        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
        on_configure(const rclcpp_lifecycle::State &);

        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
        on_activate(const rclcpp_lifecycle::State &state);

        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
        on_deactivate(const rclcpp_lifecycle::State &state);

        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
        on_cleanup(const rclcpp_lifecycle::State &);

        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
        on_shutdown(const rclcpp_lifecycle::State &state);

    private:
        // Parameters from YAML
        std::string enable_b_; // b = button
        std::string panic_b_;
        std::string modes_state_machine_b_;
        std::string modes_manual_b_;
        std::string modes_autonomy_b_;
        std::string sm_idle_b_; // sm = state machine
        std::string sm_travel_b_;
        std::string sm_excavate_b_;
        std::string sm_deposit_b_;
        std::string manual_la_extend_b_;
        std::string manual_la_retract_b_;
        std::string manual_latch_toggle_b_;
        std::string manual_vibe_toggle_b_;
        std::string manual_excav_axis_;

        // Internal variables
        std::atomic<State> current_state_;
        std::shared_ptr<rclcpp::TimerBase> timer_;
        auto msg_ = interfaces::msg::Commands();

        // Subs
        rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;

        // Pubs
        std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>> command_pub_;

        // Realtime buffers
        realtime_tools::RealtimeBuffer<sensor_msgs::msg::Joy> joy_cmd_buffer_;

        // Service server
        rclcpp::Service<interfaces::srv::ChangeState>::SharedPtr change_state_service_;

        // State enum
        enum class State
        {
            STATE_MACHINE,
            MANUAL,
            AUTONOMY,
            STATE_NR_ITEMS, // NOT A REAL STATE, this is just for bounds checking
        };

        // Functions
        void change_state_callback(
            const std::shared_ptr<interfaces::srv::ChangeState::Request> request,
            std::shared_ptr<interfaces::srv::ChangeState::Response> response);

        void control_loop();
    }
} // namespace command_center

#endif // COMMAND_CENTER_HPP