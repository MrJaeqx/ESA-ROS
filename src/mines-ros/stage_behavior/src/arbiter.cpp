#include <ros/ros.h>
#include <geometry_msgs/Twist.h>

struct TwistWrapper {
  int prio;
  geometry_msgs::Twist vel;
};

/**
 * Priority based arbiter, forwards highest priority commands
 */
class Arbiter {
public:
  // members

  // methods
  Arbiter();
  
protected:
  // members
  int rate_;
  ros::NodeHandle nh_;
  ros::Publisher cmd_vel_pub_;
  ros::Subscriber cmd_vel0_, cmd_vel1_, cmd_vel2_, cmd_vel3_;
  ros::Timer timer_;

  // methods
  void update();
  void updatePrioMsg(const geometry_msgs::Twist::ConstPtr& cmd, int prio);
  void cmdCallback0(const geometry_msgs::Twist::ConstPtr& cmd);
  void cmdCallback1(const geometry_msgs::Twist::ConstPtr& cmd);
  void cmdCallback2(const geometry_msgs::Twist::ConstPtr& cmd);
  void cmdCallback3(const geometry_msgs::Twist::ConstPtr& cmd);

  TwistWrapper prioMsg_;
};

///////////////////////////////////////////////////////////////////////////

Arbiter::Arbiter() : rate_(100) {
  cmd_vel0_ = nh_.subscribe<geometry_msgs::Twist>("cmd_vel0", 100, &Arbiter::cmdCallback0, this);
  cmd_vel1_ = nh_.subscribe<geometry_msgs::Twist>("cmd_vel1", 100, &Arbiter::cmdCallback1, this);
  cmd_vel2_ = nh_.subscribe<geometry_msgs::Twist>("cmd_vel2", 100, &Arbiter::cmdCallback2, this);
  cmd_vel3_ = nh_.subscribe<geometry_msgs::Twist>("cmd_vel3", 100, &Arbiter::cmdCallback3, this);

  cmd_vel_pub_ = nh_.advertise<geometry_msgs::Twist>("cmd_vel", 1);
  timer_ = nh_.createTimer(ros::Duration(1.0/rate_), boost::bind(&Arbiter::update, this));
}

void Arbiter::update() {
  if (prioMsg_.prio != -1) {
    cmd_vel_pub_.publish(prioMsg_.vel);
    prioMsg_.prio = -1;
  }
}

void Arbiter::updatePrioMsg(const geometry_msgs::Twist::ConstPtr& cmd, int prio) {
  if (prioMsg_.prio == -1 || prioMsg_.prio >= prio) {
    prioMsg_.vel = *cmd;
    prioMsg_.prio = prio;
  }
}

void Arbiter::cmdCallback0(const geometry_msgs::Twist::ConstPtr& cmd) {
  updatePrioMsg(cmd, 0);
}

void Arbiter::cmdCallback1(const geometry_msgs::Twist::ConstPtr& cmd) {
  updatePrioMsg(cmd, 1);
}

void Arbiter::cmdCallback2(const geometry_msgs::Twist::ConstPtr& cmd) {
  updatePrioMsg(cmd, 2);
}

void Arbiter::cmdCallback3(const geometry_msgs::Twist::ConstPtr& cmd) {
  updatePrioMsg(cmd, 3);
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "arbiter");
  Arbiter arbiter;
  ros::spin();

  return 0;
}
