/*
 * 	This file is part of CrsmSlam.
    CrsmSlam is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CrsmSlam is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrsmSlam.  If not, see <http://www.gnu.org/licenses/>.
* 
* Author : Manos Tsardoulias, etsardou@gmail.com
* Organization : AUTH, PANDORA Robotics Team
* */

#ifndef CRSM_SLAM_HEADER
#define CRSM_SLAM_HEADER


#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>

#include "tf/transform_broadcaster.h"
#include "tf/transform_listener.h"
#include "ros/ros.h"

#include <geometry_msgs/Quaternion.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>

#include "crsm_laser.h"
#include "crsm_hillClimbing.h"
#include "crsm_pose.h"
#include "crsm_map.h"
#include "crsm_slamParameters.h"

namespace crsm_slam{

	/**
	 @class CrsmSlam
	 @brief The main slam class. Contains the main functionalities of CRSM slam.
	 **/ 
	class CrsmSlam{

		private:
			ros::Subscriber clientLaserValues;				//!< The laser subscriber 
			ros::Publisher _occupancyGridPublisher;			//!< The occupancy grid map publisher
			ros::Publisher _pathPublisher;					//!< The robot trajectory publisher
			tf::TransformBroadcaster _slamFrameBroadcaster; //!< The tf robpt pose broadcaster
			tf::TransformListener _listener;				//!< Tf listener to aquire the transformation between the laser and the robot center
			
			CrsmMap map;		//!< The OGM container
			CrsmLaser laser;	//!< The laser container
				
			int argc;			//!< Number of input arguments
			char **argv;		//!< The input arguments

			unsigned int counter;	//!< Slam iterations counter

			float bestFitness;		//!< The best RRHC fitness for a specific iteration
			float meanDensity;		//!< The mean laser scan density for a specific iteration

			CrsmTransformation bestTransformation;	//!< The best RRHC transformation for a specific iteration
			CrsmPose robotPose;					//!< The robot pose
			CrsmSlamParameters slamParams;			//!< The slam parameters
			
			std::vector<CrsmPose> robotTrajectory;	//!< Container for the robot trajectory

			std::set<int> scanSelections;			//!< Holds the critical rays, on which the scan matching is performed
			std::set<int> bigChanges;				//!< Holds the irregularities of a specific scan in terms of distance	
			
			/**
			@brief Reads the CRSM slam parameters from the yaml file and fills the CrsmSlamParameters structure
			@return void
			**/
			void updateParameters(void);
			
			ros::Timer _pathPublishingTimer;		//!< The trajectory publishing timer
			ros::Timer _robotPosePublishingTimer;	//!< The robot pose publishing timer
			ros::Timer _mapPublishingTimer;			//!< The map publishing timer

		public:
		
			ros::NodeHandle n;	//!< The ROS node handle
			

			/**
			@brief Default costructor
			@param argc [int] The number of input arguments
			@param argv [char **] The input arguments
			@return void
			**/
			CrsmSlam(int argc, char **argv);
			
			/**
			@brief Destructor
			@return void
			**/
			~CrsmSlam(){}
			
			/**
			@brief Updates map after finding the new robot pose
			@return void
			**/
			void updateMapProbabilities(void);

			/**
			@brief Chooses important rays for RRHC
			@return void
			**/
			void calculateCriticalRays(void);

			/**
			@brief Calculates the transformation (translation & rotation) with RRHC
			@return void
			**/
			void findTransformation(void);
			
			/**
			@brief Serves the laser scan messages
			@param msg [sensor_msgs::LaserScanConstPtr&] : The laser rays distances
			@return void
			**/
			void fixNewScans(const sensor_msgs::LaserScanConstPtr& msg);
			
			/**
			@brief Starts the laser subscriber, listening to laser_subscriber_topic from parameters
			@return void
			**/
			void startLaserSubscriber(void);
			
			/**
			@brief Stops the laser subscriber
			@return void
			**/
			void stopLaserSubscriber(void);

			/**
			@brief Starts the OccupancyGrid publisher, posting to occupancy_grid_publish_topic from parameters
			@return void
			**/
			void startOGMPublisher(void);
			
			/**
			@brief Stops the OccupancyGrid publisher
			@return void
			**/
			void stopOGMPublisher(void);
			
			/**
			@brief Publishes the OccupancyGrid map as nav_msgs::OccupancyGrid, posting with occupancy_grid_map_freq Hz from parameters
			@param e [const ros::TimerEvent&] The timer event
			@return void
			**/
			void publishOGM(const ros::TimerEvent& e);
			
			/**
			@brief Publishes the Tf robot pose, posting with robot_pose_tf_freq Hz from parameters
			@param e [const ros::TimerEvent&] The timer event
			@return void
			**/
			void publishRobotPoseTf(const ros::TimerEvent& e);
			
			/**
			@brief Starts the Trajectory publisher, posting to robot_trajectory_publish_topic from parameters, with trajectory_publisher_frame_id as frame ID.
			@return void
			**/
			void startTrajectoryPublisher(void);
			
			/**
			@brief Stops the Trajectory publisher.
			@return void
			**/
			void stopTrajectoryPublisher(void);
			
			/**
			@brief Publishes the robot trajectory as nav_msgs::Path, posting with trajectory_freq Hz from parameters
			@param e [const ros::TimerEvent&] The timer event
			@return void
			**/
			void publishTrajectory(const ros::TimerEvent& e);
			
			/**
			@brief Returns the map info in a CrsmMapInfo structure
			@return CrsmMapInfo
			**/
			CrsmMapInfo getMapInfo(void);
			
			/**
			@brief Returns the map occupancy probability of coordinates (x,y) ranging from 0-255 (0 is occupied, 255 is free)
			@param x [int] : The x coordinate
			@param y [int] : The y coordinate
			@return char probability
			**/
			char getMapProbability(int x,int y);
			
			/**
			@brief Returns the robot pose in a CrsmPose structure
			@return CrsmPose
			**/
			CrsmPose getRobotPose(void);
			
			/**
			@brief Returns the laser info in a CrsmLaserInfo structure
			@return CrsmLaserInfo
			**/
			CrsmLaserInfo getLaserInfo(void);
			
			/**
			@brief Returns the robot trajectory in a vector of CrsmPose structures
			@return std::vector<CrsmPose>
			**/
			std::vector<CrsmPose> getTrajectory(void);
			
			//---------------------- Setters and getters for slamParameters ----------------------------//
			/**
			@brief Sets the disparity of CRSM_SlamParameters
			@param disparity [int] Disparity of mutation in pixels at hill climbing
			@return void
			**/
			void setDisparity(int disparity);
			
			/**
			@brief Sets the map_size of CRSM_SlamParameters
			@param size [int] Map size of initial allocated map
			@return void
			**/
			void setInitialMapSize(int size);
			
			/**
			@brief Sets the ocgd of CRSM_SlamParameters
			@param ocgd [double] [OC]cupancy [G]rid [D]imentionality - the width and height in meters of a pixel
			@return void
			**/
			void setOcgd(double ocgd);
			
			/**
			@brief Sets the density of CRSM_SlamParameters
			@param density [double] Map update density (0-127)
			@return void
			**/
			void setDensity(double density);
			
			/**
			@brief Sets the obstacle_density of CRSM_SlamParameters
			@param ob_density [double] Coefficient for obstacle update density (0+)
			@return void
			**/
			void setObstacleDensity(double ob_density);
			
			/**
			@brief Sets the scan_selection_meters of CRSM_SlamParameters
			@param scan_selection_meters [double] Scan density lower boundary for a scan-part identification
			@return void
			**/
			void setScanSelectionMeters(double scan_selection_meters);
			
			/**
			@brief Sets the max_hill_climbing_iterations of CRSM_SlamParameters
			@param iterations [int] Maximum RRHC iterations
			@return void
			**/
			void setMaxHillClimbingIterations(int iterations);
			
			/**
			@brief Sets the dx_laser_robotCenter of CRSM_SlamParameters
			@param dx [double] Translation in x axis of laser in comparison to robot center
			@return void
			**/
			void setDxLaserRobotCenter(double dx);
			
			/**
			@brief Sets the occupancy_grid_map_freq of CRSM_SlamParameters
			@param freq [double] The occupancy grid map publishing frequency
			@return void
			**/
			void setOccupancyGridMapFreq(double freq);
			
			/**
			@brief Sets the robot_pose_tf_freq of CRSM_SlamParameters
			@param freq [double] The robot pose publishing frequency
			@return void
			**/
			void setRobotPoseTfFreq(double freq);
			
			/**
			@brief Sets the trajectory_freq of CRSM_SlamParameters
			@param freq [double] The trajectory publishing frequency
			@return void
			**/
			void setTrajectoryFreq(double freq);
			
			/**
			@brief Sets the desired_number_of_picked_rays of CRSM_SlamParameters
			@param rays [int] The desired number of picked rays [algorithm specific]
			@return void
			**/
			void setDesiredNumberOfPickedRays(int rays);
			
			/**
			@brief Sets the robot_width of CRSM_SlamParameters
			@param width [double] The robot width
			@return void
			**/
			void setRobotWidth(double width);
			
			/**
			@brief Sets the robot_length of CRSM_SlamParameters
			@param length [double] The robot length
			@return void
			**/
			void setRobotLength(double length);
			
			/**
			@brief Sets the occupancy_grid_publish_topic of CRSM_SlamParameters
			@param topic [std::string] The occupancy grid publishing topic
			@return void
			**/
			void setOccupancyGridPublishTopic(std::string topic);
			
			/**
			@brief Sets the robot_trajectory_publish_topic of CRSM_SlamParameters
			@param topic [std::string] The trajectory publishing topic
			@return void
			**/
			void setRobotTrajectoryPublishTopic(std::string topic);
			
			/**
			@brief Sets the trajectory_publisher_frame_id of CRSM_SlamParameters
			@param frame_id [std::string] The trajectory frame ID
			@return void
			**/
			void setTrajectoryPublisherFrameId(std::string frame_id);
			
			/**
			@brief Sets the laser_subscriber_topic of CRSM_SlamParameters
			@param topic [std::string] The laser subscriber topic
			@return void
			**/
			void setLaserSubscriberTopic(std::string topic);
			
			/**
			@brief Sets the world_frame of CRSM_SlamParameters
			@param frame [std::string] Holds the world frame 
			@return void
			**/
			void setWorldFrame(std::string frame);
			
			/**
			@brief Sets the base_footprint_frame of CRSM_SlamParameters
			@param frame [std::string] Holds the base footprint frame - (x,y,yaw)
			@return void
			**/
			void setBaseFootprintFrame(std::string frame);
			
			/**
			@brief Sets the base_frame of CRSM_SlamParameters
			@param frame [std::string] Holds the base footprint frame - (x,y,yaw)
			@return void
			**/
			void setBaseFrame(std::string frame);
			void setMapFrame(std::string frame);
			void setLaserFrame(std::string frame);
		};

}

#endif

