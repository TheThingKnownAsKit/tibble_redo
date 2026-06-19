#include <rclcpp/rclcpp.hpp>
#include <interfaces/srv/change_state.hpp>

#include <memory>

namespace change_state_server
{

    void change_state(const std::shared_ptr<interfaces::srv::ChangeState::Request> request,
                      std::shared_ptr<interfaces::srv::ChangeState::Response> response)
    {
        
    }

    void send_state_request(int target_state)
    {

    }

    int main(int argc, char **argv)
    {
        rclcpp::init(argc, argv);

        std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("change_state_server");

        rclcpp::Service<interfaces::srv::ChangeState>::SharedPtr service =
            node->create_service<interfaces::srv::ChangeState>("change_state", &change_state);

        rclcpp::spin(node);
        rclcpp::shutdown();
    }

} // namespace change_state_server