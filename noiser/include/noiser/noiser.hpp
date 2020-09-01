#ifndef NOISER_HPP_
#define NOISER_HPP_

#include <pcl/PCLPointCloud2.h>
#include <pcl/conversions.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/transforms.h>
#include <sensor_msgs/PointCloud2.h>

#include <iostream>
#include <iterator>
#include <random>

#include "ros/ros.h"
namespace noiser {
class NoiserClass {
 public:
  NoiserClass();

 private:
  void pclCallback(const sensor_msgs::PointCloud2ConstPtr& cloud_msg);

  float noise_mean_;
  float noise_stddev_;
  std::default_random_engine generator_;
  ros::Publisher pub_;
  std::string input_topic_name_;
  std::string output_topic_name_;
  std::vector<float> noise_factors_;
};
}  // namespace noiser
#endif  // NOISER_HPP_