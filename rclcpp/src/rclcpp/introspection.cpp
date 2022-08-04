#include "rclcpp/introspection.hpp"
#include "rclcpp/node_interfaces/node_parameters_interface.hpp"
#include "rcl/introspection.h"


using rclcpp::IntrospectionUtils;

IntrospectionUtils::IntrospectionUtils(
        rcl_node_t * rcl_node_ptr,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr & node_parameters)
: rcl_node_ptr_(rcl_node_ptr)
{
  // declare service introspection parameters
  if (!node_parameters->has_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_PARAMETER)) {
    node_parameters->declare_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_PARAMETER,
        rclcpp::ParameterValue(true));
  }
  if (!node_parameters->has_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_EVENT_CONTENT_PARAMETER)) {
    node_parameters->declare_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_EVENT_CONTENT_PARAMETER,
        rclcpp::ParameterValue(true));
  }
  if (!node_parameters->has_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_PARAMETER)) {
    node_parameters->declare_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_PARAMETER,
        rclcpp::ParameterValue(true));
  }
  if (!node_parameters->has_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_EVENT_CONTENT_PARAMETER)) {
    node_parameters->declare_parameter(RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_EVENT_CONTENT_PARAMETER,
        rclcpp::ParameterValue(true));
  }

  std::function<void(const std::vector<rclcpp::Parameter> &)>
    configure_service_introspection_callback = 
    [this](const std::vector<rclcpp::Parameter> & parameters) {
      rcl_ret_t ret;
      for (const auto & param: parameters) {
        if (param.get_name() == RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_PARAMETER) {
          for (rcl_service_t * srv: services) {
            ret = rcl_service_introspection_configure_service_events(
                srv, this->rcl_node_ptr_, param.get_value<bool>());
            if (RCL_RET_OK != ret) {
              throw std::runtime_error("Could not configure service introspection events");
            }
          }
        } else if (param.get_name() == RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_PARAMETER) {
          for (rcl_client_t * clt: clients) {
            ret = rcl_service_introspection_configure_client_events(
                clt, this->rcl_node_ptr_, param.get_value<bool>());
            if (RCL_RET_OK != ret) {
              throw std::runtime_error("Could not configure client introspection events");
            }
          }
        } else if (param.get_name() == RCL_SERVICE_INTROSPECTION_PUBLISH_SERVICE_EVENT_CONTENT_PARAMETER) {
          for (rcl_service_t * srv: services) {
            rcl_service_introspection_configure_service_content(srv, param.get_value<bool>());
          }
        } else if (param.get_name() == RCL_SERVICE_INTROSPECTION_PUBLISH_CLIENT_EVENT_CONTENT_PARAMETER) {
          for (rcl_client_t * clt: clients) {
            rcl_service_introspection_configure_client_content(clt, param.get_value<bool>());
          }
        }
      }
    };
  // register callbacks
  post_set_parameters_callback_handle_ = node_parameters->add_post_set_parameters_callback(configure_service_introspection_callback);

}

// Alternatively this wrapper can be made to wrap a create_client call?

void IntrospectionUtils::register_service(
    const rclcpp::ServiceBase::SharedPtr& service){
  this->services.push_back(service->get_service_handle().get());
}

void IntrospectionUtils::register_client(
    const rclcpp::ClientBase::SharedPtr& client){
  this->clients.push_back(client->get_client_handle().get());
}

IntrospectionUtils::~IntrospectionUtils() = default;
