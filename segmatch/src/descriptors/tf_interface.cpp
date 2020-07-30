#include "segmatch/descriptors/tf_interface.hpp"

namespace ns_tf_interface {

TensorflowInterface::TensorflowInterface() {
  ros::NodeHandle nh;

  cnn_input_publisher_ = nh.advertise<segmatch::cnn_input_msg>(
      "tf_interface_topic/cnn_input_topic", 50u);

  cnn_output_subscriber_ =
      nh.subscribe("tf_interface_topic/cnn_output_topic", 1,
                   &TensorflowInterface::cnn_output_callback, this);

  ROS_INFO_STREAM("advertising");
}

void TensorflowInterface::batchFullForwardPass(
    const std::vector<Array3D>& inputs, const std::string& input_tensor_name,
    const std::vector<std::vector<float> >& scales,
    const std::string& scales_tensor_name,
    const std::string& descriptor_values_name,
    const std::string& reconstruction_values_name,
    std::vector<std::vector<float> >& descriptors,
    std::vector<Array3D>& reconstructions) {
  CHECK(!inputs.empty());
  descriptors.clear();
  reconstructions.clear();

  segmatch::cnn_input_msg msg;

  msg.input_tensor_name = input_tensor_name;
  msg.scales_tensor_name = scales_tensor_name;
  msg.descriptor_values_name = descriptor_values_name;
  msg.reconstruction_values_name = reconstruction_values_name;

  std_msgs::MultiArrayDimension layout_dim;

  // Scales
  layout_dim.size = scales.size();
  layout_dim.stride = scales.size() * scales[0].size();
  msg.scales.layout.dim.push_back(layout_dim);

  layout_dim.size = scales[0].size();
  layout_dim.stride = scales[0].size();
  msg.scales.layout.dim.push_back(layout_dim);

  for (int i = 0; i < msg.scales.layout.dim[0].size; ++i) {
    for (int j = 0; j < msg.scales.layout.dim[1].size; ++j) {
      msg.scales.data.push_back(scales[i][j]);
    }
    // ROS_INFO_STREAM(scales[i][0] << " " << scales[i][1] << " " <<
    // scales[i][2]);
  }

  // Inputs
  size_t dim_1 = inputs.size();
  size_t dim_2 = inputs[0].container.size();
  size_t dim_3 = inputs[0].container[0].size();
  size_t dim_4 = inputs[0].container[0][0].size();
  layout_dim.size = dim_1;
  layout_dim.stride = dim_1 * dim_2 * dim_3 * dim_4;
  msg.inputs.layout.dim.push_back(layout_dim);

  layout_dim.size = dim_2;
  layout_dim.stride = dim_2 * dim_3 * dim_4;
  msg.inputs.layout.dim.push_back(layout_dim);

  layout_dim.size = dim_3;
  layout_dim.stride = dim_3 * dim_4;
  msg.inputs.layout.dim.push_back(layout_dim);

  layout_dim.size = dim_4;
  layout_dim.stride = dim_4;
  msg.inputs.layout.dim.push_back(layout_dim);

  for (int i = 0; i < msg.inputs.layout.dim[0].size; ++i) {
    for (int j = 0; j < msg.inputs.layout.dim[1].size; ++j) {
      for (int k = 0; k < msg.inputs.layout.dim[2].size; ++k) {
        for (int l = 0; l < msg.inputs.layout.dim[3].size; ++l) {
          msg.inputs.data.push_back(inputs[i].container[j][k][l]);
        }
      }
    }
  }
  auto msg_time_stamp = ros::Time::now().toNSec();
  msg.timestamp = msg_time_stamp;
  ROS_INFO_STREAM("Sending: " << msg.timestamp);
  cnn_input_publisher_.publish(msg);
  ros::Rate loop_rate(100);
  ros::spinOnce();
  loop_rate.sleep();

  ros::Rate wait_rate(10);
  while (ros::ok()) {
    auto it = returned_cnn_msgs_.find(msg_time_stamp + 1);
    if (it != returned_cnn_msgs_.end()) {
      ROS_INFO_STREAM("Found message: " << msg_time_stamp);
      returned_cnn_msgs_.erase(it);
      break;
    } else {
      ROS_INFO_STREAM("waiting");
      wait_rate.sleep();
      if (ros::Time::now().toNSec() - msg_time_stamp > 1e9) {
        ROS_WARN_STREAM("Message lost!: " << msg_time_stamp);
        return;
      }
    }
  }


}

void TensorflowInterface::cnn_output_callback(segmatch::cnn_output_msg msg) {
  returned_cnn_msgs_.insert(make_pair(msg.timestamp, msg));
}

}  // namespace ns_tf_interface