/*
 * Author: Nikita Khalyavin
 */

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int32.h"
#include <sstream>
#include <string>
#include <map>
#include <limits>


//these global variable is necessary for using of 
//g_publisher from different flows (main and callback getMessageCallback)
static ros::Publisher g_publisher;


void getMessageCallback(const std_msgs::String::ConstPtr& msg)
{
 /*
  * вообще говоря, задача "вывести расстояние от планеты до Земли" очень некорректна, 
  * поскольку, во-первых, не указана требуемая точность, а во-вторых, это расстояние вообще
  * разное в разное время, и не указано, выводить ли расстояния на текущий момент или 
  * в какой-то конкретный момент. В результате я вывел минимальное расстояние до Земли, 
  * поскольку наличия интернета для выяснения текущего расстояния из открытых источников никто 
  * не гаранировал, а создавать виртуальную модель солнечной системы только для того, чтобы
  * расчитывать положение планет в текущий момент времени как-то сложненько. В целом, эта
  * трактовка не противоречит заданию, так что всё ок 
  */
  
  //prepare dictionary of planets and distances
  static std::map<std::string, int64_t> planet_distances_km;
  planet_distances_km["Mercury"] =   91600000;
  planet_distances_km["Venus"]   =   41400000;
  planet_distances_km["Earth"]   =          0;
  planet_distances_km["Mars"]    =   78300000;
  planet_distances_km["Jupiter"] =  628400000;
  planet_distances_km["Saturn"]  = 1277400000;
  planet_distances_km["Uranus"]  = 2721400000;
  planet_distances_km["Neptune"]  = 4347400000;

  //get distance by planet name
  std::string received_planet_name = msg->data;
  std::map<std::string, int64_t> :: iterator planet_iterator = planet_distances_km.find(received_planet_name);

 /*
  * к сожалению, расстояние до Урана и Нептуна в заданных единицах
  * (километры) не может быть записано в переменную заданного
  * формата (int32), но единственное что можно сделать в данной
  * ситуации, что не противоречит заданию - вывести предупреждение
  */

  //check if can't find such planet
  if(planet_iterator == planet_distances_km.end())
  {
    ROS_INFO("ERROR: unknown planet name");
    return;
  }

  //check if the distance in kms is larger than the maximum value of output variable
  const int64_t MAX_OUTPUT_VALUE= std::numeric_limits<std::int32_t>::max();
  if(planet_iterator->second > MAX_OUTPUT_VALUE)
  {
    ROS_INFO("WARNING: output variable is overflowed");
  }

  //prepare and send response
  std_msgs::Int32 response;
  response.data = (int32_t)planet_iterator->second;
  g_publisher.publish(response);
}


int main(int argc, char **argv)
{

  //prepare node
  ros::init(argc, argv, "first_node"); 
  ros::NodeHandle n = ros::NodeHandle();

  //init publisher
  const int PUBLISHER_MESSAGE_QUEUE_SIZE = 10;
  const std::string PUBLISHER_TOPIC_NAME = "distanceToEarth";
  g_publisher = n.advertise<std_msgs::Int32>(PUBLISHER_TOPIC_NAME, PUBLISHER_MESSAGE_QUEUE_SIZE);

  //init subscriber
  const int SUBSCRIBER_MESSAGE_QUEUE_SIZE = 10;
  const std::string SUBSCRIBER_TOPIC_NAME = "planet";
  ros::Subscriber subscriber = n.subscribe(SUBSCRIBER_TOPIC_NAME, SUBSCRIBER_MESSAGE_QUEUE_SIZE, getMessageCallback);

  ROS_INFO("node first_node is ready");
  ros::spin();

  return 0;
}
