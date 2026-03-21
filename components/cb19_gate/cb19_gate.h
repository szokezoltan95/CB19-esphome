#pragma once

#include <array>
#include <string>
#include <vector>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace cb19_gate {

class CB19GateComponent;

enum class GateMotionState : uint8_t {
  UNKNOWN = 0,
  OPENING,
  OPENED,
  CLOSING,
  CLOSED,
  STOPPED,
  PED_OPENING,
  PED_OPENED
};

class CB19GateCover : public cover::Cover {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }
  cover::CoverTraits get_traits() override;
  void control(const cover::CoverCall &call) override;
  void sync_from_parent();

 protected:
  CB19GateComponent *parent_{nullptr};
};

class CB19PedestrianButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19GateComponent : public Component, public uart::UARTDevice {
 public:
  explicit CB19GateComponent(uart::UARTComponent *parent) : uart::UARTDevice(parent) {}

  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_position_range(uint8_t min_pos, uint8_t max_pos) {
    this->min_position_ = min_pos;
    this->max_position_ = max_pos;
  }

  void set_cover(CB19GateCover *cover) { this->cover_ = cover; }
  void set_pedestrian_button(CB19PedestrianButton *button) { this->pedestrian_button_ = button; }

  void set_motor1_raw_sensor(sensor::Sensor *s) { this->motor1_raw_sensor_ = s; }
  void set_motor2_raw_sensor(sensor::Sensor *s) { this->motor2_raw_sensor_ = s; }
  void set_motor1_percent_sensor(sensor::Sensor *s) { this->motor1_percent_sensor_ = s; }
  void set_motor2_percent_sensor(sensor::Sensor *s) { this->motor2_percent_sensor_ = s; }
  void set_overall_percent_sensor(sensor::Sensor *s) { this->overall_percent_sensor_ = s; }

  void set_last_state_text_sensor(text_sensor::TextSensor *s) { this->last_state_text_sensor_ = s; }
  void set_last_ack_text_sensor(text_sensor::TextSensor *s) { this->last_ack_text_sensor_ = s; }
  void set_last_rs_text_sensor(text_sensor::TextSensor *s) { this->last_rs_text_sensor_ = s; }

  void set_moving_binary_sensor(binary_sensor::BinarySensor *s) { this->moving_binary_sensor_ = s; }
  void set_fully_open_binary_sensor(binary_sensor::BinarySensor *s) { this->fully_open_binary_sensor_ = s; }
  void set_fully_closed_binary_sensor(binary_sensor::BinarySensor *s) { this->fully_closed_binary_sensor_ = s; }
  void set_photocell_binary_sensor(binary_sensor::BinarySensor *s) { this->photocell_binary_sensor_ = s; }
  void set_obstruction_binary_sensor(binary_sensor::BinarySensor *s) { this->obstruction_binary_sensor_ = s; }

  void open_gate();
  void close_gate();
  void stop_gate();
  void pedestrian_open();

  float get_overall_position_percent() const { return this->overall_percent_; }
  GateMotionState get_motion_state() const { return this->motion_state_; }

 protected:
  void send_command_(const std::string &cmd);
  void parse_line_(const std::string &line);
  bool parse_rs_frame_(const std::string &payload, std::array<uint8_t, 9> &out);
  void apply_rs_frame_(const std::array<uint8_t, 9> &frame, const std::string &raw_line);
  void apply_state_line_(const std::string &line);
  void publish_all_();
  float scale_position_(uint8_t raw) const;
  std::string motion_state_to_string_(GateMotionState state) const;

  std::string buffer_;
  bool has_frame_{false};
  std::array<uint8_t, 9> last_frame_{};

  uint8_t min_position_{1};
  uint8_t max_position_{225};

  uint8_t motor1_raw_{0};
  uint8_t motor2_raw_{0};
  float motor1_percent_{0.0f};
  float motor2_percent_{0.0f};
  float overall_percent_{0.0f};

  bool photocell_active_{false};
  bool obstruction_active_{false};
  GateMotionState motion_state_{GateMotionState::UNKNOWN};

  std::string last_state_line_;
  std::string last_ack_line_;
  std::string last_rs_line_;

  CB19GateCover *cover_{nullptr};
  CB19PedestrianButton *pedestrian_button_{nullptr};

  sensor::Sensor *motor1_raw_sensor_{nullptr};
  sensor::Sensor *motor2_raw_sensor_{nullptr};
  sensor::Sensor *motor1_percent_sensor_{nullptr};
  sensor::Sensor *motor2_percent_sensor_{nullptr};
  sensor::Sensor *overall_percent_sensor_{nullptr};

  text_sensor::TextSensor *last_state_text_sensor_{nullptr};
  text_sensor::TextSensor *last_ack_text_sensor_{nullptr};
  text_sensor::TextSensor *last_rs_text_sensor_{nullptr};

  binary_sensor::BinarySensor *moving_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fully_open_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fully_closed_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *photocell_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *obstruction_binary_sensor_{nullptr};
};

}  // namespace cb19_gate
}  // namespace esphome
