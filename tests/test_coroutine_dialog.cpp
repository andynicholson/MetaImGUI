/*
    MetaImGUI
    Copyright (C) 2026  A P Nicholson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "Coroutine.h"

#include <catch2/catch_test_macros.hpp>

#include <coroutine>
#include <functional>
#include <string>
#include <utility>

using MetaImGUI::Task;

// Mini awaiter that mimics how DialogManager::AwaitConfirmation suspends
// the coroutine and resumes it from a callback. We don't pull in the real
// DialogManager here because it transitively requires ImGui — the awaiter
// shape is the contract we care about.
namespace {

struct ManualResumeBoolAwaiter {
    std::function<void(std::function<void(bool)>)> registerCallback;
    bool result = false;

    bool await_ready() const noexcept {
        return false;
    }
    void await_suspend(std::coroutine_handle<> handle) {
        registerCallback([this, handle](bool value) mutable {
            result = value;
            handle.resume();
        });
    }
    bool await_resume() const noexcept {
        return result;
    }
};

} // namespace

TEST_CASE("Task coroutine starts eagerly and resumes from callback", "[coroutine]") {
    std::function<void(bool)> resume;
    bool completed = false;
    bool answer = false;

    auto run = [&]() -> Task {
        const bool result = co_await ManualResumeBoolAwaiter{
            [&](std::function<void(bool)> cb) { resume = std::move(cb); }, false};
        answer = result;
        completed = true;
    };

    // Calling run() starts the body up to the first co_await, then suspends.
    run();
    REQUIRE_FALSE(completed);
    REQUIRE(static_cast<bool>(resume));

    resume(true);
    REQUIRE(completed);
    REQUIRE(answer == true);
}

TEST_CASE("Task: awaiter sees cancellation as a 'no' result", "[coroutine]") {
    std::function<void(bool)> resume;
    bool completed = false;
    bool answer = true;

    auto run = [&]() -> Task {
        const bool result = co_await ManualResumeBoolAwaiter{
            [&](std::function<void(bool)> cb) { resume = std::move(cb); }, false};
        answer = result;
        completed = true;
    };

    run();
    resume(false);
    REQUIRE(completed);
    REQUIRE(answer == false);
}

TEST_CASE("Task: chained co_awaits run sequentially", "[coroutine]") {
    std::function<void(bool)> resume1;
    std::function<void(bool)> resume2;
    int step = 0;

    auto run = [&]() -> Task {
        step = 1;
        co_await ManualResumeBoolAwaiter{[&](std::function<void(bool)> cb) { resume1 = std::move(cb); }, false};
        step = 2;
        co_await ManualResumeBoolAwaiter{[&](std::function<void(bool)> cb) { resume2 = std::move(cb); }, false};
        step = 3;
    };

    run();
    REQUIRE(step == 1);
    REQUIRE(static_cast<bool>(resume1));
    REQUIRE_FALSE(static_cast<bool>(resume2));

    resume1(true);
    REQUIRE(step == 2);
    REQUIRE(static_cast<bool>(resume2));

    resume2(true);
    REQUIRE(step == 3);
}
