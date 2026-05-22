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

#pragma once

#include <coroutine>
#include <exception>

namespace MetaImGUI {

/**
 * @brief Eager fire-and-forget coroutine return type.
 *
 * Both initial and final suspend return `suspend_never`, so:
 *   - the coroutine starts immediately on call,
 *   - the coroutine frame is destroyed automatically when the body returns.
 *
 * Use this for "kick off an async UI flow and forget" — e.g. a dialog
 * sequence that drives itself via co_await on UI events. Exceptions that
 * escape the body terminate; coroutines on the UI thread should not throw.
 *
 * Lifetime contract: any awaiter inside the coroutine must keep the data
 * it captures alive at least until it resumes the handle. The coroutine
 * frame holds the awaiter, so capturing `this` on a stack-resident
 * awaiter is safe.
 */
struct Task {
    struct promise_type {
        Task get_return_object() noexcept {
            return {};
        }
        std::suspend_never initial_suspend() noexcept {
            return {};
        }
        std::suspend_never final_suspend() noexcept {
            return {};
        }
        void return_void() noexcept {}
        void unhandled_exception() noexcept {
            // UI-thread coroutines should not throw — terminate so the bug
            // is loud rather than silently dropping the rest of the flow.
            std::terminate();
        }
    };
};

} // namespace MetaImGUI
