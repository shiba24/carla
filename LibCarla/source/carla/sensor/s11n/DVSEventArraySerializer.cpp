// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/DVSEventArraySerializer.h"

#include "carla/sensor/data/DVSEventArray.h"

namespace carla {
namespace sensor {
namespace s11n {

  SharedPtr<SensorData> DVSEventArraySerializer::Deserialize(RawData &&data) {

    auto events_array = SharedPtr<data::DVSEventArray>(new data::DVSEventArray{std::move(data)});

    //std::cout<<"DESERIALIZER Size"<<(*events_array).size()<<" sizeof(Event):"<<sizeof(data::DVSEvent) <<" Frame:"<<(*events_array).GetFrame()<<" t:"<<(*events_array).GetTimestamp()<<std::endl;
    //std::cout<<"[0]:"<<(*events_array)[0].x<<","<<(*events_array)[0].y<<","<<(*events_array)[0].t<<","<<(*events_array)[0].pol<<std::endl;
    //std::cout<<"[1]:"<<(*events_array)[1].x<<","<<(*events_array)[1].y<<","<<(*events_array)[1].t<<","<<(*events_array)[1].pol<<std::endl;
    //std::cout<<"[2]:"<<(*events_array)[2].x<<","<<(*events_array)[2].y<<","<<(*events_array)[2].t<<","<<(*events_array)[2].pol<<std::endl;
    //std::cout<<"["<<(*events_array).size()-3<<"]:"<<(*events_array)[(*events_array).size()-3].x<<","<<(*events_array)[(*events_array).size()-3].y<<","<<(*events_array)[(*events_array).size()-3].t<<","<<(*events_array)[(*events_array).size()-3].pol<<std::endl;
    //std::cout<<"["<<(*events_array).size()-2<<"]:"<<(*events_array)[(*events_array).size()-2].x<<","<<(*events_array)[(*events_array).size()-2].y<<","<<(*events_array)[(*events_array).size()-2].t<<","<<(*events_array)[(*events_array).size()-2].pol<<std::endl;
    //std::cout<<"["<<(*events_array).size()-1<<"]:"<<(*events_array)[(*events_array).size()-1].x<<","<<(*events_array)[(*events_array).size()-1].y<<","<<(*events_array)[(*events_array).size()-1].t<<","<<(*events_array)[(*events_array).size()-1].pol<<std::endl;
    return events_array;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
