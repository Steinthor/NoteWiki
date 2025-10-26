#pragma once

// remote logging dependencies
#include <arpa/inet.h>          // for sockaddr_in, htons(), inet_pton()
#include <cstring>              // for strerror(), strlen()
#include <sys/socket.h>         // for AF_INET, SOCK_STREAM, socket(), send()
// other
#include <atomic>               // for std::atomic<>
#include <chrono>               // for std::put_time(), std::setfill(), std::setw(), 
#include <condition_variable>   // for std::condition_variable
#include <fstream>              // for std::ofstream
#include <functional>           // for std::function
#include <iostream>             // for std::cout
#include <mutex>                // for std::mutex, std::lock_guard<>
#include <queue>                // for std::priority_queue
#include <sstream>              // for std::ostringstream
#include <thread>               // for std::thread

#include "thread_id.h"          // for thread_id_to_hex()
#include "color.h"              // for terminal colors.

// an enum for various levels of logging
enum struct LogLevel : uint8_t {
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

// a buffer size for remote server response
#define BUFFER_SIZE 1024

// LogMessage object that includes a log level, timestamp, message, thread id, 
// and supports comparison based on timestamp
struct LogMessage {
    LogLevel level;
    std::chrono::system_clock::time_point timestamp;
    std::string message;
    std::thread::id thread_id;

    bool operator<(const LogMessage& other) const {
        return timestamp < other.timestamp;
    }

    // Priority queue requires greater-than for min-heap behavior
    bool operator>(const LogMessage& other) const {
        return timestamp > other.timestamp;
    }
};

// A thread-safe priority queue to send messages to, so working threads aren't
//   blocked from slow I/O
// A priority queue with std::greater<T> means the smallest time/oldest timestamp gets
//   written out first.
// That does not guarantee the written order of timestamps, it only guarantees
//   the order currently in the queue
template<typename T>
class ThreadSafePriorityQueue  {
private:
    std::priority_queue<T, std::vector<T>, std::greater<T>> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    void emplace(const T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.emplace(item);
        cv.notify_one();
    }

    bool pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !queue.empty(); });
        // This 'wait' means the thread processing 'pop' is blocked until awakened.
        if(queue.empty()) {
            return false;
        }
        queue.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    auto top() {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.top();
    }
};

#ifdef LOGGER_TEST_HOOKS
using LogTestObserver = void(*)(const LogMessage&);
inline std::atomic<LogTestObserver> g_testObserver{nullptr}; // internal linkage
inline void setLogTestObserver(LogTestObserver cb) {
    g_testObserver.store(cb, std::memory_order_release);
}
#endif

// Logger class is a singleton, so should only be constructed once.
// default: logging to console: true, logging to file: false.
// * The processing of the logs is in its own thread so as not to delay other threads
// * Along with the macros defined below allows the use of the '<<' operator:
//      ex.: LOG_INFO() << "variable: " << var;
// * Timestamp format can be customized with 'setTimestampFormatter'
class Logger {
public:
    using TimestampFormatter = std::function<std::string(const std::chrono::system_clock::time_point&)>;

    // Helper class, allows using << for a log message.
    // Nested so it has access to Loggers' functions.
    class LogStream {
    private:
        Logger& logger;
        std::ostringstream oss;
        LogLevel level;
        std::chrono::system_clock::time_point timestamp;

    public:
        LogStream(Logger& logger, LogLevel level) :
            logger(logger), level(level), timestamp(std::chrono::system_clock::now()) {}

        // Overload << operator to accumulate strings/variables.
        template<typename T>
        LogStream& operator<<(const T& msg) {
            if (logger.isLoggingLevel(level)) {
                oss << msg;
            }
            return *this;
        }

        // Destructor sends accumulated message to logger
        ~LogStream() {
            if (logger.isLoggingLevel(level)) {
                logger.logMessage(level, timestamp, oss.str());
            }
        }
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Delete copy constructor and assignment operator so it's a singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void enableConsoleLogging(bool enable) {
        logToConsole = enable;
    }

    /**
     * @brief Enables or disables file logging.
     * 
     * This function is not thread-safe and should not be called concurrently from multiple threads.
     * Call this function during the setup phase or ensure it is properly synchronized if called at runtime.
     * 
     * @param filename The filename to log to. If empty, disables file logging.
     */
    void enableFileLogging(const std::string& filename) {
        // If an empty string is given, it is a call to disable file logging
        if (filename.empty()) {
            if (fileStream.is_open()) {
                fileStream.close();
            }
            logToFile = false;
            this->filename.clear();
        } else {
            // Close the current file if we are opening a new file
            if (fileStream.is_open() && filename != this->filename) {
                fileStream.close();
            }
            // Open the new file
            fileStream.open(filename, std::ios::out | std::ios::app);
            if (!fileStream) {
                throw std::runtime_error("Unable to open file: " + filename);
            }
            logToFile = true;
            this->filename = filename;
        }
    }

    /**
     * @brief Enables remote logging with a valid IP addres or disables remote logging with an empty string
     * 
     * This function is not thread-safe and should not be called concurrently from multiple threads.
     * Call this function during the setup phase or ensure it is properly synchronized if called at runtime.
     * 
     * @param ip_address string with a valid IP address, or empty string to disable remote logging
     * @param port the port to use for the remote connection
     */
    void enableServerLogging(std::string ip_address = "127.0.0.1", int port = 8080) {
        std::lock_guard<std::mutex> lock(serverMtx);
        close_socket();
        if (ip_address.empty()) {
            logToServer = false;
            return;
        }
        setup_socket();
        setup_address(ip_address, port);
        connect_address();
        logToServer = true;
    }

    // Used by the LOG_* macros below to allow '<<' operation.
    LogStream log(LogLevel level) {
        return LogStream(*this, level);
    }

    // Allows customizing the timestamp.
    // To reset formatter to default use 'nullptr' as input
    void setTimestampFormatter(TimestampFormatter formatter) {
        std::lock_guard<std::mutex> lock(queueMtx);
        this->formatter = formatter;
    }

    // To ensure that the queue is empty before manipulating the logger
    void waitForQueueToEmpty() {
        std::unique_lock<std::mutex> lock(queueMtx);
        queueCv.wait(lock, [this] { return logQueue.empty(); });
    }

    void setLogLevel(LogLevel lvl) noexcept { loglevel_ = lvl; }

private:
    Logger() : logToConsole(true),
               logToFile(false),
               logToServer(false),
               active_socket(-1),
               done(false) {
        loggerThread = std::thread(&Logger::processQueue, this);
    }

    ~Logger() {
        done = true;
        queueCv.notify_all();
        if (loggerThread.joinable()) {
            loggerThread.join();
        }
        if (fileStream.is_open()) {
            fileStream.close();
        }
    }

    // Adds a LogMessage object to the log queue.
    void logMessage(const LogLevel level,
                    const std::chrono::system_clock::time_point& timestamp,
                    const std::string& message) {
        std::thread::id thread_id = std::this_thread::get_id();
        LogMessage msg{level, timestamp, message, thread_id};
#ifdef LOGGER_TEST_HOOKS
        if (auto cb = g_testObserver.load(std::memory_order_acquire)) cb(msg);
#endif
        std::lock_guard<std::mutex> lock(queueMtx);
        logQueue.emplace(msg);
        queueCv.notify_one();
    }

    // Helper function to format the timestamp.
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) const {
        if (formatter) {
            return formatter(timestamp);
        } else {
            // Default format "YYYY-MM-DD hh:mm:ss.mmm.uuu.nnn" 
            // (mmm is milliseconds, uuu is microseconds, nnn is nanoseconds)
            auto duration = timestamp.time_since_epoch();
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - 
                std::chrono::duration_cast<std::chrono::milliseconds>(seconds);
            auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration) - 
                std::chrono::duration_cast<std::chrono::microseconds>(seconds + milliseconds);
            auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration) - 
                std::chrono::duration_cast<std::chrono::nanoseconds>(seconds + milliseconds + microseconds);

            // Convert to time_t to get the time as a string
            std::time_t time_t_seconds = std::chrono::system_clock::to_time_t(timestamp);
            std::tm* tm = std::localtime(&time_t_seconds);

            // Format the time as a string
            std::ostringstream oss;
            oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << '.'
                << std::setw(3) << std::setfill('0') << milliseconds.count() << '.'
                << std::setw(3) << std::setfill('0') << microseconds.count() << '.'
                << std::setw(3) << std::setfill('0') << nanoseconds.count();

            return oss.str();
        }
    }

    // Remote server connection helper functions.
    void close_socket() {
        if (active_socket != -1) {
            close(active_socket);
            active_socket = -1;
        }
    }

    // Remote server basic socket setup.
    void setup_socket() {
        const int max_tries = 5;
        int count = 0;
        bool active = false;
        while (!active && count < max_tries) {
            if ((active_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            }
            else active = true;
            ++count;
        }
        if (!active) {
            std::cerr << "Creating a socket failed: " << strerror(errno) << ": " << active_socket;
            exit(EXIT_FAILURE);
        }
    }

    // Remote server address setup.
    void setup_address(std::string ip_address = "127.0.0.1", int port = 8080) {
        address.sin_family = AF_INET;
        // address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Converting IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, ip_address.c_str(), &address.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported";
            exit(EXIT_FAILURE);
        }
    }

    // Remote server connection.
    void connect_address() {
        // Connecting to the server
        if (connect(active_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Connection Failed";
            exit(EXIT_FAILURE);
        }
    }

    // Send message to remote server.
    void send_message(int active_socket, std::string message) {
        if (active_socket < 0) {
            std::cerr << "Trying to send a message to a non-socket: " << active_socket;
            return;
        }

        // get thread id string
        std::string id =  "[þr:" + thread_id_to_hex() + "] ";

        // Sending a message to the server
        ssize_t sent_bytes = send(active_socket, message.c_str(), strlen(message.c_str()), 0);
        if (sent_bytes == -1) {
            std::cerr << "Failed to send message: " << strerror(errno);
            exit(EXIT_FAILURE);
        }

        // Reading the server's response
        ssize_t received_bytes = read(active_socket, buffer, BUFFER_SIZE);
        if (received_bytes < 0) {
            std::cerr << "Failed to read from server: " << strerror(errno);
            exit(EXIT_FAILURE);
        }
        //buffer[received_bytes] = '\0'; // Null-terminate the buffer
        //std::cout << id << "inside logger::send_message() response from server: " << buffer << ", bytes: " << received_bytes << "\n";
    }
    /* end of remote server connection functions*/

    // Function that loops in the Logger thread
    void processQueue() {
        while (!done) {
            std::unique_lock<std::mutex> lock(queueMtx);
            queueCv.wait(lock, [this] { return !logQueue.empty() || done; });

            if (done && logQueue.empty()) {
                break;
            }

            while (!logQueue.empty()) {
                const LogMessage& log = logQueue.top();
                logQueue.pop();
                lock.unlock();
                std::chrono::system_clock::time_point timestamp = log.timestamp;
                std::string message = log.message;
                LogLevel level = log.level;
                // get thread id string
                std::string thread_id =  "[þr:" + thread_id_to_hex(log.thread_id) + "] ";

                auto color = color::RED;
                std::ostringstream output;
                output << logLevelToColor(level) \
                       << formatTimestamp(timestamp) \
                       << " [" << logLevelToString(level) << "] " \
                       << thread_id \
                       << message \
                       << color::RESET \
                       ;

                if (logToConsole) {
                    std::cout << output.str() << std::endl;
                }
                if (logToFile && fileStream.is_open()) {
                    fileStream << output.str() << std::endl;
                }
                if (logToServer && active_socket > 0) {
                    send_message(active_socket, output.str());
                }

                lock.lock();
            }
            // let other threads know the queue is empty,
            // ex: threads waiting on waitForQueueToEmpty()
            queueCv.notify_all();
        }
    }

    bool isLoggingLevel(LogLevel level) const {
#ifdef DEBUGGING
        if (level == LogLevel::DEBUG) {
            return true;
        }
#endif
        return (level <= loglevel_ ? true : false);
    }

    // Helper function to return the log level color.
    std::string_view logLevelToColor(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG: return color::GREY;
            case LogLevel::INFO: return color::WHITE;
            case LogLevel::WARNING: return color::YELLOW;
            case LogLevel::ERROR: return color::RED;
            default: return "";
        }
    }

    // Helper function to return the log level as a string
    std::string logLevelToString(LogLevel level) const {
        std::ostringstream oss;
        switch (level) {
            case LogLevel::DEBUG: oss << "DEBUG"; break;
            case LogLevel::INFO: oss << "INFO"; break;
            case LogLevel::WARNING: oss << "WARNING"; break;
            case LogLevel::ERROR: oss << "ERROR"; break;
            default: break;
        }
        return oss.str();
    }

    std::atomic<bool> done;
    std::atomic<bool> logToConsole;
    std::atomic<bool> logToFile;
    std::atomic<bool> logToServer;
    std::atomic<LogLevel> loglevel_ = {LogLevel::WARNING};
    std::ofstream fileStream;
    std::string filename;
    ThreadSafePriorityQueue<LogMessage> logQueue;
    std::thread loggerThread;
    std::mutex queueMtx;
    std::mutex serverMtx;
    int active_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    std::condition_variable queueCv;
    TimestampFormatter formatter;
};

// logging macros
#define LOG_ERROR() Logger::getInstance().log(LogLevel::ERROR)
#define LOG_WARNING() Logger::getInstance().log(LogLevel::WARNING)
#define LOG_INFO() Logger::getInstance().log(LogLevel::INFO)
#ifdef DEBUGGING
    #define LOG_DEBUG() Logger::getInstance().log(LogLevel::DEBUG)
#else
    #define LOG_DEBUG() if (false) Logger::getInstance().log(LogLevel::DEBUG)
#endif
