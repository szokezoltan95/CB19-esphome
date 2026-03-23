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
}

void CB19GateComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CB19 Gate:");
  ESP_LOGCONFIG(TAG, "  Position range fallback: min=%u max=%u", this->min_position_, this->max_position_);
  ESP_LOGCONFIG(TAG, "  Opening start percent: %.1f", this->opening_start_percent_);
  ESP_LOGCONFIG(TAG, "  Closing start percent: %.1f", this->closing_start_percent_);
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
}

void CB19GateComponent::open_gate() {
  this->send_command_("FULL OPEN");
}

void CB19GateComponent::close_gate() {
  this->send_command_("FULL CLOSE");
}

void CB19GateComponent::stop_gate() {
  this->send_command_("STOP");
}

void CB19GateComponent::pedestrian_open() {
  this->send_command_("PED OPEN");
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

  if (line.rfind("ACK ", 0) == 0 || line.rfind("NAK ", 0) == 0) {
    this->last_ack_line_ = line;
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
    if (token.empty()) {
      return false;
    }
    if (index >= out.size()) {
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

float CB19GateComponent::scale_position_fallback_(uint8_t raw) const {
  if (this->max_position_ <= this->min_position_) {
    return 0.0f;
  }

  const float value = 100.0f * float(raw - this->min_position_) /
                      float(this->max_position_ - this->min_position_);
  return std::max(0.0f, std::min(100.0f, value));
}

float CB19GateComponent::scale_motor_position_(uint8_t raw,
                                               bool closed_valid, uint8_t closed_ref,
                                               bool open_valid, uint8_t open_ref) const {
  if (!(closed_valid && open_valid)) {
    return this->scale_position_fallback_(raw);
  }

  if (open_ref <= closed_ref) {
    return this->scale_position_fallback_(raw);
  }

  const float value = 100.0f * float(raw - closed_ref) / float(open_ref - closed_ref);
  return std::max(0.0f, std::min(100.0f, value));
}

float CB19GateComponent::apply_cover_calibration_(float base_percent) const {
  float value = base_percent;

  if (this->motion_state_ == GateMotionState::OPENING || this->motion_state_ == GateMotionState::PED_OPENING) {
    if (this->opening_start_percent_ < 99.0f) {
      value = (base_percent - this->opening_start_percent_) * 100.0f /
              (100.0f - this->opening_start_percent_);
    }
  } else if (this->motion_state_ == GateMotionState::CLOSING) {
    if (this->closing_start_percent_ > 0.0f) {
      value = base_percent * 100.0f / this->closing_start_percent_;
    }
  }

  return std::max(0.0f, std::min(100.0f, value));
}

void CB19GateComponent::recalculate_positions_() {
  this->motor1_percent_ = this->scale_motor_position_(
      this->motor1_raw_,
      this->motor1_closed_ref_valid_, this->motor1_closed_ref_,
      this->motor1_open_ref_valid_, this->motor1_open_ref_);

  this->motor2_percent_ = this->scale_motor_position_(
      this->motor2_raw_,
      this->motor2_closed_ref_valid_, this->motor2_closed_ref_,
      this->motor2_open_ref_valid_, this->motor2_open_ref_);

  if (this->motion_state_ == GateMotionState::PED_OPENING || this->motion_state_ == GateMotionState::PED_OPENED) {
    this->overall_percent_raw_ = this->motor1_percent_;
  } else {
    this->overall_percent_raw_ = (this->motor1_percent_ + this->motor2_percent_) / 2.0f;
  }

  this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
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

  switch (frame[2]) {
    case 0xCC:
      this->obstruction_active_ = false;
      if (this->last_state_line_ == "Closing") {
        this->set_motion_state_(GateMotionState::CLOSING);
      } else if (this->last_state_line_ == "PedOpening") {
        this->set_motion_state_(GateMotionState::PED_OPENING);
      } else {
        this->set_motion_state_(GateMotionState::OPENING);
      }
      this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
      break;

    case 0xAA:
      this->obstruction_active_ = false;
      if (this->overall_percent_raw_ > 90.0f) {
        this->set_motion_state_(GateMotionState::OPENED);
      } else if (this->overall_percent_raw_ < 10.0f) {
        this->set_motion_state_(GateMotionState::CLOSED);
      }
      this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
      break;

    case 0xEE:
      this->obstruction_active_ = true;
      this->set_motion_state_(GateMotionState::STOPPED);
      this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
      break;

    case 0xDB:
    case 0xFB:
      this->obstruction_active_ = false;
      if (this->last_state_line_ == "PedOpened") {
        this->set_motion_state_(GateMotionState::PED_OPENED);
      } else {
        this->set_motion_state_(GateMotionState::PED_OPENING);
      }
      this->overall_percent_raw_ = this->motor1_percent_;
      this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
      break;

    default:
      break;
  }

  if (this->last_rs_text_sensor_ != nullptr) {
    this->last_rs_text_sensor_->publish_state(raw_line);
  }

  this->publish_all_();
}

std::string CB19GateComponent::extract_protocol_state_(const std::string &line) {
  if (line.rfind("$V1PKF", 0) != 0) {
    return trim_copy(line);
  }

  const size_t first_comma = line.find(',');
  if (first_comma == std::string::npos) {
    return trim_copy(line);
  }

  const size_t second_comma = line.find(',', first_comma + 1);
  if (second_comma == std::string::npos) {
    return trim_copy(line);
  }

  std::string payload = line.substr(second_comma + 1);
  const size_t src_pos = payload.find(";src=");
  if (src_pos != std::string::npos) {
    payload = payload.substr(0, src_pos);
  }

  return trim_copy(payload);
}

void CB19GateComponent::learn_current_refs_from_state_(const std::string &state) {
  if (state == "Closed") {
    this->motor1_closed_ref_ = this->motor1_raw_;
    this->motor2_closed_ref_ = this->motor2_raw_;
    this->motor1_closed_ref_valid_ = true;
    this->motor2_closed_ref_valid_ = true;
    ESP_LOGI(TAG, "Learned CLOSED refs: motor1=%u motor2=%u", this->motor1_closed_ref_, this->motor2_closed_ref_);
  } else if (state == "Opened") {
    this->motor1_open_ref_ = this->motor1_raw_;
    this->motor2_open_ref_ = this->motor2_raw_;
    this->motor1_open_ref_valid_ = true;
    this->motor2_open_ref_valid_ = true;
    ESP_LOGI(TAG, "Learned OPEN refs: motor1=%u motor2=%u", this->motor1_open_ref_, this->motor2_open_ref_);
  } else if (state == "PedOpened") {
    this->motor1_open_ref_ = this->motor1_raw_;
    this->motor1_open_ref_valid_ = true;
    ESP_LOGI(TAG, "Learned PED OPEN ref: motor1=%u", this->motor1_open_ref_);
  }
}

void CB19GateComponent::apply_state_line_(const std::string &line) {
  const std::string state = this->extract_protocol_state_(line);
  this->last_state_line_ = state;

  this->learn_current_refs_from_state_(state);
  this->recalculate_positions_();

  if (state == "Opening") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::OPENING);
    this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
  } else if (state == "Opened") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::OPENED);
    this->overall_percent_raw_ = 100.0f;
    this->overall_percent_ = 100.0f;
  } else if (state == "Closing") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::CLOSING);
    this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
  } else if (state == "Closed") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::CLOSED);
    this->overall_percent_raw_ = 0.0f;
    this->overall_percent_ = 0.0f;
  } else if (state == "Stopped") {
    this->set_motion_state_(GateMotionState::STOPPED);
    this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
  } else if (state == "PedOpening") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::PED_OPENING);
    this->overall_percent_raw_ = this->motor1_percent_;
    this->overall_percent_ = this->apply_cover_calibration_(this->overall_percent_raw_);
  } else if (state == "PedOpened") {
    this->obstruction_active_ = false;
    this->set_motion_state_(GateMotionState::PED_OPENED);
    this->overall_percent_raw_ = 100.0f;
    this->overall_percent_ = 100.0f;
  } else {
    ESP_LOGD(TAG, "Unhandled state line payload: %s", state.c_str());
  }

  if (this->last_state_text_sensor_ != nullptr) {
    this->last_state_text_sensor_->publish_state(state);
  }

  this->publish_all_();
}

void CB19GateComponent::publish_all_() {
  if (this->motor1_raw_sensor_ != nullptr) {
    this->motor1_raw_sensor_->publish_state(this->motor1_raw_);
  }
  if (this->motor2_raw_sensor_ != nullptr) {
    this->motor2_raw_sensor_->publish_state(this->motor2_raw_);
  }
  if (this->motor1_percent_sensor_ != nullptr) {
    this->motor1_percent_sensor_->publish_state(this->motor1_percent_);
  }
  if (this->motor2_percent_sensor_ != nullptr) {
    this->motor2_percent_sensor_->publish_state(this->motor2_percent_);
  }
  if (this->overall_percent_sensor_ != nullptr) {
    this->overall_percent_sensor_->publish_state(this->overall_percent_);
  }

  if (this->motor1_speed_sensor_ != nullptr) {
    this->motor1_speed_sensor_->publish_state(this->motor1_speed_);
  }
  if (this->motor1_load_sensor_ != nullptr) {
    this->motor1_load_sensor_->publish_state(this->motor1_load_);
  }
  if (this->motor2_speed_sensor_ != nullptr) {
    this->motor2_speed_sensor_->publish_state(this->motor2_speed_);
  }
  if (this->motor2_load_sensor_ != nullptr) {
    this->motor2_load_sensor_->publish_state(this->motor2_load_);
  }

  const bool moving = this->is_moving_state_(this->motion_state_);
  const bool fully_open =
      this->motion_state_ == GateMotionState::OPENED ||
      this->motion_state_ == GateMotionState::PED_OPENED;
  const bool fully_closed = this->motion_state_ == GateMotionState::CLOSED;

  if (this->moving_binary_sensor_ != nullptr) {
    this->moving_binary_sensor_->publish_state(moving);
  }
  if (this->fully_open_binary_sensor_ != nullptr) {
    this->fully_open_binary_sensor_->publish_state(fully_open);
  }
  if (this->fully_closed_binary_sensor_ != nullptr) {
    this->fully_closed_binary_sensor_->publish_state(fully_closed);
  }
  if (this->photocell_binary_sensor_ != nullptr) {
    this->photocell_binary_sensor_->publish_state(this->photocell_active_);
  }
  if (this->obstruction_binary_sensor_ != nullptr) {
    this->obstruction_binary_sensor_->publish_state(this->obstruction_active_);
  }

  if (this->cover_ != nullptr) {
    this->cover_->sync_from_parent();
  }
}

std::string CB19GateComponent::motion_state_to_string_(GateMotionState state) const {
  switch (state) {
    case GateMotionState::OPENING:
      return "Opening";
    case GateMotionState::OPENED:
      return "Opened";
    case GateMotionState::CLOSING:
      return "Closing";
    case GateMotionState::CLOSED:
      return "Closed";
    case GateMotionState::STOPPED:
      return "Stopped";
    case GateMotionState::PED_OPENING:
      return "PedOpening";
    case GateMotionState::PED_OPENED:
      return "PedOpened";
    case GateMotionState::UNKNOWN:
    default:
      return "Unknown";
  }
}

bool CB19GateComponent::is_moving_state_(GateMotionState state) const {
  return state == GateMotionState::OPENING ||
         state == GateMotionState::CLOSING ||
         state == GateMotionState::PED_OPENING;
}

void CB19GateComponent::set_motion_state_(GateMotionState state) {
  if (this->motion_state_ != state) {
    this->motion_state_ = state;
    this->last_motion_change_time_ = millis();
    ESP_LOGD(TAG, "Motion state -> %s", this->motion_state_to_string_(state).c_str());
  }
}

uint32_t CB19GateComponent::get_poll_interval_ms_() const {
  if (this->is_moving_state_(this->motion_state_)) {
    return 200;
  }

  if (this->motion_state_ == GateMotionState::UNKNOWN) {
    return 1000;
  }

  const uint32_t now = millis();
  const uint32_t since_change = now - this->last_motion_change_time_;

  if (since_change < 10000) {
    return 500;
  }

  return 60000;
}

void CB19GateComponent::maybe_poll_rs_() {
  const uint32_t now = millis();

  if (now < this->suppress_poll_until_) {
    return;
  }

  const uint32_t interval = this->get_poll_interval_ms_();

  if (now - this->last_poll_time_ >= interval) {
    this->last_poll_time_ = now;
    this->send_command_("RS");
  }
}

cover::CoverTraits CB19GateCover::get_traits() {
  auto traits = cover::CoverTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  traits.set_supports_stop(true);
  return traits;
}

void CB19GateCover::control(const cover::CoverCall &call) {
  if (this->parent_ == nullptr) {
    return;
  }

  if (call.get_stop()) {
    this->parent_->stop_gate();
    return;
  }

  if (call.get_position().has_value()) {
    const float pos = *call.get_position();

    if (pos >= 0.99f) {
      this->parent_->open_gate();
      return;
    }

    if (pos <= 0.01f) {
      this->parent_->close_gate();
      return;
    }

    if (pos > this->position) {
      this->parent_->open_gate();
    } else if (pos < this->position) {
      this->parent_->close_gate();
    }
  }
}

void CB19GateCover::sync_from_parent() {
  if (this->parent_ == nullptr) {
    return;
  }

  this->position = this->parent_->get_overall_position_percent() / 100.0f;

  switch (this->parent_->get_motion_state()) {
    case GateMotionState::OPENING:
    case GateMotionState::PED_OPENING:
      this->current_operation = cover::COVER_OPERATION_OPENING;
      break;
    case GateMotionState::CLOSING:
      this->current_operation = cover::COVER_OPERATION_CLOSING;
      break;
    default:
      this->current_operation = cover::COVER_OPERATION_IDLE;
      break;
  }

  this->publish_state();
}

void CB19PedestrianButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->pedestrian_open();
  }
}

}  // namespace cb19_gate
}  // namespace esphome