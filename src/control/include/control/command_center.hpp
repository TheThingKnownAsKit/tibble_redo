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
#include "interfaces/msg/manual_commands.hpp"
#include "interfaces/msg/state_machine_commands.hpp"

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
        std::string modes_enable_b_; // b = button
        std::string modes_panic_b_;
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
        std::atomic<ControlState> current_state_;   // atomic just means it is safe to share between multiple threads (necessary for real time)
        std::atomic<ControlState> previous_state_;  // needed for transition logic
        std::shared_ptr<rclcpp::TimerBase> timer_;
        auto manual_msg_ = interfaces::msg::ManualCommands();
        auto sm_msg_ = interfaces::msg::StateMachineCommands(); // sm = state machine always in these files

        // Subs
        rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
        // Eventually something like nav2 commands sub will go here

        // Pubs
        std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>> manual_command_pub_;
        std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>> sm_command_pub_;

        // Realtime buffers
        realtime_tools::RealtimeBuffer<sensor_msgs::msg::Joy> joy_cmd_buffer_;

        // Service server
        rclcpp::Service<interfaces::srv::ChangeState>::SharedPtr change_state_service_;

        // State enum for the control schema (NOT THE SAME AS THE STATE MACHINE)
        enum class ControlState
        {
            MANUAL,
            STATE_MACHINE,
            AUTONOMY,
            PANIC,          // shut EVERYTHING down, not commands get processed or sent besides start to remove panic
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