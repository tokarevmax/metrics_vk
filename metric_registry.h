#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <fstream>
#include <thread>
#include <type_traits>
#include <mutex>

class MetricBase {
public:
    virtual ~MetricBase() = default;
    virtual std::string getName() const = 0;
    virtual std::string getAndResetAsString() = 0;
};

template <typename T>
class Metric : public MetricBase {
public:
    Metric(const std::string& name, T initial = T{})
        : name_(name), value_(initial) {
    }

    void set(T value) {
        value_.store(value, std::memory_order_relaxed);
    }

    template <typename U = T>
    auto add(U value) -> std::enable_if_t<std::is_integral<U>::value || std::is_floating_point<U>::value> {
        if constexpr (std::is_integral_v<U> || std::is_floating_point_v<U>) {
            value_.fetch_add(static_cast<T>(value), std::memory_order_relaxed);
        }
    }

    std::string getName() const override {
        return name_;
    }

    std::string getAndResetAsString() override {
        T val = value_.exchange(T{}, std::memory_order_relaxed);
        if constexpr (std::is_integral_v<T>) {
            return std::to_string(val);
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3) << val;
            return oss.str();
        }
        else {
            return std::to_string(val);
        }
    }

private:
    std::string name_;
    std::atomic<T> value_;
};

class MetricRegistry {
public:
    template <typename T>
    Metric<T>& createMetric(const std::string& name, T initial = T{}) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto metric = std::make_unique<Metric<T>>(name, initial);
        Metric<T>& ref = *metric;
        metrics_.push_back(std::move(metric));
        return ref;
    }

    void writeToFile(std::ofstream& file) {
        if (!file.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm now_tm;
        localtime_s(&now_tm, &now_time_t);

        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            file << oss.str();

            for (auto& metric : metrics_) {
                file << " \"" << metric->getName() << "\" "
                    << metric->getAndResetAsString();
            }
            file << std::endl;
        }
    }

private:
    std::vector<std::unique_ptr<MetricBase>> metrics_;
    std::mutex mutex_;
};
