/*
 * Follow-the-carrot node
 */

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseWithCovariance.h>
#include <nav_msgs/Odometry.h>
#include <math.h>
#include <tf/transform_datatypes.h>
#include <angles/angles.h>
#include <nav_msgs/Path.h>
#include "pid.h"
#include <tf/transform_listener.h>
#include <turtlesim/Spawn.h>

ros::Publisher pubVelCmd;
geometry_msgs::Pose currentPose;

double rad2deg(double rad) {
    return (rad*(180/M_PI));
}

double deg2rad(double deg) {
    return (deg*M_PI/180);
}


double getAngleBetweenPoses2D(geometry_msgs::Pose p1, geometry_msgs::Pose p2) {
    float dx = p2.position.x - p1.position.x;
    float dy = p2.position.y - p1.position.y;
    return (atan2(dy, dx));
}

double getDistBetweenPoses2D(geometry_msgs::Pose p1, geometry_msgs::Pose p2) {
    float dx = p2.position.x - p1.position.x;
	float dy = p2.position.y - p1.position.y;
    return (sqrt(dx*dx+dy*dy));
}

/*
 * Taken from the previous assignment, which took it from this source:
 * https://github.com/aniskoubaa/lab_exams/blob/master/src/shape_drawing/shape_drawing.cpp
 */
void rotate(double dest_angle){
	PID pid = PID(1, M_PI, -M_PI, 1, 0.00, 0.0);

	geometry_msgs::Twist vel_msg;

	ros::Rate loop_rate(100);

	double error = pid.calculate(dest_angle, tf::getYaw(currentPose.orientation));
	ROS_INFO("error: %lf", error);
	
	ROS_INFO(" yaw: %lf", tf::getYaw(currentPose.orientation));
	ROS_INFO("dest: %lf", dest_angle);

	while (fabs(error) > deg2rad(0.5)) {
		vel_msg.angular.z = error;
		pubVelCmd.publish(vel_msg);
		error = pid.calculate(dest_angle, tf::getYaw(currentPose.orientation));
		//ROS_INFO("loop: error: %lf", error);
		ros::spinOnce();
		loop_rate.sleep();
	}
	ROS_INFO("houdoe");
	
	//force the robot to stop when it reaches the desired angle
	vel_msg.angular.z = 0;
	pubVelCmd.publish(vel_msg);
}

/*
 * Simple move() derived from a function in the previous functions source.
 */
void shoot(const geometry_msgs::PoseStamped::ConstPtr &msg) {
	PID pid = PID(1, 100, -100, 1, 0.00, 0.0);
	geometry_msgs::Twist vel_msg;
	ros::Rate loop_rate(100);

	double dest = getDistBetweenPoses2D(currentPose, msg->pose);
	double error = pid.calculate(dest, 0.0);
	double prevError = error;
	while(fabs(error) > 0.01 ) {
		dest = getDistBetweenPoses2D(currentPose, msg->pose);
		error = pid.calculate(dest, 0.0);
		if (error > prevError) {
			ROS_INFO("Ho! Ik ben te ver!");			
			break;
		}
		ROS_INFO("loop: error: %lf", error);
		vel_msg.linear.x = error;
		pubVelCmd.publish(vel_msg);
		ros::spinOnce();
		loop_rate.sleep();
		prevError = error;	
	}
	
	vel_msg.linear.x = 0;
	pubVelCmd.publish(vel_msg);
}

void cbGoal(const geometry_msgs::PoseStamped::ConstPtr &msg) {
    double angle = getAngleBetweenPoses2D(currentPose, msg->pose);
	double distance = getDistBetweenPoses2D(currentPose, msg->pose);
    
    double yaw = tf::getYaw(currentPose.orientation);

    ROS_INFO("planned heading: %lf\n", rad2deg(angle));
    ROS_INFO("pre heading    : %lf\n", rad2deg(yaw));

	rotate(angle);
	shoot(msg);
	
	ros::spinOnce();

	double gYaw = tf::getYaw(msg->pose.orientation);
	ROS_INFO("final heading  : %lf\n", rad2deg(gYaw));
	
	ros::spinOnce();
	rotate(gYaw);
}

void cbOdom(const nav_msgs::Odometry::ConstPtr &msg) {
    currentPose = msg->pose.pose;
}

void cbPath(const nav_msgs::Path::ConstPtr &msg) {
	auto goals = msg->poses;
	ROS_INFO("Path sz: %d", (int)goals.size());
	for (auto goal : goals) {
		ROS_INFO("x: %lf, y: %lf", goal.pose.position.x, goal.pose.position.y);		
		ROS_INFO("kek");
		geometry_msgs::PoseStamped::ConstPtr goalPtr(new geometry_msgs::PoseStamped(goal));
		cbGoal(goalPtr);
	}
}

int main(int argc, char **argv) {
	ros::init(argc, argv, "follow_carrot_node");
	ros::NodeHandle n;
	//ros::Subscriber subGoal = n.subscribe("/goal", 100, cbGoal);
	
	ros::Subscriber subPath = n.subscribe("/plan", 100, cbPath);
    ros::Subscriber subOdom = n.subscribe("/odom", 100, cbOdom);
    pubVelCmd = n.advertise<geometry_msgs::Twist>("/cmd_vel", 100);
		
	// ros::service::waitForService("spawn");
	// ros::ServiceClient add_turtle = n.serviceClient<turtlesim::Spawn>("spawn");
	// turtlesim::Spawn srv;
	// add_turtle.call(srv);
	// tf::TransformListener listener;

	// ros::Rate rate(10.0);	
	// while (n.ok()){
	// 	tf::StampedTransform transform;
	// 	try{
	// 	  listener.lookupTransform("/turtle2", "/turtle1",  
	// 							   ros::Time(0), transform);
	// 	}
	// 	catch (tf::TransformException ex){
	// 	  ROS_ERROR("%s",ex.what());
	// 	  ros::Duration(1.0).sleep();
	// 	}
	
	// 	turtlesim::Velocity vel_msg;
	// 	vel_msg.angular = 4.0 * atan2(transform.getOrigin().y(),
	// 								transform.getOrigin().x());
	// 	vel_msg.linear = 0.5 * sqrt(pow(transform.getOrigin().x(), 2) +
	// 								pow(transform.getOrigin().y(), 2));
	// 	turtle_vel.publish(vel_msg);
	// 	ros.spinOnce();
	// 	rate.sleep();
	//   }

	ros::spin();

	return 0;
}
