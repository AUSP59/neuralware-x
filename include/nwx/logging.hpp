// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>

namespace nwx {

enum class LogLevel { Trace=0, Debug, Info, Warn, Error, Fatal };

class Logger {
 public:
  explicit Logger(LogLevel level = LogLevel::Info) : level_(level) {}
  void set_level(LogLevel level) { level_ = level; }
  LogLevel level() const { return level_; }

  template <typename... Args>
  void log(LogLevel lvl, std::string_view fmt, Args&&... args) {
    if (lvl < level_) return;
    std::lock_guard<std::mutex> lock(mu_);
    std::string msg = format(fmt, std::forward<Args>(args)...);
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    std::cerr << buf << " [" << level_name(lvl) << "] " << msg << '\n';
  }

  template <typename... Args> void info(std::string_view fmt, Args&&... args) { log(LogLevel::Info, fmt, std::forward<Args>(args)...); }
  template <typename... Args> void warn(std::string_view fmt, Args&&... args) { log(LogLevel::Warn, fmt, std::forward<Args>(args)...); }
  template <typename... Args> void error(std::string_view fmt, Args&&... args) { log(LogLevel::Error, fmt, std::forward<Args>(args)...); }

 private:
  std::mutex mu_;
  LogLevel level_;

  static const char* level_name(LogLevel lvl) {
    switch (lvl) {
      case LogLevel::Trace: return "TRACE";
      case LogLevel::Debug: return "DEBUG";
      case LogLevel::Info:  return "INFO";
      case LogLevel::Warn:  return "WARN";
      case LogLevel::Error: return "ERROR";
      case LogLevel::Fatal: return "FATAL";
    }
    return "UNKNOWN";
  }

  template <typename... Args>
  static std::string format(std::string_view fmt, Args&&... args) {
    int size = std::snprintf(nullptr, 0, std::string(fmt).c_str(), args...) + 1;
    if (size <= 0) return std::string(fmt);
    std::string buf;
    buf.resize(static_cast<size_t>(size));
    std::snprintf(buf.data(), static_cast<size_t>(size), std::string(fmt).c_str(), args...);
    if (!buf.empty() && buf.back() == '\0') buf.pop_back();
    return buf;
  }
};

} // namespace nwx
