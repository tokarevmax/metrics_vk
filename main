#include "metric_registry.h"
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#define _CRT_SECURE_NO_WARNINGS

int main() {
    MetricRegistry registry;

    auto& cpu_metric = registry.createMetric<double>("CPU");
    auto& http_metric = registry.createMetric<int>("HTTP requests RPS");
    auto& cache_metric = registry.createMetric<int>("Cache hits");

    std::thread http_worker([&http_metric]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 5);

        for (int i = 0; i < 100; ++i) {
            http_metric.add(dis(gen));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    std::thread cpu_worker([&cpu_metric]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 3.5);

        for (int i = 0; i < 100; ++i) {
            cpu_metric.set(dis(gen));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    std::thread cache_worker([&cache_metric]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::poisson_distribution<> dis(8);

        for (int i = 0; i < 100; ++i) {
            cache_metric.add(dis(gen));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    std::ofstream file("metrics.txt");
    if (!file) {
        std::cerr << "Failed to open metrics.txt" << std::endl;
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        registry.writeToFile(file);
        std::cout << "Metrics written (" << i + 1 << "/5)" << std::endl;
    }

    http_worker.join();
    cpu_worker.join();
    cache_worker.join();

    return 0;
}
