// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>
#include <carla/image/ImageConverter.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/pointcloud/PointCloudIO.h>
#include <carla/sensor/SensorData.h>
#include <carla/sensor/data/CollisionEvent.h>
#include <carla/sensor/data/IMUMeasurement.h>
#include <carla/sensor/data/ObstacleDetectionEvent.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LaneInvasionEvent.h>
#include <carla/sensor/data/LidarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/data/DVSEventArray.h>

#include <carla/sensor/s11n/RadarData.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <ostream>
#include <iostream>

namespace carla {
namespace sensor {
namespace data {

  std::ostream &operator<<(std::ostream &out, const Image &image) {
    out << "Image(frame=" << std::to_string(image.GetFrame())
        << ", timestamp=" << std::to_string(image.GetTimestamp())
        << ", size=" << std::to_string(image.GetWidth()) << 'x' << std::to_string(image.GetHeight())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LidarMeasurement &meas) {
    out << "LidarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", number_of_points=" << std::to_string(meas.size())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const CollisionEvent &meas) {
    out << "CollisionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const ObstacleDetectionEvent &meas) {
    out << "ObstacleDetectionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", other_actor=" << meas.GetOtherActor()
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const LaneInvasionEvent &meas) {
    out << "LaneInvasionEvent(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const GnssMeasurement &meas) {
    out << "GnssMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", lat=" << std::to_string(meas.GetLatitude())
        << ", lon=" << std::to_string(meas.GetLongitude())
        << ", alt=" << std::to_string(meas.GetAltitude())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const IMUMeasurement &meas) {
    out << "IMUMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", accelerometer=" << meas.GetAccelerometer()
        << ", gyroscope=" << meas.GetGyroscope()
        << ", compass=" << std::to_string(meas.GetCompass())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const RadarMeasurement &meas) {
    out << "RadarMeasurement(frame=" << std::to_string(meas.GetFrame())
        << ", timestamp=" << std::to_string(meas.GetTimestamp())
        << ", point_count=" << std::to_string(meas.GetDetectionAmount())
        << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const DVSEvent &event) {
    out << "Event(" << event.x
        << ',' << event.y
        << ',' << event.t
        << ',' << event.pol << ')';
    return out;
  }

  std::ostream &operator<<(std::ostream &out, const DVSEventArray &events) {
    out << "EventArray(frame=" << events.GetFrame()
        << ", timestamp=" << events.GetTimestamp()
        << ", dimensions=" << events.GetWidth() << 'x' << events.GetHeight()
        << ", number_of_events=" << events.size()
        << ')';
    return out;
  }

} // namespace data

namespace s11n {

  std::ostream &operator<<(std::ostream &out, const RadarDetection &det) {
    out << "RadarDetection(velocity=" << std::to_string(det.velocity)
        << ", azimuth=" << std::to_string(det.azimuth)
        << ", altitude=" << std::to_string(det.altitude)
        << ", depth=" << std::to_string(det.depth)
        << ')';
    return out;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla

enum class EColorConverter {
  Raw,
  Depth,
  LogarithmicDepth,
  CityScapesPalette
};

template <typename T>
static auto GetRawDataAsBuffer(T &self) {
  auto *data = reinterpret_cast<unsigned char *>(self.data());
  auto size = static_cast<Py_ssize_t>(sizeof(typename T::value_type) * self.size());
#if PY_MAJOR_VERSION >= 3
  auto *ptr = PyMemoryView_FromMemory(reinterpret_cast<char *>(data), size, PyBUF_READ);
#else
  auto *ptr = PyBuffer_FromMemory(data, size);
#endif
  return boost::python::object(boost::python::handle<>(ptr));
}

template <typename T>
static void ConvertImage(T &self, EColorConverter cc) {
  carla::PythonUtil::ReleaseGIL unlock;
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::Depth:
      ImageConverter::ConvertInPlace(view, ColorConverter::Depth());
      break;
    case EColorConverter::LogarithmicDepth:
      ImageConverter::ConvertInPlace(view, ColorConverter::LogarithmicDepth());
      break;
    case EColorConverter::CityScapesPalette:
      ImageConverter::ConvertInPlace(view, ColorConverter::CityScapesPalette());
      break;
    case EColorConverter::Raw:
      break; // ignore.
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static std::string SaveImageToDisk(T &self, std::string path, EColorConverter cc) {
  carla::PythonUtil::ReleaseGIL unlock;
  using namespace carla::image;
  auto view = ImageView::MakeView(self);
  switch (cc) {
    case EColorConverter::Raw:
      return ImageIO::WriteView(
          std::move(path),
          view);
    case EColorConverter::Depth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::Depth()));
    case EColorConverter::LogarithmicDepth:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::LogarithmicDepth()));
    case EColorConverter::CityScapesPalette:
      return ImageIO::WriteView(
          std::move(path),
          ImageView::MakeColorConvertedView(view, ColorConverter::CityScapesPalette()));
    default:
      throw std::invalid_argument("invalid color converter!");
  }
}

template <typename T>
static std::string SavePointCloudToDisk(T &self, std::string path) {
  carla::PythonUtil::ReleaseGIL unlock;
  return carla::pointcloud::PointCloudIO::SaveToDisk(std::move(path), self.begin(), self.end());
}

void export_sensor_data() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace cr = carla::rpc;
  namespace cs = carla::sensor;
  namespace csd = carla::sensor::data;
  namespace css = carla::sensor::s11n;

  class_<cs::SensorData, boost::noncopyable, boost::shared_ptr<cs::SensorData>>("SensorData", no_init)
    .add_property("frame", &cs::SensorData::GetFrame)
    .add_property("frame_number", &cs::SensorData::GetFrame) // deprecated.
    .add_property("timestamp", &cs::SensorData::GetTimestamp)
    .add_property("transform", CALL_RETURNING_COPY(cs::SensorData, GetSensorTransform))
  ;

  enum_<EColorConverter>("ColorConverter")
    .value("Raw", EColorConverter::Raw)
    .value("Depth", EColorConverter::Depth)
    .value("LogarithmicDepth", EColorConverter::LogarithmicDepth)
    .value("CityScapesPalette", EColorConverter::CityScapesPalette)
  ;

  class_<csd::Image, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::Image>>("Image", no_init)
    .add_property("width", &csd::Image::GetWidth)
    .add_property("height", &csd::Image::GetHeight)
    .add_property("fov", &csd::Image::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::Image>)
    .def("convert", &ConvertImage<csd::Image>, (arg("color_converter")))
    .def("save_to_disk", &SaveImageToDisk<csd::Image>, (arg("path"), arg("color_converter")=EColorConverter::Raw))
    .def("__len__", &csd::Image::size)
    .def("__iter__", iterator<csd::Image>())
    .def("__getitem__", +[](const csd::Image &self, size_t pos) -> csd::Color {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::Image &self, size_t pos, csd::Color color) {
      self.at(pos) = color;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LidarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LidarMeasurement>>("LidarMeasurement", no_init)
    .add_property("horizontal_angle", &csd::LidarMeasurement::GetHorizontalAngle)
    .add_property("channels", &csd::LidarMeasurement::GetChannelCount)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::LidarMeasurement>)
    .def("get_point_count", &csd::LidarMeasurement::GetPointCount, (arg("channel")))
    .def("save_to_disk", &SavePointCloudToDisk<csd::LidarMeasurement>, (arg("path")))
    .def("__len__", &csd::LidarMeasurement::size)
    .def("__iter__", iterator<csd::LidarMeasurement>())
    .def("__getitem__", +[](const csd::LidarMeasurement &self, size_t pos) -> cr::Location {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::LidarMeasurement &self, size_t pos, const cr::Location &point) {
      self.at(pos) = point;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::CollisionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::CollisionEvent>>("CollisionEvent", no_init)
    .add_property("actor", &csd::CollisionEvent::GetActor)
    .add_property("other_actor", &csd::CollisionEvent::GetOtherActor)
    .add_property("normal_impulse", CALL_RETURNING_COPY(csd::CollisionEvent, GetNormalImpulse))
    .def(self_ns::str(self_ns::self))
  ;

    class_<csd::ObstacleDetectionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::ObstacleDetectionEvent>>("ObstacleDetectionEvent", no_init)
    .add_property("actor", &csd::ObstacleDetectionEvent::GetActor)
    .add_property("other_actor", &csd::ObstacleDetectionEvent::GetOtherActor)
    .add_property("distance", CALL_RETURNING_COPY(csd::ObstacleDetectionEvent, GetDistance))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::LaneInvasionEvent, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::LaneInvasionEvent>>("LaneInvasionEvent", no_init)
    .add_property("actor", &csd::LaneInvasionEvent::GetActor)
    .add_property("crossed_lane_markings", CALL_RETURNING_LIST(csd::LaneInvasionEvent, GetCrossedLaneMarkings))
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::GnssMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::GnssMeasurement>>("GnssMeasurement", no_init)
    .add_property("latitude", &csd::GnssMeasurement::GetLatitude)
    .add_property("longitude", &csd::GnssMeasurement::GetLongitude)
    .add_property("altitude", &csd::GnssMeasurement::GetAltitude)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::IMUMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::IMUMeasurement>>("IMUMeasurement", no_init)
    .add_property("accelerometer", &csd::IMUMeasurement::GetAccelerometer)
    .add_property("gyroscope", &csd::IMUMeasurement::GetGyroscope)
    .add_property("compass", &csd::IMUMeasurement::GetCompass)
    .def(self_ns::str(self_ns::self))
  ;

  class_<csd::RadarMeasurement, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::RadarMeasurement>>("RadarMeasurement", no_init)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::RadarMeasurement>)
    .def("get_detection_count", &csd::RadarMeasurement::GetDetectionAmount)
    .def("__len__", &csd::RadarMeasurement::size)
    .def("__iter__", iterator<csd::RadarMeasurement>())
    .def("__getitem__", +[](const csd::RadarMeasurement &self, size_t pos) -> css::RadarDetection {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::RadarMeasurement &self, size_t pos, const css::RadarDetection &detection) {
      self.at(pos) = detection;
    })
    .def(self_ns::str(self_ns::self))
  ;

  class_<css::RadarDetection>("RadarDetection")
    .def_readwrite("velocity", &css::RadarDetection::velocity)
    .def_readwrite("azimuth", &css::RadarDetection::azimuth)
    .def_readwrite("altitude", &css::RadarDetection::altitude)
    .def_readwrite("depth", &css::RadarDetection::depth)
    .def(self_ns::str(self_ns::self))
  ;

    class_<std::vector<csd::Color> >("ColorVector")
    .add_property("raw_data", &GetRawDataAsBuffer< std::vector<csd::Color> >)
    .def("__len__", &std::vector<csd::Color>::size)
    .def(vector_indexing_suite< std::vector<csd::Color> >())
  ;

  class_<std::vector<std::int64_t>>("IntVector")
    .add_property("raw_data", &GetRawDataAsBuffer< std::vector<std::int64_t> >)
    .def("__len__", &std::vector<std::int64_t>::size)
    .def(vector_indexing_suite< std::vector<std::int64_t> >())
  ;

  class_<std::vector<short>>("ShortVector")
    .add_property("raw_data", &GetRawDataAsBuffer< std::vector<short> >)
    .def("__len__", &std::vector<short>::size)
    .def(vector_indexing_suite< std::vector<short> >())
  ;

  class_<std::vector<std::vector<std::int64_t>> >("IntMatrix")
    .add_property("raw_data", &GetRawDataAsBuffer< std::vector<std::vector<std::int64_t>> >)
    .def("__len__", &std::vector<std::vector<std::int64_t>>::size)
    .def(vector_indexing_suite< std::vector<std::vector<std::int64_t>> >())
  ;

  class_<csd::DVSEvent>("DVSEvent")
    .add_property("x", &csd::DVSEvent::x)
    .add_property("y", &csd::DVSEvent::y)
    .add_property("t", &csd::DVSEvent::t)
    .add_property("pol", &csd::DVSEvent::pol)
    .def(self_ns::str(self_ns::self))
  ;
  
  class_<csd::DVSEventArray, bases<cs::SensorData>, boost::noncopyable, boost::shared_ptr<csd::DVSEventArray>>("DVSEventArray", no_init)
    .add_property("width", &csd::DVSEventArray::GetWidth)
    .add_property("height", &csd::DVSEventArray::GetHeight)
    .add_property("fov", &csd::DVSEventArray::GetFOVAngle)
    .add_property("raw_data", &GetRawDataAsBuffer<csd::DVSEventArray>)
    .def("__len__", &csd::DVSEventArray::size)
    .def("__iter__", iterator<csd::DVSEventArray>())
    .def("__getitem__", +[](const csd::DVSEventArray &self, size_t pos) -> csd::DVSEvent {
      return self.at(pos);
    })
    .def("__setitem__", +[](csd::DVSEventArray &self, size_t pos, csd::DVSEvent event) {
      self.at(pos) = event;
    })
    .def("to_image", +[](const csd::DVSEventArray &self) -> std::vector<csd::Color> {
      std::vector<csd::Color> img (self.GetHeight() * self.GetWidth());
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        size_t index = (self.GetWidth() * event.y) + event.x;
        if (event.pol == true)
        {
          /** Blue is positive **/
          img[index].b = 255u;
        }
        else
        {
          /** Red is negative **/
          img[index].r = 255u;
        }
     }
      return img;
    })
    .def("to_array", +[](const csd::DVSEventArray &self) -> std::vector<std::vector<std::int64_t>> {
      std::vector<std::vector<std::int64_t>> array (self.size());
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        std::vector<std::int64_t> element = {static_cast<std::int64_t>(event.x), static_cast<std::int64_t>(event.y), static_cast<std::int64_t>(event.t), static_cast<std::int64_t>(event.pol)};
        array[i] = element;
      }
      return array;
    })

    .def("to_array_x", +[](const csd::DVSEventArray &self) -> std::vector<std::int64_t> {
      std::vector<std::int64_t> array;
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        array.push_back(event.x);
      }
      return array;
    })

     .def("to_array_y", +[](const csd::DVSEventArray &self) -> std::vector<std::int64_t> {
      std::vector<std::int64_t> array;
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        array.push_back(event.y);
      }
      return array;
    })
      .def("to_array_t", +[](const csd::DVSEventArray &self) -> std::vector<std::int64_t> {
      std::vector<std::int64_t> array;
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        array.push_back(event.t);
      }
      return array;
    })

      .def("to_array_pol", +[](const csd::DVSEventArray &self) -> std::vector<short> {
      std::vector<short> array;
      for (size_t i=0; i<self.size(); ++i)
      {
        const csd::DVSEvent &event = self[i];
        if (event.pol == true)
          array.push_back(1);
        else
          array.push_back(-1);

      }
      return array;
    })
    .def(self_ns::str(self_ns::self))
  ;
}
