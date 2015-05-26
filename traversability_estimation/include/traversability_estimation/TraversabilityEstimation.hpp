/*
 * TraversabilityEstimation.hpp
 *
 *  Created on: Oct 22, 2014
 *      Author: Ralf Kaestner, Peter Fankhauser
 *	 Institute: ETH Zurich, Autonomous Systems Lab
 */

#pragma once

// Grid Map
#include <grid_map/grid_map.hpp>
#include <grid_map_msgs/GetGridMapInfo.h>

// Traversability estimation
#include "traversability_msgs/CheckFootprintPath.h"

// ROS
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <filters/filter_chain.h>
#include <std_srvs/Empty.h>

// Schweizer-Messer
#include <sm/timing/Timer.hpp>

// STD
#include <vector>
#include <string>

// Eigen
#include <Eigen/Core>

namespace traversability_estimation {

/*!
 * The terrain traversability estimation main class. Coordinates the ROS
 * interfaces, the timing, and the data handling between the other classes.
 */
class TraversabilityEstimation
{

 public:
  /*!
   * Constructor.
   * @param nodeHandle the ROS node handle.
   */
  TraversabilityEstimation(ros::NodeHandle& nodeHandle);

  /*!
   * Destructor.
   */
  virtual ~TraversabilityEstimation();

  /*!
   * Computes the traversability of each filter and adds it as layer to the elevation map.
   * Traversability is set between 0.0 and 1.0, where a value of 0.0 means not
   * traversable and 1.0 means fully traversable.
   * @param[in/out] elevationMap the map for which the traversability is computed.
   */
  bool updateFilters(const grid_map::GridMap& elevationMap,
                     grid_map::GridMap& traversabilityMap);

  /*!
   * Computes the traversability and publishes it as grid map.
   * Traversability is set between 0.0 and 1.0, where a value of 0.0 means not
   * traversable and 1.0 means fully traversable.
   */
  void computeTraversability();

  /*!
   * ROS service callback function that forces an update of the traversability
   * map from a new elevation map requested from the grid map service.
   * @param request the ROS service request.
   * @param response the ROS service response.
   * @return true if successful.
   */
  bool updateServiceCallback(grid_map_msgs::GetGridMapInfo::Request& request,
                             grid_map_msgs::GetGridMapInfo::Response& response);

  /*!
   * ROS service callback function that forces an update of the filter parameters.
   * The parameters are read from the .yaml file and put on the parameter server.
   * The filter chain is reconfigured with the new parameter.
   * @param request the ROS service request.
   * @param response the ROS service response.
   * @return true if successful.
   */
  bool updateParameter(std_srvs::Empty::Request& request,
                       std_srvs::Empty::Response& response);

  /*!
   * ROS service callback function to return a boolean to indicate if a path is traversable.
   * @param request the ROS service request defining footprint path.
   * @param response the ROS service response containing the traversability of the footprint path.
   * @return true if successful.
   */
  bool checkFootprintPath(
      traversability_msgs::CheckFootprintPath::Request& request,
      traversability_msgs::CheckFootprintPath::Response& response);

  /*!
   * Callback function that receives an elevation map as grid map.
   * @param elevationMap the received elevation map.
   */
  void elevationMapCallback(const grid_map_msgs::GridMap& elevationMap);

 private:

  /*!
   * Reads and verifies the ROS parameters.
   * @return true if successful.
   */
  bool readParameters();

  /*!
   * Callback function for the update timer. Forces an update of the traversability
   * map from a new elevation map requested from the grid map service.
   * @param timerEvent the timer event.
   */
  void updateTimerCallback(const ros::TimerEvent& timerEvent);

  /*!
   * Gets the grid map for the desired submap center point.
   * @param[out] map the map that is received.
   * @return true if successful, false if ROS service call failed.
   */
  bool getGridMap(grid_map_msgs::GridMap& map);

  /*!
   * Gets the traversability value of the submap defined by the polygon. Is true if the
   * whole polygon is traversable.
   * @param[in] polygon polygon that defines submap of the traversability map.
   * @param[out] traversability traversability value of submap defined by the polygon, the traversability
   * is the mean of each cell within the polygon.
   * @return true if the whole polygon is traversable, false otherwise.
   */
  bool isTraversable(const grid_map::Polygon& polygon, double& traversability);

  /*!
   * Checks if the overall inclination of the robot on a line between two
   * positions is feasible.
   * @param[in] start first position of the line.
   * @param[in] end last position of the line.
   * @return true if the whole line has a feasible inclination, false otherwise.
   */
  bool checkInclination(const grid_map::Position start,
                        const grid_map::Position end);

  //! ROS node handle.
  ros::NodeHandle& nodeHandle_;

  //! ROS service server.
  ros::ServiceServer footprintPathService_;
  ros::ServiceServer updateTraversabilityService_;
  ros::ServiceServer updateParameters_;

  //! Elevation map subscriber.
  ros::Subscriber elevationMapSub_;

  //! Name of the elevation map topic.
  std::string elevationMapTopic_;
  bool getGridMap_;

  //! Elevation map service client.
  ros::ServiceClient submapClient_;

  //! Name of the elevation submap service.
  std::string submapServiceName_;

  //! TF listener.
  tf::TransformListener transformListener_;

  //! Center point of the requested map.
  geometry_msgs::PointStamped submapPoint_;

  //! Id of the frame of the elevation map.
  std::string mapFrameId_;

  //! Id of the frame of the robot.
  std::string robotFrameId_;

  //! Publisher of the traversability occupancy grid.
  ros::Publisher traversabilityMapPublisher_;

  //! Publisher of the roughness filter occupancy grid.
  ros::Publisher footprintPolygonPublisher_;

  //! Timer for the map update.
  ros::Timer updateTimer_;

  //! Duration between map updates.
  ros::Duration updateDuration_;

  //! Default value for traversability of unknown regions.
  double traversabilityDefault_;

  //! Requested map cell types.
  std::vector<std::string> requestedMapTypes_;

  //! Requested map length in [m].
  Eigen::Array2d mapLength_;

  //! Traversability map types.
  const std::string traversabilityType_;
  const std::string slopeType_;
  const std::string stepType_;
  const std::string roughnessType_;
  const std::string robotSlopeType_;

  //! Filter Chain
  filters::FilterChain<grid_map::GridMap> filter_chain_;

  //! Traversability map.
  grid_map::GridMap traversabilityMap_;

  //! Traversability map.
  grid_map::GridMap elevationMap_;

  //! Timer
  std::string timerId_;
  sm::timing::Timer timer_;
};

} /* namespace */
