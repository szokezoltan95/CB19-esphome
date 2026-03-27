#include "cb19_gate.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

#include "esphome/core/log.h"

namespace esphome {
namespace cb19_gate {

static const char *const TAG = "cb19_gate";

static std::string trim_copy(std::string s) {
  while (!s.empty() &&
         (s.back() == '\r' || s.back() == '\n' ||
          std::isspace(static_cast<unsigned char>(s.back())))) {
    s.pop_back();
  }
  size_t start = 0;
  while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
    start++;
  }
  return s.substr(start);
}

void CB19OpeningStartNumber::control(float value) {
  if (this->parent_ != nullptr) {
    this->parent_->set_opening_start_percent(value);
  }
  this->publish_state(value);
}

void CB19ClosingStartNumber::control(float value) {
  if (this->parent_ != nullptr) {
    this->parent_->set_closing_start_percent(value);
  }
  this->publish_state(value);
}

void CB19PedestrianButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pedestrian_open();
  }
}

void CB19ApplyParametersButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->apply_pending_parameters();
  }
}

void CB19ReloadParametersButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->request_param_read();
  }
}

void CB19RevertParametersButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->revert_pending_parameters();
  }
}

void CB19FactoryResetButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->start_factory_reset();
  }
}

void CB19AutoLearnButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->start_auto_learn();
  }
}

void CB19RemoteLearnButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->start_remote_learn();
  }
}

void CB19ClearRemoteLearnButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->start_clear_remote_learn();
  }
}

void CB19ParameterSelect::control(const std::string &value) {
  if (this->parent_ != nullptr) {
    this->parent_->set_pending_parameter_from_option(this->parameter_index_, value);
  }
}

void CB19GateComponent::set_opening_start_percent(float value) {
  this->opening_start_percent_ = std::max(0.0f, std::min(99.0f, value));
  this->recalculate_positions_();
  this->publish_all_();
}

void CB19GateComponent::set_closing_start_percent(float value) {
  this->closing_start_percent_ = std::max(1.0f, std::min(100.0f, value));
  this->recalculate_positions_();
  this->publish_all_();
}

void CB19GateComponent::setup() {
  ESP_LOGI(TAG, "CB19 gate component initialized");
  const uint32_t now = millis();
  this->last_poll_time_ = now;
  this->last_motion_change_time_ = now;
  this->suppress_poll_until_ = now + 1000;

  if (this->opening_start_number_ != nullptr) {
    this->opening_start_percent_ = this->opening_start_number_->get_initial_value();
    this->opening_start_number_->publish_state(this->opening_start_percent_);
  }
  if (this->closing_start_number_ != nullptr) {
    this->closing_start_percent_ = this->closing_start_number_->get_initial_value();
    this->closing_start_number_->publish_state(this->closing_start_percent_);
  }

  this->set_learn_status_("Idle", false);
}

void CB19GateComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CB19 Gate:");
  ESP_LOGCONFIG(TAG, "  Position range fallback: min=%u max=%u", this->min_position_, this->max_position_);
  ESP_LOGCONFIG(TAG, "  Opening start percent: %.1f", this->opening_start_percent_);
  ESP_LOGCONFIG(TAG, "  Closing start percent: %.1f", this->closing_start_percent_);
  ESP_LOGCONFIG(TAG, "  Parameters known: %s", this->params_known_ ? "yes" : "no");
}

void CB19GateComponent::loop() {
  while (this->available()) {
    const char c = static_cast<char>(this->read());
    if (c == '\n' || c == '\r') {
      if (!this->buffer_.empty()) {
        const std::string line = trim_copy(this->buffer_);
        this->buffer_.clear();
        if (!line.empty()) {
          this->parse_line_(line);
        }
      }
    } else {
      this->buffer_.push_back(c);
      if (this->buffer_.size() > 256) {
        ESP_LOGW(TAG, "RX buffer overflow, dropping line");
        this->buffer_.clear();
      }
    }
  }

  this->maybe_poll_rs_();
  this->maybe_sync_parameters_();
  this->maybe_poll_learn_status_();
}

void CB19GateComponent::open_gate() {
  this->clear_stop_context_();
  this->send_command_("FULL OPEN");
}
void CB19GateComponent::close_gate() {
  this->clear_stop_context_();
  this->send_command_("FULL CLOSE");
}
void CB19GateComponent::stop_gate() {
  this->set_stop_command_pending_();
  this->send_command_("STOP");
}
void CB19GateComponent::pedestrian_open() {
  this->clear_stop_context_();
  this->send_command_("PED OPEN");
}
void CB19GateComponent::request_param_read() { this->send_command_("RP,1"); }

void CB19GateComponent::apply_pending_parameters() {
  if (!this->params_known_) {
    ESP_LOGW(TAG, "Cannot apply parameters before first RP,1 sync");
    return;
  }
  this->send_command_(this->build_wp1_command_());
}

void CB19GateComponent::revert_pending_parameters() {
  if (!this->params_known_) {
    return;
  }
  this->params_pending_ = this->params_current_;
  this->params_dirty_ = false;
  this->publish_parameter_entities_();
  this->publish_all_();
}

void CB19GateComponent::start_factory_reset() { this->send_command_("RESTORE"); }

void CB19GateComponent::start_auto_learn() {
  this->learn_mode_ = LearnMode::AUTO_LEARN;
  this->learn_polling_active_ = true;
  this->learn_started_at_ = millis();
  this->last_learn_poll_time_ = 0;
  this->set_learn_status_("Starting", true);
  this->send_command_("AUTO LEARN");
}

void CB19GateComponent::start_remote_learn() {
  this->learn_mode_ = LearnMode::REMOTE_LEARN;
  this->learn_polling_active_ = false;
  this->learn_started_at_ = millis();
  this->set_learn_status_("Remote learn waiting", true);
  this->send_command_("REMOTE LEARN");
}

void CB19GateComponent::start_clear_remote_learn() {
  this->learn_mode_ = LearnMode::CLEAR_REMOTE;
  this->learn_polling_active_ = false;
  this->learn_started_at_ = millis();
  this->set_learn_status_("Clearing remotes", true);
  this->send_command_("CLEAR REMOTE LEARN");
}

void CB19GateComponent::send_command_(const std::string &cmd) {
  const std::string full = cmd + ";src=P0031DA2\r\n";
  ESP_LOGI(TAG, "TX RAW: [%s]", full.c_str());
  this->write_str(full.c_str());
  this->flush();

  const uint32_t now = millis();
  this->suppress_poll_until_ = now + 500;
  this->last_poll_time_ = now;
}

void CB19GateComponent::parse_line_(const std::string &line) {
  ESP_LOGD(TAG, "RX: %s", line.c_str());

  if (line.rfind("ACK RS:", 0) == 0) {
    std::array<uint8_t, 9> frame{};
    if (this->parse_rs_frame_(line.substr(7), frame)) {
      this->apply_rs_frame_(frame, line);
    } else {
      ESP_LOGW(TAG, "Failed to parse RS frame: %s", line.c_str());
    }
    return;
  }

  if (line.rfind("ACK RP,1:", 0) == 0) {
    std::array<uint8_t, 20> values{};
    if (this->parse_param_block_(line.substr(9), values)) {
      this->apply_param_block_(values, true);
      this->last_param_sync_time_ = millis();
    }
    this->last_ack_line_ = line;
    if (this->last_ack_text_sensor_ != nullptr) {
      this->last_ack_text_sensor_->publish_state(line);
    }
    return;
  }

  if (line.rfind("ACK LEARN STATUS:", 0) == 0) {
    this->handle_learn_status_ack_(trim_copy(line.substr(17)));
    this->last_ack_line_ = line;
    if (this->last_ack_text_sensor_ != nullptr) {
      this->last_ack_text_sensor_->publish_state(line);
    }
    return;
  }

  if (line == "ACK WP,1" || line == "ACK WP,1:") {
    this->last_ack_line_ = line;
    if (this->last_ack_text_sensor_ != nullptr) {
      this->last_ack_text_sensor_->publish_state(line);
    }
    this->request_param_read();
    return;
  }

  if (line.rfind("ACK ", 0) == 0 || line.rfind("NAK ", 0) == 0) {
    this->last_ack_line_ = line;
    if (line == "ACK STOP") {
      this->stop_ack_received_ = true;
    }
    if (this->last_ack_text_sensor_ != nullptr) {
      this->last_ack_text_sensor_->publish_state(line);
    }
    return;
  }

  this->apply_state_line_(line);
}

bool CB19GateComponent::parse_rs_frame_(const std::string &payload, std::array<uint8_t, 9> &out) {
  std::stringstream ss(trim_copy(payload));
  std::string token;
  size_t index = 0;

  while (std::getline(ss, token, ',')) {
    token = trim_copy(token);
    if (token.empty() || index >= out.size()) {
      return false;
    }
    unsigned int value = 0;
    if (std::sscanf(token.c_str(), "%x", &value) != 1 || value > 0xFF) {
      return false;
    }
    out[index++] = static_cast<uint8_t>(value);
  }

  return index == out.size();
}

bool CB19GateComponent::parse_param_block_(const std::string &payload, std::array<uint8_t, 20> &out) const {
  std::stringstream ss(trim_copy(payload));
  std::string token;
  size_t index = 0;

  while (std::getline(ss, token, ',')) {
    token = trim_copy(token);
    if (token.empty() || index >= out.size()) {
      return false;
    }
    int value = -1;
    if (std::sscanf(token.c_str(), "%d", &value) != 1 || value < 0 || value > 255) {
      return false;
    }
    out[index++] = static_cast<uint8_t>(value);
  }
  return index == out.size();
}

std::string CB19GateComponent::build_wp1_command_() const {
  std::ostringstream oss;
  oss << "WP,1:";
  for (size_t i = 0; i < this->params_pending_.size(); i++) {
    if (i != 0) oss << ',';
    oss << static_cast<int>(this->params_pending_[i]);
  }
  return oss.str();
}

std::string CB19GateComponent::format_param_block_(const std::array<uint8_t, 20> &values) const {
  std::ostringstream oss;
  for (size_t i = 0; i < values.size(); i++) {
    if (i != 0) oss << ',';
    oss << static_cast<int>(values[i]);
  }
  return oss.str();
}

int CB19GateComponent::parse_raw_value_from_option_(const std::string &option) const {
  const auto dash = option.find('-');
  if (dash == std::string::npos) return -1;
  size_t pos = dash + 1;
  std::string digits;
  while (pos < option.size() && std::isdigit(static_cast<unsigned char>(option[pos]))) {
    digits.push_back(option[pos]);
    pos++;
  }
  if (digits.empty()) return -1;
  int value = -1;
  if (std::sscanf(digits.c_str(), "%d", &value) != 1) return -1;
  return value;
}

void CB19GateComponent::apply_param_block_(const std::array<uint8_t, 20> &values, bool update_pending) {
  this->params_current_ = values;
  if (update_pending) {
    this->params_pending_ = values;
    this->params_dirty_ = false;
  }
  this->params_known_ = true;
  this->update_config_warning_();
  this->publish_parameter_entities_();
  this->publish_all_();
}

void CB19GateComponent::update_config_warning_() {
  if (this->config_warning_text_sensor_ == nullptr) return;
  if (!this->params_known_) {
    this->config_warning_text_sensor_->publish_state("Unknown");
  } else if (this->params_current_[0] > 1) {
    this->config_warning_text_sensor_->publish_state("Unsupported F1 mode detected");
  } else {
    this->config_warning_text_sensor_->publish_state("OK");
  }
}

void CB19GateComponent::publish_parameter_entities_() {
  if (this->param_current_text_sensor_ != nullptr) {
    this->param_current_text_sensor_->publish_state(this->format_param_block_(this->params_current_));
  }
  if (this->param_pending_text_sensor_ != nullptr) {
    this->param_pending_text_sensor_->publish_state(this->format_param_block_(this->params_pending_));
  }
  if (this->params_dirty_binary_sensor_ != nullptr) {
    this->params_dirty_binary_sensor_->publish_state(this->params_dirty_);
  }

  for (size_t i = 0; i < this->parameter_selects_.size(); i++) {
    auto *sel = this->parameter_selects_[i];
    if (sel == nullptr) continue;
    const auto raw = this->params_pending_[i];
    for (const auto &option : sel->traits.get_options()) {
      if (this->parse_raw_value_from_option_(option) == static_cast<int>(raw)) {
        sel->publish_state(option);
        break;
      }
    }
  }
}

void CB19GateComponent::set_pending_parameter_from_option(uint8_t index, const std::string &option) {
  if (index >= this->params_pending_.size()) return;
  const int raw = this->parse_raw_value_from_option_(option);
  if (raw < 0 || raw > 255) return;
  this->params_pending_[index] = static_cast<uint8_t>(raw);
  this->params_dirty_ = this->params_known_ && (this->params_pending_ != this->params_current_);
  this->publish_parameter_entities_();
  this->publish_all_();
}

float CB19GateComponent::scale_position_fallback_(uint8_t raw) const {
  if (this->max_position_ <= this->min_position_) return 0.0f;
  const float value = 100.0f * float(raw - this->min_position_) / float(this->max_position_ - this->min_position_);
  return std::max(0.0f, std::min(100.0f, value));
}

float CB19GateComponent::scale_motor_position_(uint8_t raw, bool closed_valid, uint8_t closed_ref,
                                               bool open_valid, uint8_t open_ref) const {
  if (!(closed_valid && open_valid) || open_ref <= closed_ref) {
    return this->scale_position_fallback_(raw);
  }
  const float value = 100.0f * float(raw - closed_ref) / float(open_ref - closed_ref);
  return std::max(0.0f, std::min(100.0f, value));
}

float CB19GateComponent::apply_cover_calibration_(float base_percent) const {
  float value = base_percent;
  if (this->motion_state_ == GateMotionState::OPENING || this->motion_state_ == GateMotionState::PED_OPENING) {
    if (this->opening_start_percent_ < 99.0f) {
      value = (base_percent - this->opening_start_percent_) * 100.0f / (100.0f - this->opening_start_percent_);
    }
  } else if (this->motion_state_ == GateMotionState::CLOSING) {
    if (this->closing_start_percent_ > 0.0f) {
      value = base_percent * 100.0f / this->closing_start_percent_;
    }
  }
  return std::max(0.0f, std::min(100.0f, value));
}

void CB19GateComponent::recalculate_positions_() {
  this->motor1_position_ = this->scale_motor_position_(this->motor1_raw_, this->motor1_closed_ref_valid_, this->motor1_closed_ref_, this->motor1_open_ref_valid_, this->motor1_open_ref_);
  this->motor2_position_ = this->scale_motor_position_(this->motor2_raw_, this->motor2_closed_ref_valid_, this->motor2_closed_ref_, this->motor2_open_ref_valid_, this->motor2_open_ref_);
  if (this->motion_state_ == GateMotionState::PED_OPENING || this->motion_state_ == GateMotionState::PED_OPENED) {
    this->gate_position_raw_ = this->motor1_position_;
  } else {
    this->gate_position_raw_ = (this->motor1_position_ + this->motor2_position_) / 2.0f;
  }
  this->gate_position_ = this->apply_cover_calibration_(this->gate_position_raw_);
}

void CB19GateComponent::apply_rs_frame_(const std::array<uint8_t, 9> &frame, const std::string &raw_line) {
  this->last_frame_ = frame;
  this->has_frame_ = true;
  this->last_rs_line_ = raw_line;
  this->photocell_active_ = (frame[0] == 0x62);
  this->motor1_raw_ = frame[3];
  this->motor1_speed_ = frame[4];
  this->motor1_load_ = frame[5];
  this->motor2_raw_ = frame[6];
  this->motor2_speed_ = frame[7];
  this->motor2_load_ = frame[8];
  this->recalculate_positions_();

  if (frame[2] == 0x62 || frame[2] == 0xEE) {
    // EE can mean manual stop or obstruction. Final classification is decided when V1PKF Stopped arrives.
  }

  this->update_status_flags_();

  if (this->last_rs_text_sensor_ != nullptr) {
    this->last_rs_text_sensor_->publish_state(raw_line);
  }
  this->publish_all_();
}

std::string CB19GateComponent::extract_protocol_state_(const std::string &line) {
  if (line.rfind("$V1PKF", 0) != 0) return trim_copy(line);
  const size_t first_comma = line.find(',');
  if (first_comma == std::string::npos) return trim_copy(line);
  const size_t second_comma = line.find(',', first_comma + 1);
  if (second_comma == std::string::npos) return trim_copy(line);
  std::string payload = line.substr(second_comma + 1);
  const size_t src_pos = payload.find(";src=");
  if (src_pos != std::string::npos) payload = payload.substr(0, src_pos);
  return trim_copy(payload);
}

void CB19GateComponent::learn_current_refs_from_state_(const std::string &state) {
  if (state == "Closed") {
    this->motor1_closed_ref_ = this->motor1_raw_;
    this->motor2_closed_ref_ = this->motor2_raw_;
    this->motor1_closed_ref_valid_ = true;
    this->motor2_closed_ref_valid_ = true;
  } else if (state == "Opened") {
    this->motor1_open_ref_ = this->motor1_raw_;
    this->motor2_open_ref_ = this->motor2_raw_;
    this->motor1_open_ref_valid_ = true;
    this->motor2_open_ref_valid_ = true;
  } else if (state == "PedOpened") {
    this->motor1_open_ref_ = this->motor1_raw_;
    this->motor1_open_ref_valid_ = true;
  }
}

void CB19GateComponent::apply_state_line_(const std::string &line) {
  const std::string state = this->extract_protocol_state_(line);
  this->last_state_line_ = state;

  if (state == "Restored") {
    this->request_param_read();
  } else if (state == "AutoLearn") {
    this->set_learn_status_("System learning", true);
  } else if (state == "LearnStart") {
    this->set_learn_status_("Remote learn waiting", true);
  } else if (state == "RemoteAdd") {
    this->set_learn_status_("Remote added", true);
  } else if (state == "LearnComplete") {
    this->learn_mode_ = LearnMode::NONE;
    this->learn_polling_active_ = false;
    this->set_learn_status_("Success", false);
  } else if (state == "ClearComplete") {
    this->learn_mode_ = LearnMode::NONE;
    this->learn_polling_active_ = false;
    this->set_learn_status_("Remote cleared", false);
  }

  this->learn_current_refs_from_state_(state);
  this->recalculate_positions_();

  if (state == "Opening") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::OPENING);
  } else if (state == "Opened") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::OPENED);
    this->gate_position_raw_ = 100.0f;
    this->gate_position_ = 100.0f;
  } else if (state == "Closing") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::CLOSING);
  } else if (state == "AutoClosing") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::AUTO_CLOSING);
  } else if (state == "Closed") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::CLOSED);
    this->gate_position_raw_ = 0.0f;
    this->gate_position_ = 0.0f;
  } else if (state == "Stopped") {
    this->set_motion_state_(GateMotionState::STOPPED);
    const bool manual_stop = this->stop_command_pending_ && this->stop_ack_received_;
    this->manual_stop_ = manual_stop;
    this->obstruction_active_ = !manual_stop;
  } else if (state == "PedOpening") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::PED_OPENING);
    this->gate_position_raw_ = this->motor1_position_;
  } else if (state == "PedOpened") {
    this->clear_stop_context_();
    this->obstruction_active_ = false;
    this->manual_stop_ = false;
    this->set_motion_state_(GateMotionState::PED_OPENED);
    this->gate_position_raw_ = 100.0f;
    this->gate_position_ = 100.0f;
  }

  this->recalculate_positions_();
  this->update_status_flags_();

  if (this->gate_state_text_sensor_ != nullptr) {
    this->gate_state_text_sensor_->publish_state(state);
  }
  this->publish_all_();
}

void CB19GateComponent::set_learn_status_(const std::string &status, bool active) {
  if (this->learn_status_text_sensor_ != nullptr) this->learn_status_text_sensor_->publish_state(status);
  if (this->learning_active_binary_sensor_ != nullptr) this->learning_active_binary_sensor_->publish_state(active);
}

void CB19GateComponent::handle_learn_status_ack_(const std::string &payload) {
  if (payload == "SYSTEM LEARNING") {
    this->set_learn_status_("System learning", true);
  } else if (payload == "SYSTEM LEARN COMPLETE") {
    this->learn_mode_ = LearnMode::NONE;
    this->learn_polling_active_ = false;
    this->set_learn_status_("Success", false);
    this->request_param_read();
  } else if (payload == "SYSTEM LEARN FAIL") {
    this->learn_mode_ = LearnMode::NONE;
    this->learn_polling_active_ = false;
    this->set_learn_status_("Failed", false);
  } else {
    this->set_learn_status_(payload, this->learn_polling_active_);
  }
}

void CB19GateComponent::publish_all_() {
  if (this->motor1_raw_sensor_ != nullptr) this->motor1_raw_sensor_->publish_state(this->motor1_raw_);
  if (this->motor2_raw_sensor_ != nullptr) this->motor2_raw_sensor_->publish_state(this->motor2_raw_);
  if (this->motor1_position_sensor_ != nullptr) this->motor1_position_sensor_->publish_state(this->motor1_position_);
  if (this->motor2_position_sensor_ != nullptr) this->motor2_position_sensor_->publish_state(this->motor2_position_);
  if (this->gate_position_sensor_ != nullptr) this->gate_position_sensor_->publish_state(this->gate_position_);
  if (this->motor1_speed_sensor_ != nullptr) this->motor1_speed_sensor_->publish_state(this->motor1_speed_);
  if (this->motor1_load_sensor_ != nullptr) this->motor1_load_sensor_->publish_state(this->motor1_load_);
  if (this->motor2_speed_sensor_ != nullptr) this->motor2_speed_sensor_->publish_state(this->motor2_speed_);
  if (this->motor2_load_sensor_ != nullptr) this->motor2_load_sensor_->publish_state(this->motor2_load_);

  const bool moving = this->is_moving_state_(this->motion_state_);
  const bool fully_opened = this->motion_state_ == GateMotionState::OPENED;
  const bool fully_closed = this->motion_state_ == GateMotionState::CLOSED;
  const bool ped_opened = this->motion_state_ == GateMotionState::PED_OPENED;

  if (this->moving_binary_sensor_ != nullptr) this->moving_binary_sensor_->publish_state(moving);
  if (this->fully_opened_binary_sensor_ != nullptr) this->fully_opened_binary_sensor_->publish_state(fully_opened);
  if (this->fully_closed_binary_sensor_ != nullptr) this->fully_closed_binary_sensor_->publish_state(fully_closed);
  if (this->ped_opened_binary_sensor_ != nullptr) this->ped_opened_binary_sensor_->publish_state(ped_opened);
  if (this->manual_stop_binary_sensor_ != nullptr) this->manual_stop_binary_sensor_->publish_state(this->manual_stop_);
  if (this->photocell_binary_sensor_ != nullptr) this->photocell_binary_sensor_->publish_state(this->photocell_active_);
  if (this->obstruction_binary_sensor_ != nullptr) this->obstruction_binary_sensor_->publish_state(this->obstruction_active_);
  if (this->params_dirty_binary_sensor_ != nullptr) this->params_dirty_binary_sensor_->publish_state(this->params_dirty_);

}

std::string CB19GateComponent::motion_state_to_string_(GateMotionState state) const {
  switch (state) {
    case GateMotionState::OPENING: return "Opening";
    case GateMotionState::OPENED: return "Opened";
    case GateMotionState::CLOSING: return "Closing";
    case GateMotionState::CLOSED: return "Closed";
    case GateMotionState::STOPPED: return "Stopped";
    case GateMotionState::PED_OPENING: return "PedOpening";
    case GateMotionState::PED_OPENED: return "PedOpened";
    case GateMotionState::AUTO_CLOSING: return "AutoClosing";
    default: return "Unknown";
  }
}

bool CB19GateComponent::is_moving_state_(GateMotionState state) const {
  return state == GateMotionState::OPENING || state == GateMotionState::CLOSING || state == GateMotionState::PED_OPENING || state == GateMotionState::AUTO_CLOSING;
}

void CB19GateComponent::set_motion_state_(GateMotionState state) {
  if (this->motion_state_ != state) {
    this->motion_state_ = state;
    this->last_motion_change_time_ = millis();
  }
}


void CB19GateComponent::set_stop_command_pending_() {
  this->stop_command_pending_ = true;
  this->stop_ack_received_ = false;
  this->manual_stop_ = false;
}

void CB19GateComponent::clear_stop_context_() {
  this->stop_command_pending_ = false;
  this->stop_ack_received_ = false;
  this->manual_stop_ = false;
}

void CB19GateComponent::update_status_flags_() {
  this->gate_position_ = this->apply_cover_calibration_(this->gate_position_raw_);
}

uint32_t CB19GateComponent::get_poll_interval_ms_() const {
  if (this->is_moving_state_(this->motion_state_)) return 200;
  if (this->motion_state_ == GateMotionState::UNKNOWN) return 1000;
  const uint32_t now = millis();
  const uint32_t since_change = now - this->last_motion_change_time_;
  if (since_change < 10000) return 500;
  return 60000;
}

void CB19GateComponent::maybe_poll_rs_() {
  const uint32_t now = millis();
  if (now < this->suppress_poll_until_) return;
  const uint32_t interval = this->get_poll_interval_ms_();
  if (now - this->last_poll_time_ >= interval) {
    this->last_poll_time_ = now;
    this->send_command_("RS");
  }
}

void CB19GateComponent::maybe_sync_parameters_() {
  const uint32_t now = millis();
  if (this->learn_polling_active_ || this->is_moving_state_(this->motion_state_) || now < this->suppress_poll_until_) return;
  if (!this->params_known_) {
    if (now > 2500) this->request_param_read();
    return;
  }
  if (now - this->last_param_sync_time_ >= 300000UL) {
    this->request_param_read();
  }
}

void CB19GateComponent::maybe_poll_learn_status_() {
  if (!this->learn_polling_active_ || this->learn_mode_ != LearnMode::AUTO_LEARN) return;
  const uint32_t now = millis();
  if (now - this->learn_started_at_ > 300000UL) {
    this->learn_polling_active_ = false;
    this->learn_mode_ = LearnMode::NONE;
    this->set_learn_status_("Timeout", false);
    return;
  }
  if (now - this->last_learn_poll_time_ >= 1000UL) {
    this->last_learn_poll_time_ = now;
    this->send_command_("READ LEARN STATUS");
  }
}

}  // namespace cb19_gate
}  // namespace esphome
