#pragma once

#include <array>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/number/number.h"
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

class CB19CalibrationNumber : public number::Number {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }
  void set_initial_value(float value) { this->initial_value_ = value; }
  float get_initial_value() const { return this->initial_value_; }

 protected:
  CB19GateComponent *parent_{nullptr};
  float initial_value_{0.0f};
};

class CB19OpeningStartNumber : public CB19CalibrationNumber {
 protected:
  void control(float value) override;
};

class CB19ClosingStartNumber : public CB19CalibrationNumber {
 protected:
  void control(float value) override;
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

  void set_opening_start_number(CB19CalibrationNumber *n) { this->opening_start_number_ = n; }
  void set_closing_start_number(CB19CalibrationNumber *n) { this->closing_start_number_ = n; }

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

  float get_opening_start_percent() const { return this->opening_start_percent_; }
  float get_closing_start_percent() const { return this->closing_start_percent_; }
  void set_opening_start_percent(float value);
  void set_closing_start_percent(float value);

 protected:
  void send_command_(const std::string &cmd);
  void parse_line_(const std::string &line);
  bool parse_rs_frame_(const std::string &payload, std::array<uint8_t, 9> &out);
  void apply_rs_frame_(const std::array<uint8_t, 9> &frame, const std::string &raw_line);
  void apply_state_line_(const std::string &line);
  std::string extract_protocol_state_(const std::string &line);
  void publish_all_();
  float scale_position_fallback_(uint8_t raw) const;
  float scale_motor_position_(uint8_t raw, bool closed_valid, uint8_t closed_ref, bool open_valid, uint8_t open_ref) const;
  void recalculate_positions_();
  float apply_cover_calibration_(float base_percent) const;
  void learn_current_refs_from_state_(const std::string &state);
  std::string motion_state_to_string_(GateMotionState state) const;
  bool is_moving_state_(GateMotionState state) const;
  void set_motion_state_(GateMotionState state);
  void maybe_poll_rs_();
  uint32_t get_poll_interval_ms_() const;

  std::string buffer_;
  bool has_frame_{false};
  std::array<uint8_t, 9> last_frame_{};

  uint8_t min_position_{1};
  uint8_t max_position_{225};

  uint8_t motor1_raw_{0};
  uint8_t motor2_raw_{0};
  float motor1_percent_{0.0f};
  float motor2_percent_{0.0f};
  float overall_percent_raw_{0.0f};
  float overall_percent_{0.0f};

  bool motor1_closed_ref_valid_{false};
  bool motor1_open_ref_valid_{false};
  bool motor2_closed_ref_valid_{false};
  bool motor2_open_ref_valid_{false};

  uint8_t motor1_closed_ref_{0};
  uint8_t motor1_open_ref_{0};
  uint8_t motor2_closed_ref_{0};
  uint8_t motor2_open_ref_{0};

  float opening_start_percent_{56.0f};
  float closing_start_percent_{44.0f};

  bool photocell_active_{false};
  bool obstruction_active_{false};
  GateMotionState motion_state_{GateMotionState::UNKNOWN};

  std::string last_state_line_;
  std::string last_ack_line_;
  std::string last_rs_line_;

  uint32_t last_poll_time_{0};
  uint32_t last_motion_change_time_{0};
  uint32_t suppress_poll_until_{0};

  CB19GateCover *cover_{nullptr};
  CB19PedestrianButton *pedestrian_button_{nullptr};
  CB19CalibrationNumber *opening_start_number_{nullptr};
  CB19CalibrationNumber *closing_start_number_{nullptr};

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