#pragma once

#include <array>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
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
  PED_OPENED,
  AUTO_CLOSING
};

enum class LearnMode : uint8_t {
  NONE = 0,
  AUTO_LEARN,
  REMOTE_LEARN,
  CLEAR_REMOTE
};

class CB19PedestrianButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19ApplyParametersButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19ReloadParametersButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19RevertParametersButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19FactoryResetButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19AutoLearnButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19RemoteLearnButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19ClearRemoteLearnButton : public button::Button {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }

 protected:
  void press_action() override;
  CB19GateComponent *parent_{nullptr};
};

class CB19ParameterSelect : public select::Select {
 public:
  void set_parent(CB19GateComponent *parent) { this->parent_ = parent; }
  void set_parameter_index(uint8_t index) { this->parameter_index_ = index; }

 protected:
  void control(const std::string &value) override;

  CB19GateComponent *parent_{nullptr};
  uint8_t parameter_index_{0};
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

  void set_pedestrian_button(CB19PedestrianButton *button) { this->pedestrian_button_ = button; }
  void set_apply_parameters_button(CB19ApplyParametersButton *button) { this->apply_parameters_button_ = button; }
  void set_reload_parameters_button(CB19ReloadParametersButton *button) { this->reload_parameters_button_ = button; }
  void set_revert_parameters_button(CB19RevertParametersButton *button) { this->revert_parameters_button_ = button; }
  void set_factory_reset_button(CB19FactoryResetButton *button) { this->factory_reset_button_ = button; }
  void set_auto_learn_button(CB19AutoLearnButton *button) { this->auto_learn_button_ = button; }
  void set_remote_learn_button(CB19RemoteLearnButton *button) { this->remote_learn_button_ = button; }
  void set_clear_remote_learn_button(CB19ClearRemoteLearnButton *button) { this->clear_remote_learn_button_ = button; }
  void set_parameter_select(uint8_t index, CB19ParameterSelect *sel) {
    if (index < this->parameter_selects_.size()) {
      this->parameter_selects_[index] = sel;
    }
  }

  void set_opening_start_number(CB19CalibrationNumber *n) { this->opening_start_number_ = n; }
  void set_closing_start_number(CB19CalibrationNumber *n) { this->closing_start_number_ = n; }

  void set_motor1_raw_sensor(sensor::Sensor *s) { this->motor1_raw_sensor_ = s; }
  void set_motor2_raw_sensor(sensor::Sensor *s) { this->motor2_raw_sensor_ = s; }
  void set_motor1_position_sensor(sensor::Sensor *s) { this->motor1_position_sensor_ = s; }
  void set_motor2_position_sensor(sensor::Sensor *s) { this->motor2_position_sensor_ = s; }
  void set_gate_position_sensor(sensor::Sensor *s) { this->gate_position_sensor_ = s; }
  void set_motor1_speed_sensor(sensor::Sensor *s) { this->motor1_speed_sensor_ = s; }
  void set_motor1_load_sensor(sensor::Sensor *s) { this->motor1_load_sensor_ = s; }
  void set_motor2_speed_sensor(sensor::Sensor *s) { this->motor2_speed_sensor_ = s; }
  void set_motor2_load_sensor(sensor::Sensor *s) { this->motor2_load_sensor_ = s; }

  void set_gate_state_text_sensor(text_sensor::TextSensor *s) { this->gate_state_text_sensor_ = s; }
  void set_last_ack_text_sensor(text_sensor::TextSensor *s) { this->last_ack_text_sensor_ = s; }
  void set_last_rs_text_sensor(text_sensor::TextSensor *s) { this->last_rs_text_sensor_ = s; }
  void set_learn_status_text_sensor(text_sensor::TextSensor *s) { this->learn_status_text_sensor_ = s; }
  void set_param_current_text_sensor(text_sensor::TextSensor *s) { this->param_current_text_sensor_ = s; }
  void set_param_pending_text_sensor(text_sensor::TextSensor *s) { this->param_pending_text_sensor_ = s; }
  void set_config_warning_text_sensor(text_sensor::TextSensor *s) { this->config_warning_text_sensor_ = s; }

  void set_moving_binary_sensor(binary_sensor::BinarySensor *s) { this->moving_binary_sensor_ = s; }
  void set_fully_opened_binary_sensor(binary_sensor::BinarySensor *s) { this->fully_opened_binary_sensor_ = s; }
  void set_fully_closed_binary_sensor(binary_sensor::BinarySensor *s) { this->fully_closed_binary_sensor_ = s; }
  void set_ped_opened_binary_sensor(binary_sensor::BinarySensor *s) { this->ped_opened_binary_sensor_ = s; }
  void set_manual_stop_binary_sensor(binary_sensor::BinarySensor *s) { this->manual_stop_binary_sensor_ = s; }
  void set_photocell_binary_sensor(binary_sensor::BinarySensor *s) { this->photocell_binary_sensor_ = s; }
  void set_obstruction_binary_sensor(binary_sensor::BinarySensor *s) { this->obstruction_binary_sensor_ = s; }
  void set_params_dirty_binary_sensor(binary_sensor::BinarySensor *s) { this->params_dirty_binary_sensor_ = s; }
  void set_learning_active_binary_sensor(binary_sensor::BinarySensor *s) { this->learning_active_binary_sensor_ = s; }

  void open_gate();
  void close_gate();
  void stop_gate();
  void pedestrian_open();

  void request_param_read();
  void apply_pending_parameters();
  void revert_pending_parameters();
  void start_factory_reset();
  void start_auto_learn();
  void start_remote_learn();
  void start_clear_remote_learn();
  void set_pending_parameter_from_option(uint8_t index, const std::string &option);

  float get_gate_position_percent() const { return this->gate_position_; }
  GateMotionState get_motion_state() const { return this->motion_state_; }
  float get_opening_start_percent() const { return this->opening_start_percent_; }
  float get_closing_start_percent() const { return this->closing_start_percent_; }
  void set_opening_start_percent(float value);
  void set_closing_start_percent(float value);

 protected:
  void set_stop_command_pending_();
  void clear_stop_context_();
  void update_status_flags_();
  void send_command_(const std::string &cmd);
  void parse_line_(const std::string &line);
  bool parse_rs_frame_(const std::string &payload, std::array<uint8_t, 9> &out);
  void apply_rs_frame_(const std::array<uint8_t, 9> &frame, const std::string &raw_line);
  void apply_state_line_(const std::string &line);
  std::string extract_protocol_state_(const std::string &line);

  bool parse_param_block_(const std::string &payload, std::array<uint8_t, 20> &out) const;
  std::string build_wp1_command_() const;
  void apply_param_block_(const std::array<uint8_t, 20> &values, bool update_pending);
  void publish_parameter_entities_();
  std::string format_param_block_(const std::array<uint8_t, 20> &values) const;
  int parse_raw_value_from_option_(const std::string &option) const;
  void update_config_warning_();

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
  void maybe_sync_parameters_();
  void maybe_poll_learn_status_();
  void handle_learn_status_ack_(const std::string &payload);
  void set_learn_status_(const std::string &status, bool active);

  std::string buffer_;
  bool has_frame_{false};
  std::array<uint8_t, 9> last_frame_{};
  uint8_t min_position_{1};
  uint8_t max_position_{225};
  uint8_t motor1_raw_{0};
  uint8_t motor2_raw_{0};
  float motor1_position_{0.0f};
  float motor2_position_{0.0f};
  float gate_position_raw_{0.0f};
  float gate_position_{0.0f};
  uint8_t motor1_speed_{0};
  uint8_t motor1_load_{0};
  uint8_t motor2_speed_{0};
  uint8_t motor2_load_{0};
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
  bool manual_stop_{false};
  bool stop_command_pending_{false};
  bool stop_ack_received_{false};
  GateMotionState motion_state_{GateMotionState::UNKNOWN};
  std::array<uint8_t, 20> params_current_{};
  std::array<uint8_t, 20> params_pending_{};
  bool params_known_{false};
  bool params_dirty_{false};
  std::string last_state_line_;
  std::string last_ack_line_;
  std::string last_rs_line_;
  uint32_t last_poll_time_{0};
  uint32_t last_motion_change_time_{0};
  uint32_t suppress_poll_until_{0};
  uint32_t last_param_sync_time_{0};
  LearnMode learn_mode_{LearnMode::NONE};
  bool learn_polling_active_{false};
  uint32_t learn_started_at_{0};
  uint32_t last_learn_poll_time_{0};

  CB19PedestrianButton *pedestrian_button_{nullptr};
  CB19ApplyParametersButton *apply_parameters_button_{nullptr};
  CB19ReloadParametersButton *reload_parameters_button_{nullptr};
  CB19RevertParametersButton *revert_parameters_button_{nullptr};
  CB19FactoryResetButton *factory_reset_button_{nullptr};
  CB19AutoLearnButton *auto_learn_button_{nullptr};
  CB19RemoteLearnButton *remote_learn_button_{nullptr};
  CB19ClearRemoteLearnButton *clear_remote_learn_button_{nullptr};
  std::array<CB19ParameterSelect *, 20> parameter_selects_{};
  CB19CalibrationNumber *opening_start_number_{nullptr};
  CB19CalibrationNumber *closing_start_number_{nullptr};
  sensor::Sensor *motor1_raw_sensor_{nullptr};
  sensor::Sensor *motor2_raw_sensor_{nullptr};
  sensor::Sensor *motor1_position_sensor_{nullptr};
  sensor::Sensor *motor2_position_sensor_{nullptr};
  sensor::Sensor *gate_position_sensor_{nullptr};
  sensor::Sensor *motor1_speed_sensor_{nullptr};
  sensor::Sensor *motor1_load_sensor_{nullptr};
  sensor::Sensor *motor2_speed_sensor_{nullptr};
  sensor::Sensor *motor2_load_sensor_{nullptr};
  text_sensor::TextSensor *gate_state_text_sensor_{nullptr};
  text_sensor::TextSensor *last_ack_text_sensor_{nullptr};
  text_sensor::TextSensor *last_rs_text_sensor_{nullptr};
  text_sensor::TextSensor *learn_status_text_sensor_{nullptr};
  text_sensor::TextSensor *param_current_text_sensor_{nullptr};
  text_sensor::TextSensor *param_pending_text_sensor_{nullptr};
  text_sensor::TextSensor *config_warning_text_sensor_{nullptr};
  binary_sensor::BinarySensor *moving_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fully_opened_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fully_closed_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *ped_opened_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *manual_stop_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *photocell_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *obstruction_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *params_dirty_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *learning_active_binary_sensor_{nullptr};
};

}  // namespace cb19_gate
}  // namespace esphome
