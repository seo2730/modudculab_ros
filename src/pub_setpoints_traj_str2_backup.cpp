/**
 * @file offb_node.cpp
 * @brief offboard example node, written with mavros version 0.14.2, px4 flight
 * stack and tested in Gazebo SITL
 */

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include "math.h"

double r;
double theta;
double count=0.0;
double wn;
double z;
mavros_msgs::State current_state;
void state_cb(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}

geometry_msgs::Vector3Stamped vec;
void vector3(const geometry_msgs::Vector3Stamped::ConstPtr& vec_){ 
    vec = *vec_;   
    wn = double(vec.vector.x);
    r = double(vec.vector.y);
    z = double(vec.vector.z);
    ROS_INFO("recieve vec = %lf, %lf",vec.vector.x,vec.vector.y);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "pub_setpoints_traj_str2");
    ros::NodeHandle nh;

    ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
            ("mavros/state", 10, state_cb);
    ros::Subscriber state_position = nh.subscribe<geometry_msgs::Vector3Stamped>
	    ("ros/state",1,vector3);
    ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
            ("mavros/setpoint_position/local", 10);
    ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
            ("mavros/cmd/arming");
    ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
            ("mavros/set_mode");

    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20);
	

	nh.param("pub_setpoints_traj_str2/wn", wn, 0.0);
        nh.param("pub_setpoints_traj_strw2/z", z, 2.0);

	nh.param("pub_setpoints_traj_str2/r", r, 1.0);
    // wait for FCU connection
    while(ros::ok() && current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

    geometry_msgs::PoseStamped pose;
    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 2;


    //send a few setpoints before starting
    for(int i = 1000; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose); 
        //local_pos_twi.publish(twist);
        ros::spinOnce();
        rate.sleep();
    }

    mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "OFFBOARD";

    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;

    ros::Time last_request = ros::Time::now();
    ROS_INFO("Misson Received");
    while(ros::ok()){

	//r = double(vec.vector.x);
	//wn = double(vec.vector.y);
	theta = wn*count*0.05;


    	pose.pose.position.x = r*sin(theta);
    	pose.pose.position.y = r*cos(theta);
    	pose.pose.position.z = z;


	ROS_INFO("Target Position");
	ROS_INFO("r : [%lf]",r);
  	ROS_INFO("wn : [%lf]",wn);
	ROS_INFO("x : [%lf]",pose.pose.position.x);
	ROS_INFO("y : [%lf]",pose.pose.position.y);
	ROS_INFO("z : [%lf]",pose.pose.position.z);
	count++;

        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
