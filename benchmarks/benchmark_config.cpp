// ConfigManager benchmarks
#include "ConfigManager.h"

#include <benchmark/benchmark.h>

using namespace MetaImGUI;

// Benchmark config loading
static void BM_ConfigLoad(benchmark::State& state) {
    // Setup - save a config first
    {
        ConfigManager config;
        config.SetString("test_key", "test_value");
        config.SetInt("test_int", 42);
        config.SetBool("test_bool", true);
        config.SetWindowSize(1920, 1080);
        config.Save();
    }

    for (auto _ : state) {
        ConfigManager config;
        benchmark::DoNotOptimize(config.Load());
    }
}
BENCHMARK(BM_ConfigLoad);

// Benchmark config saving
static void BM_ConfigSave(benchmark::State& state) {
    for (auto _ : state) {
        ConfigManager config;
        config.SetString("key1", "value1");
        config.SetString("key2", "value2");
        config.SetInt("number", 123);
        config.SetBool("flag", true);
        config.SetWindowSize(1920, 1080);
        benchmark::DoNotOptimize(config.Save());
    }
}
BENCHMARK(BM_ConfigSave);

// Benchmark getting values
static void BM_ConfigGetString(benchmark::State& state) {
    ConfigManager config;
    config.SetString("test_key", "test_value");

    for (auto _ : state) {
        benchmark::DoNotOptimize(config.GetString("test_key"));
    }
}
BENCHMARK(BM_ConfigGetString);

// Benchmark setting values
static void BM_ConfigSetString(benchmark::State& state) {
    ConfigManager config;

    for (auto _ : state) {
        config.SetString("test_key", "test_value");
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_ConfigSetString);

// Benchmark multiple operations
static void BM_ConfigMultipleOps(benchmark::State& state) {
    for (auto _ : state) {
        ConfigManager config;
        config.SetString("key1", "value1");
        config.SetString("key2", "value2");
        config.SetInt("number", 123);
        config.SetBool("flag", true);
        config.SetWindowSize(1920, 1080);

        benchmark::DoNotOptimize(config.GetString("key1"));
        benchmark::DoNotOptimize(config.GetInt("number"));
        benchmark::DoNotOptimize(config.GetBool("flag"));
        benchmark::DoNotOptimize(config.GetWindowSize());

        config.Save();
        ConfigManager config2;
        config2.Load();
    }
}
BENCHMARK(BM_ConfigMultipleOps);
