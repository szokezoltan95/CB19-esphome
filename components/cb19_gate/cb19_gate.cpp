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

void CB19GateComponent::setup() {
  ESP_LOGI(TAG, "CB19 gate component initialized");
}

void CB19GateComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "CB19 Gate:");
  ESP_LOGCONFIG(TAG, "  Position range: min=%u max=%u", this->min_position_, this->max_position_);
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
}

void CB19GateComponent::open_gate() { this->send_command_("FULL OPEN"); }

void CB19GateComponent::close_gate() { this->send_command_("FULL CLOSE"); }

void CB19GateComponent::stop_gate() { this->send_command_("STOP"); }

void CB19GateComponent::pedestrian_open() { this->send_command_("PED OPEN"); }

void CB19GateComponent::send_command_(const std::string &cmd) {
  std::string full = cmd + ";src=P0031DA2\r\n";

  ESP_LOGI(TAG, "TX RAW: [%s]", full.c_str());

  this->write_str(full.c_str());
  this->flush();
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

  if (line.rfind("ACK ", 0) == 0) {
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

void CB19GateComponent::apply_rs_frame_(const std::array<uint8_t, 9> &frame, const std::string &raw_line) {
  this->last_frame_ = frame;
  this->has_frame_ = true;
  this->last_rs_line_ = raw_line;

  this->photocell_active_ = (frame[0] == 0x62);
  this->motor1_raw_ = frame[3];
  this->motor2_raw_ = frame[6];
  this->motor1_percent_ = this->scale_position_(this->motor1_raw_);
  this->motor2_percent_ = this->scale_position_(this->motor2_raw_);
  this->overall_percent_ = (this->motor1_percent_ + this->motor2_percent_) / 2.0f;

  switch (frame[2]) {
    case 0xCC:
      if (this->motion_state_ != GateMotionState::PED_OPENING) {
        if (this->last_state_line_ == "Closing") {
          this->motion_state_ = GateMotionState::CLOSING;
        } else {
          this->motion_state_ = GateMotionState::OPENING;
        }
      }
      this->obstruction_active_ = false;
      break;

    case 0xAA:
      this->obstruction_active_ = false;
      if (this->overall_percent_ > 90.0f) {
        this->motion_state_ = GateMotionState::OPENED;
      } else if (this->overall_percent_ < 10.0f) {
        this->motion_state_ = GateMotionState::CLOSED;
      }
      break;

    case 0xEE:
      this->motion_state_ = GateMotionState::STOPPED;
      this->obstruction_active_ = true;
      break;

    case 0xDB:
    case 0xFB:
      if (this->last_state_line_ == "PedOpened") {
        this->motion_state_ = GateMotionState::PED_OPENED;
      } else {
        this->motion_state_ = GateMotionState::PED_OPENING;
      }
      this->obstruction_active_ = false;
      break;

    default:
      break;
  }

  if (this->last_rs_text_sensor_ != nullptr) {
    this->last_rs_text_sensor_->publish_state(raw_line);
  }

  this->publish_all_();
}

void CB19GateComponent::apply_state_line_(const std::string &line) {
  this->last_state_line_ = line;

  if (line == "Opening") {
    this->motion_state_ = GateMotionState::OPENING;
    this->obstruction_active_ = false;
  } else if (line == "Opened") {
    this->motion_state_ = GateMotionState::OPENED;
    this->obstruction_active_ = false;
  } else if (line == "Closing") {
    this->motion_state_ = GateMotionState::CLOSING;
    this->obstruction_active_ = false;
  } else if (line == "Closed") {
    this->motion_state_ = GateMotionState::CLOSED;
    this->obstruction_active_ = false;
  } else if (line == "Stopped") {
    this->motion_state_ = GateMotionState::STOPPED;
  } else if (line == "PedOpening") {
    this->motion_state_ = GateMotionState::PED_OPENING;
    this->obstruction_active_ = false;
  } else if (line == "PedOpened") {
    this->motion_state_ = GateMotionState::PED_OPENED;
    this->obstruction_active_ = false;
  }

  if (this->last_state_text_sensor_ != nullptr) {
    this->last_state_text_sensor_->publish_state(line);
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

  const bool moving = this->motion_state_ == GateMotionState::OPENING ||
                      this->motion_state_ == GateMotionState::CLOSING ||
                      this->motion_state_ == GateMotionState::PED_OPENING;
  const bool fully_open = this->motion_state_ == GateMotionState::OPENED ||
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

float CB19GateComponent::scale_position_(uint8_t raw) const {
  if (this->max_position_ <= this->min_position_) {
    return 0.0f;
  }

  float value = 100.0f * float(raw - this->min_position_) / float(this->max_position_ - this->min_position_);
  value = std::max(0.0f, std::min(100.0f, value));
  return value;
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
  }

  if (call.get_position().has_value()) {
    const float pos = *call.get_position();
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