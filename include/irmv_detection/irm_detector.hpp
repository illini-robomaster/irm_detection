#pragma once

#include "irmv_detection/yolo_engine.hpp"
#include "irmv_detection/armor.hpp"
#include "irmv_detection/pnp_solver.hpp"

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/float64.hpp"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "auto_aim_interfaces/msg/armors.hpp"
#include <memory>


namespace irmv_detection
{
  class IrmDetector
  {
    public:
      explicit IrmDetector(const rclcpp::NodeOptions & options);
      rclcpp::node_interfaces::NodeBaseInterface::SharedPtr get_node_base_interface() const
      {
        return node_->get_node_base_interface();
      }

    private:
      void declare_parameters();
      void message_callback(const sensor_msgs::msg::Image::ConstSharedPtr& msg);
      std::vector<Armor> extract_armors(const cv::Mat &image, const std::vector<YoloEngine::bbox> &bboxes);
      void visualize_armors(cv::Mat &image, const std::vector<Armor> &armors);
      rcl_interfaces::msg::SetParametersResult param_event_callback(const std::vector<rclcpp::Parameter> &parameters);

      rclcpp::Node::SharedPtr node_;

      std::unique_ptr<YoloEngine> yolo_engine_;
      std::unique_ptr<PnPSolver> pnp_solver_;
      rclcpp::Publisher<auto_aim_interfaces::msg::Armors>::SharedPtr armors_pub_;
      rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr camera_info_sub_;
      image_transport::Subscriber img_sub_;
      rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_event_handle_;

      // Parameters
      bool enable_debug_; // This publishes visualized image and profiling data
      bool enable_profiling_; // This publishes profiling data (always enabled if enable_debug_ is true)
      bool enable_rviz_; // This publishes armor markers for visualization in Rviz
      int binary_threshold_; // Binary threshold for thresholding the image during armor extraction
      int enemy_color_;
      float light_min_ratio_;
      float light_max_ratio_;
      float light_max_angle_;
      float armor_min_small_center_distance_;
      float armor_max_small_center_distance_;
      float armor_min_large_center_distance_;
      float armor_max_large_center_distance_;
      cv::Size image_input_size_; // Input size of the YOLO model

      // Debug & profiling
      image_transport::Publisher visualized_img_pub_; // Visualized image with bboxes and armors
      image_transport::Publisher binary_img_pub_; // Binary image after thresholding
      rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr total_latency_pub_; // Total latency from image fetch to PnP estimation
      rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr comm_latency_pub_; // Communication latency from image fetch to callback start (this is caused by ROS2 communication, we can't do anything about it)
      rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr processing_latency_pub_; // Processing latency = total latency - communication latency
      rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr inference_latency_pub_; // Inference latency caused by YOLOEngine, this includes preprocessing, inference, and postprocessing
      rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr pnp_latency_pub_; // PnP latency caused by PnPSolver

      // Rviz visualization
      visualization_msgs::msg::Marker armor_marker_;
      visualization_msgs::msg::Marker text_marker_;
      visualization_msgs::msg::MarkerArray marker_array_;
      rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_array_pub_;
  };
}