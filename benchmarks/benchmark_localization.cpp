// Localization benchmarks
#include "Localization.h"

#include <benchmark/benchmark.h>

using namespace MetaImGUI;

// Benchmark simple translation lookup
static void BM_LocalizationGet(benchmark::State& state) {
    // Initialize with built-in translations
    Localization::Instance().InitializeBuiltInTranslations();
    Localization::Instance().SetLanguage("en");

    for (auto _ : state) {
        benchmark::DoNotOptimize(Localization::Instance().Tr("menu.file"));
    }
}
BENCHMARK(BM_LocalizationGet);

// Benchmark language switching
static void BM_LocalizationSwitchLanguage(benchmark::State& state) {
    Localization::Instance().InitializeBuiltInTranslations();

    bool toggle = false;
    for (auto _ : state) {
        if (toggle) {
            Localization::Instance().SetLanguage("en");
        } else {
            Localization::Instance().SetLanguage("es");
        }
        toggle = !toggle;
    }
}
BENCHMARK(BM_LocalizationSwitchLanguage);

// Benchmark adding translation
static void BM_LocalizationAdd(benchmark::State& state) {
    int counter = 0;

    for (auto _ : state) {
        std::string key = "test.key." + std::to_string(counter++);
        Localization::Instance().AddTranslation("en", key, "Test value");
    }
}
BENCHMARK(BM_LocalizationAdd);

// Benchmark multiple lookups
static void BM_LocalizationMultipleLookups(benchmark::State& state) {
    Localization::Instance().InitializeBuiltInTranslations();
    Localization::Instance().SetLanguage("en");

    const std::vector<std::string> keys = {"menu.file", "menu.edit", "menu.view",
                                           "menu.help", "button.ok", "button.cancel"};

    for (auto _ : state) {
        for (const auto& key : keys) {
            benchmark::DoNotOptimize(Localization::Instance().Tr(key));
        }
    }
}
BENCHMARK(BM_LocalizationMultipleLookups);

// Benchmark missing key lookup
static void BM_LocalizationMissingKey(benchmark::State& state) {
    Localization::Instance().InitializeBuiltInTranslations();

    for (auto _ : state) {
        benchmark::DoNotOptimize(Localization::Instance().Tr("nonexistent.key.that.does.not.exist"));
    }
}
BENCHMARK(BM_LocalizationMissingKey);
