// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "fmt/core.h"

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  // Disable min/max macros in windows.h to avoid conflicts with std::min/max
  // cxxopts.hpp uses std::min/max
  #endif

  // Undefine Raylib functions to avoid conflicts
  #define Rectangle WindowsRectangle
  #define CloseWindow WindowsCloseWindow
  #define ShowCursor WindowsShowCursor
  #define DrawText WindowsDrawText
  #define PlaySound WindowsPlaySound
  #define PlaySoundA WindowsPlaySoundA
  #define PlaySoundW WindowsPlaySoundW
  #define LoadImage WindowsLoadImage
  #define DrawTextEx WindowsDrawTextEx

  #include <windows.h>

  // Restore Raylib functions
  #undef Rectangle
  #undef CloseWindow
  #undef ShowCursor
  #undef DrawText
  #undef PlaySound
  #undef PlaySoundA
  #undef PlaySoundW
  #undef LoadImage
  #undef DrawTextEx

#endif

// Function name macros for different compilers
#if defined(__GNUC__) || defined(__clang__)
  #define FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
  #define FUNCTION_NAME __FUNCSIG__
#else
  #define FUNCTION_NAME __func__
#endif

class Logger {

private:
  std::mutex logMutex_;

private:
  std::string caller_;
  std::ostringstream messageStream_;
  std::ofstream logFile_;

protected:
  Logger () = default;
  ~Logger () {
    std::lock_guard<std::mutex> lock (logMutex_);
    // if (logFile_.is_open ()) {
    //   logFile_.close ();
    // }
  }

public:
  Logger (const Logger&) = delete;            // Logger a; Logger b(a);
  Logger (Logger&&) = delete;                 // Logger a; Logger b(std::move(a));
  Logger& operator= (const Logger&) = delete; // Logger a, b; a = b;
  Logger& operator= (Logger&&) = delete;      // Logger a, b; a = std::move(b);
  static Logger& getInstance ()               // Singleton pattern
  {
    static Logger instance;
    return instance;
  }

public:
  enum class Level { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRITICAL };

private:
  Level currentLevel_ = Level::LOG_INFO;

public:
  Logger& operator<< (Level level) // Logger a; a << Logger::Level::LOG_DEBUG;
  {
    currentLevel_ = level;
    return *this;
  }

  // Logger a; a << std::endl;
  Logger& operator<< (std::ostream& (*outputManipulator) (std::ostream&)) {
    if (outputManipulator == static_cast<std::ostream& (*)(std::ostream&)> (std::endl)) {
      log (currentLevel_, messageStream_.str (), caller_);
      // Reset state
      messageStream_.str ("");
      messageStream_.clear ();
      caller_ = "";
    } else {
      messageStream_ << outputManipulator;
    }
    return *this;
  }

  // Logger a; a << "message";
  template <typename T> Logger& operator<< (const T& message) {
    std::lock_guard<std::mutex> lock (logMutex_);
    messageStream_ << message;
    return *this;
  }

  // Logger a; a.setCaller(FUNCTION_NAME);
  Logger& setCaller (const std::string& caller) {
    std::lock_guard<std::mutex> lock (logMutex_);
    try {
      // Vytvoření explicitní kopie vstupního parametru
      std::string callerCopy = caller.empty () ? "Unknown" : caller;
      caller_ = callerCopy;
    } catch (const std::exception& e) {
      std::cerr << "Error setting caller: " << e.what () << std::endl;
      caller_ = "Error in caller"; // Nastavit bezpečnou hodnotu
    } catch (...) {
      std::cerr << "Unknown error setting caller" << std::endl;
      caller_ = "Unknown error in caller"; // Nastavit bezpečnou hodnotu
    }
    return *this;
  }

public:
  void debug (const std::string& message, const std::string& caller = "") {
    log (Level::LOG_DEBUG, message, caller);
  }

  void info (const std::string& message, const std::string& caller = "") {
    log (Level::LOG_INFO, message, caller);
  }

  void warning (const std::string& message, const std::string& caller = "") {
    log (Level::LOG_WARNING, message, caller);
  }

  void error (const std::string& message, const std::string& caller = "") {
    log (Level::LOG_ERROR, message, caller);
  }

  void critical (const std::string& message, const std::string& caller = "") {
    log (Level::LOG_CRITICAL, message, caller);
  }

  void log (Level level, const std::string& message, const std::string& caller = "") {
    std::lock_guard<std::mutex> lock (logMutex_);

    auto now = std::chrono::system_clock::now ();
    auto now_time = std::chrono::system_clock::to_time_t (now);

    // Use thread-safe localtime
    std::tm now_tm;
#ifdef _WIN32
    localtime_s (&now_tm, &now_time);
#else
    localtime_r (&now_time, &now_tm);
#endif

    // Output to appropriate stream
    if (level == Level::LOG_ERROR || level == Level::LOG_CRITICAL) {
      logToStream (std::cerr, level, message, caller, now_tm);
    } else if (level == Level::LOG_DEBUG || level == Level::LOG_INFO
               || level == Level::LOG_WARNING) {
      logToStream (std::cout, level, message, caller, now_tm);
    }

    // file output
    if (logFile_.is_open ()) {
      logFile_ << "[" << std::put_time (&now_tm, "%d-%m-%Y %H:%M:%S") << "] ";
      if (!caller.empty ()) {
        logFile_ << "[" << caller << "] ";
      } else {
        logFile_ << "[" << "empty caller" << "] ";
      }
      logFile_ << "[" << levelToString (level) << "] " << message << std::endl;
    }
  }

public:
  template <typename... Args>
  void logFmtMessage (Level level, const std::string& format, Args&&... args) {
    std::string message = fmt::format (format, std::forward<Args> (args)...);
    log (level, message, caller_);
  }

public:
  bool enableFileLogging (const std::string& filename) {
    std::lock_guard<std::mutex> lock (logMutex_);
    try {
      logFile_.open (filename, std::ios::out | std::ios::app);
      return logFile_.is_open ();
    } catch (const std::ios_base::failure& e) {
      std::cerr << "Failed to open log file: " << filename << " - " << e.what () << std::endl;
      return false;
    } catch (const std::exception& e) {
      std::cerr << "Failed to open log file: " << filename << " - " << e.what () << std::endl;
      return false;
    } catch (...) {
      std::cerr << "Failed to open log file: " << filename << std::endl;
      return false;
    }
  }

  void disableFileLogging () {
    std::lock_guard<std::mutex> lock (logMutex_);
    if (logFile_.is_open ()) {
      logFile_.close ();
    }
  }

  std::string levelToString (Level level) const {
    switch (level) {
    case Level::LOG_DEBUG:
      return "DBG";
    case Level::LOG_INFO:
      return "INF";
    case Level::LOG_WARNING:
      return "WRN";
    case Level::LOG_ERROR:
      return "ERR";
    case Level::LOG_CRITICAL:
      return "CRI";
    default:
      return "INF";
    }
  }

  void resetConsoleColor () {
#ifdef _WIN32
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
                             FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    std::cout << "\033[0m"; // Reset
#endif
  }

#ifdef _WIN32
  void setConsoleColorWindows (Level level) {
    const std::map<Level, WORD> colorMap
        = { { Level::LOG_DEBUG, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY },
            { Level::LOG_INFO, FOREGROUND_GREEN | FOREGROUND_INTENSITY },
            { Level::LOG_WARNING, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY },
            { Level::LOG_ERROR, FOREGROUND_RED | FOREGROUND_INTENSITY },
            { Level::LOG_CRITICAL, FOREGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_BLUE } };

    auto it = colorMap.find (level);
    if (it != colorMap.end ()) {
      SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), it->second);
    } else {
      resetConsoleColor ();
    }
  }
#else
  void setConsoleColorUnix (Level level) {
    static const std::map<Level, const char*> colorMap = { { Level::LOG_DEBUG, "\033[34m" },
                                                           { Level::LOG_INFO, "\033[32m" },
                                                           { Level::LOG_WARNING, "\033[33m" },
                                                           { Level::LOG_ERROR, "\033[31m" },
                                                           { Level::LOG_CRITICAL, "\033[95m" } };

    auto it = colorMap.find (level);
    if (it != colorMap.end ()) {
      std::cout << it->second;
    } else {
      resetConsoleColor ();
    }
  }
#endif
  void setConsoleColor (Level level) {
#ifdef _WIN32
    setConsoleColorWindows (level);
#else
    setConsoleColorUnix (level);
#endif
  }

private:
  std::string headerName_ = "DotNameLib";
  bool includeName_ = true;
  bool includeTime_ = true;
  bool includeCaller_ = true;
  bool includeLevel_ = true;

public:
  void setHeaderName (const std::string& headerName) {
    std::lock_guard<std::mutex> lock (logMutex_);
    headerName_ = headerName;
  }
  void showHeaderName (bool includeName) {
    std::lock_guard<std::mutex> lock (logMutex_);
    includeName_ = includeName;
  }
  void showHeaderTime (bool includeTime) {
    std::lock_guard<std::mutex> lock (logMutex_);
    includeTime_ = includeTime;
  }
  void showHeaderCaller (bool includeCaller) {
    std::lock_guard<std::mutex> lock (logMutex_);
    includeCaller_ = includeCaller;
  }
  void showHeaderLevel (bool includeLevel) {
    std::lock_guard<std::mutex> lock (logMutex_);
    includeLevel_ = includeLevel;
  }
  void noHeader (bool noHeader) {
    if (noHeader) {
      showHeaderName (false);
      showHeaderTime (false);
      showHeaderCaller (false);
      showHeaderLevel (false);
    } else {
      showHeaderName (true);
      showHeaderTime (true);
      showHeaderCaller (true);
      showHeaderLevel (true);
    }
  }
  void visibleHeaders (bool inlcudeName, bool includeTime, bool includeCaller, bool includeLevel) {
    showHeaderName (inlcudeName);
    showHeaderTime (includeTime);
    showHeaderCaller (includeCaller);
    showHeaderLevel (includeLevel);
  }

  void logToStream (std::ostream& stream, Level level, const std::string& message,
                    const std::string& caller, const std::tm& now_tm) {

    setConsoleColor (level);
    if (includeName_) {
      stream << "[" << headerName_ << "] ";
    }
    if (includeTime_) {
      stream << "[" << std::put_time (&now_tm, "%d-%m-%Y %H:%M:%S") << "] ";
    }
    if (includeCaller_) {
      if (!caller.empty ()) {
        stream << "[" << caller << "] ";
      }
    }
    if (includeLevel_) {
      stream << "[" << levelToString (level) << "] ";
    }
    stream << message;
    resetConsoleColor ();
    stream << std::endl;
  }

}; // class Logger

// Macro definitions

#define LOG Logger::getInstance ()
#define LOG_WITH_CALLER LOG.setCaller (FUNCTION_NAME)

// stream debug
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
  #define LOG_D Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_DEBUG
    // safe caller
  #define LOG_D_DESTRUCTOR(className) \
    Logger::getInstance ().setCaller ("~" #className "::" #className) << Logger::Level::LOG_DEBUG
#else
  #define LOG_D \
    if (false)  \
    Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_DEBUG
    // safe caller
  #define LOG_D_DESTRUCTOR(className) \
    if (false)                        \
    Logger::getInstance ().setCaller ("~" #className "::" #className) << Logger::Level::LOG_DEBUG
#endif

//stream
#define LOG_I Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_INFO
#define LOG_W Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_WARNING
#define LOG_E Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_ERROR
#define LOG_C Logger::getInstance ().setCaller (FUNCTION_NAME) << Logger::Level::LOG_CRITICAL

// functional debug
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
  #define LOG_D_MSG(msg) Logger::getInstance ().debug (msg, FUNCTION_NAME)
  // safe caller
  #define LOG_D_DESTRUCTOR_MSG(className, msg) \
    Logger::getInstance ().debug (std::string (msg), "~" #className "::" #className)
#else
  #define LOG_D_MSG(msg) ((void)0)
  // safe caller
  #define LOG_D_DESTRUCTOR_MSG(className, msg) ((void)0)
#endif

// functional
#define LOG_I_MSG(msg) Logger::getInstance ().info (msg, FUNCTION_NAME)
#define LOG_W_MSG(msg) Logger::getInstance ().warning (msg, FUNCTION_NAME)
#define LOG_E_MSG(msg) Logger::getInstance ().error (msg, FUNCTION_NAME)
#define LOG_C_MSG(msg) Logger::getInstance ().critical (msg, FUNCTION_NAME)

// fmt debug
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
  #define LOG_D_FMT(format, ...)   \
    Logger::getInstance ()         \
        .setCaller (FUNCTION_NAME) \
        .logFmtMessage (Logger::Level::LOG_DEBUG, format, __VA_ARGS__)
    // safe caller
  #define LOG_D_DESTRUCTOR_FMT(className, format, ...) \
    Logger::getInstance ()                             \
        .setCaller ("~" #className "::" #className)    \
        .logFmtMessage (Logger::Level::LOG_DEBUG, format, __VA_ARGS__)
#else
  #define LOG_D_FMT(format, ...) ((void)0)
  // safe caller
  #define LOG_D_DESTRUCTOR_FMT(className, format, ...) ((void)0)
#endif

// fmt
#define LOG_I_FMT(format, ...)   \
  Logger::getInstance ()         \
      .setCaller (FUNCTION_NAME) \
      .logFmtMessage (Logger::Level::LOG_INFO, format, __VA_ARGS__)
#define LOG_W_FMT(format, ...)   \
  Logger::getInstance ()         \
      .setCaller (FUNCTION_NAME) \
      .logFmtMessage (Logger::Level::LOG_WARNING, format, __VA_ARGS__)
#define LOG_E_FMT(format, ...)   \
  Logger::getInstance ()         \
      .setCaller (FUNCTION_NAME) \
      .logFmtMessage (Logger::Level::LOG_ERROR, format, __VA_ARGS__)
#define LOG_C_FMT(format, ...)   \
  Logger::getInstance ()         \
      .setCaller (FUNCTION_NAME) \
      .logFmtMessage (Logger::Level::LOG_CRITICAL, format, __VA_ARGS__)

#endif // LOGGER_HPP