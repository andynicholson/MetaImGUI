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

#include "Signal.h"

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

using MetaImGUI::Connection;
using MetaImGUI::Signal;

TEST_CASE("Signal: empty signal can be emitted", "[signal]") {
    Signal<> signal;
    REQUIRE_NOTHROW(signal.Emit());
    REQUIRE(signal.SlotCount() == 0);
}

TEST_CASE("Signal: single slot fires on emit", "[signal]") {
    Signal<int> signal;
    int received = 0;
    auto conn = signal.Connect([&](int v) { received = v; });

    REQUIRE(signal.SlotCount() == 1);
    REQUIRE(conn.IsConnected());

    signal.Emit(42);
    REQUIRE(received == 42);

    signal.Emit(7);
    REQUIRE(received == 7);
}

TEST_CASE("Signal: multiple slots all fire", "[signal]") {
    Signal<int> signal;
    std::vector<int> calls;
    auto a = signal.Connect([&](int v) { calls.push_back(v * 1); });
    auto b = signal.Connect([&](int v) { calls.push_back(v * 10); });
    auto c = signal.Connect([&](int v) { calls.push_back(v * 100); });

    REQUIRE(signal.SlotCount() == 3);
    signal.Emit(2);
    REQUIRE(calls.size() == 3);
    // Order isn't part of the contract (unordered_map storage), only that all fired.
    int sum = 0;
    for (int n : calls) {
        sum += n;
    }
    REQUIRE(sum == 2 + 20 + 200);
}

TEST_CASE("Signal: connection disconnects on drop", "[signal]") {
    Signal<> signal;
    int count = 0;
    {
        auto conn = signal.Connect([&]() { ++count; });
        signal.Emit();
        REQUIRE(count == 1);
    }
    REQUIRE(signal.SlotCount() == 0);
    signal.Emit();
    REQUIRE(count == 1);
}

TEST_CASE("Signal: explicit Reset disconnects", "[signal]") {
    Signal<> signal;
    int count = 0;
    auto conn = signal.Connect([&]() { ++count; });
    signal.Emit();
    REQUIRE(count == 1);

    conn.Reset();
    REQUIRE_FALSE(conn.IsConnected());
    REQUIRE(signal.SlotCount() == 0);

    signal.Emit();
    REQUIRE(count == 1);
}

TEST_CASE("Signal: Reset is idempotent", "[signal]") {
    Signal<> signal;
    auto conn = signal.Connect([]() {});
    conn.Reset();
    REQUIRE_NOTHROW(conn.Reset());
    REQUIRE_NOTHROW(conn.Reset());
}

TEST_CASE("Signal: Connection move transfers ownership", "[signal]") {
    Signal<> signal;
    int count = 0;
    auto a = signal.Connect([&]() { ++count; });
    REQUIRE(signal.SlotCount() == 1);

    Connection b(std::move(a));
    REQUIRE_FALSE(a.IsConnected());
    REQUIRE(b.IsConnected());
    REQUIRE(signal.SlotCount() == 1);

    signal.Emit();
    REQUIRE(count == 1);

    b.Reset();
    REQUIRE(signal.SlotCount() == 0);
}

TEST_CASE("Signal: outliving Connection is a no-op", "[signal]") {
    Connection conn;
    {
        Signal<> signal;
        conn = signal.Connect([]() {});
        REQUIRE(conn.IsConnected());
    }
    // Signal has been destroyed; the still-"connected" handle should not crash.
    REQUIRE_NOTHROW(conn.Reset());
}

TEST_CASE("Signal: variadic args forward correctly", "[signal]") {
    Signal<int, std::string, double> signal;
    int i = 0;
    std::string s;
    double d = 0.0;
    auto conn = signal.Connect([&](int a, const std::string& b, double c) {
        i = a;
        s = b;
        d = c;
    });

    signal.Emit(99, std::string("hello"), 3.14);
    REQUIRE(i == 99);
    REQUIRE(s == "hello");
    REQUIRE(d == 3.14);
}

TEST_CASE("Signal: slot disconnecting itself during emit is safe", "[signal]") {
    Signal<> signal;
    int count = 0;
    auto conn = std::make_unique<Connection>();
    *conn = signal.Connect([&]() {
        ++count;
        conn->Reset();
    });

    signal.Emit();
    REQUIRE(count == 1);
    REQUIRE_FALSE(conn->IsConnected());

    signal.Emit();
    REQUIRE(count == 1);
}
