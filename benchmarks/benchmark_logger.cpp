// Logger benchmarks
#include "Logger.h"

#include <benchmark/benchmark.h>

using namespace MetaImGUI;

// Setup and cleanup
class LoggerFixture : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State&) override {
        // Enable file output for realistic benchmarks
        Logger::Instance().SetFileOutput(true);
    }

    void TearDown(const ::benchmark::State&) override {
        Logger::Instance().SetFileOutput(false);
    }
};

// Benchmark basic Info logging
BENCHMARK_F(LoggerFixture, BM_LoggerInfo)(benchmark::State& state) {
    for (auto _ : state) {
        Logger::Instance().Info("Test info message");
    }
}

// Benchmark Debug logging (might be filtered)
BENCHMARK_F(LoggerFixture, BM_LoggerDebug)(benchmark::State& state) {
    Logger::Instance().SetLevel(LogLevel::Debug);

    for (auto _ : state) {
        Logger::Instance().Debug("Test debug message");
    }
}

// Benchmark Error logging
BENCHMARK_F(LoggerFixture, BM_LoggerError)(benchmark::State& state) {
    for (auto _ : state) {
        Logger::Instance().Error("Test error message");
    }
}

// Benchmark formatted logging
BENCHMARK_F(LoggerFixture, BM_LoggerFormatted)(benchmark::State& state) {
    int counter = 0;

    for (auto _ : state) {
        Logger::Instance().Info("Formatted message: {} {}", counter++, "test");
    }
}

// Benchmark high-frequency logging
BENCHMARK_F(LoggerFixture, BM_LoggerHighFrequency)(benchmark::State& state) {
    for (auto _ : state) {
        for (int i = 0; i < 10; ++i) {
            Logger::Instance().Info("High frequency log message");
        }
    }
}

// Benchmark logging with filtering
BENCHMARK_F(LoggerFixture, BM_LoggerFiltered)(benchmark::State& state) {
    // Set level to Error so Debug and Info are filtered
    Logger::Instance().SetLevel(LogLevel::Error);

    for (auto _ : state) {
        Logger::Instance().Debug("This is filtered");
        Logger::Instance().Info("This is also filtered");
    }
}
