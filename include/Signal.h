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

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MetaImGUI {

template <typename... Args> class Signal;

/**
 * @brief RAII handle that owns a single subscription to a Signal.
 *
 * A Connection disconnects automatically on destruction, on assignment, or
 * via Reset(). It is move-only — copying would let two owners try to
 * disconnect the same slot.
 *
 * If the parent Signal is destroyed first, the Connection becomes a no-op:
 * Reset() / destruction is safe.
 */
class Connection {
public:
    Connection() = default;
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept : m_disconnect(std::exchange(other.m_disconnect, {})) {}

    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            Reset();
            m_disconnect = std::exchange(other.m_disconnect, {});
        }
        return *this;
    }

    ~Connection() {
        Reset();
    }

    /// Disconnect now (idempotent).
    void Reset() {
        if (m_disconnect) {
            m_disconnect();
            m_disconnect = nullptr;
        }
    }

    /// Detach without disconnecting — caller takes responsibility.
    void Release() {
        m_disconnect = nullptr;
    }

    [[nodiscard]] bool IsConnected() const noexcept {
        return static_cast<bool>(m_disconnect);
    }

private:
    template <typename... Args> friend class Signal;
    explicit Connection(std::function<void()> disconnect) : m_disconnect(std::move(disconnect)) {}

    std::function<void()> m_disconnect;
};

/**
 * @brief A single-threaded multicast event source.
 *
 * Slots receive every Emit() in connection order. The Signal stores its slot
 * table in a shared control block, so a Connection that outlives the Signal
 * harmlessly disconnects nothing rather than dereferencing a dangling pointer.
 *
 * Not thread-safe — Emit / Connect / Disconnect must all happen on the same
 * thread (the UI thread, in this codebase). Cross-thread fan-out is the
 * caller's job (see the m_pendingUpdateResult mutex hand-off in Application).
 */
template <typename... Args> class Signal {
public:
    using SlotId = std::size_t;
    using Slot = std::function<void(Args...)>;

    Signal() = default;

    // Non-copyable: a copy would have ambiguous ownership of the slot table.
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    // Non-movable: outstanding Connections capture a weak_ptr to our state,
    // which a move would invalidate the address-stability assumption that
    // simpler designs rely on. Heap-store the Signal if you need to move it.
    Signal(Signal&&) = delete;
    Signal& operator=(Signal&&) = delete;

    ~Signal() = default;

    /**
     * @brief Subscribe a slot. Returns a RAII handle that disconnects on drop.
     */
    [[nodiscard]] Connection Connect(Slot slot) {
        const SlotId id = m_state->nextId++;
        m_state->slots.emplace(id, std::move(slot));
        std::weak_ptr<State> weak = m_state;
        return Connection([weak, id]() {
            if (auto state = weak.lock()) {
                state->slots.erase(id);
            }
        });
    }

    /**
     * @brief Fire the signal. Each connected slot is invoked once with @p args.
     *
     * Slots are invoked from a snapshot, so a slot that disconnects itself or
     * another slot during emission is safe — the snapshot is unaffected.
     */
    void Emit(Args... args) const {
        std::vector<Slot> snapshot;
        snapshot.reserve(m_state->slots.size());
        for (const auto& entry : m_state->slots) {
            snapshot.push_back(entry.second);
        }
        for (auto& slot : snapshot) {
            slot(args...);
        }
    }

    [[nodiscard]] std::size_t SlotCount() const noexcept {
        return m_state->slots.size();
    }

private:
    struct State {
        std::unordered_map<SlotId, Slot> slots;
        SlotId nextId = 0;
    };
    std::shared_ptr<State> m_state = std::make_shared<State>();
};

} // namespace MetaImGUI
