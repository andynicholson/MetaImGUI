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

#include <chrono>
#include <stop_token>
#include <string>

namespace MetaImGUI {

/**
 * @brief Result classification for an HTTP fetch.
 *
 * Mirrors the failure modes the rest of the codebase already cared about
 * (see UpdateChecker), promoted to a shared vocabulary.
 */
enum class HttpStatus {
    Ok,            ///< 2xx response, body populated
    RateLimited,   ///< 403 with X-RateLimit-Remaining: 0 (GitHub-style)
    NetworkError,  ///< curl/transport failure, or non-2xx that isn't rate-limit
    Cancelled      ///< stop_token::stop_requested() fired during transfer
};

struct HttpResponse {
    HttpStatus status = HttpStatus::NetworkError;
    long httpCode = 0;
    std::string body;
};

struct HttpRequest {
    std::string url;
    std::string userAgent = "MetaImGUI/1.0";
    std::chrono::seconds timeout{10};
    bool followRedirects = true;
    int maxRetries = 0; ///< Retries on transient network failure (not on rate-limit/cancel/4xx).
};

/**
 * @brief Thin RAII wrapper around libcurl_easy for application-side fetches.
 *
 * Centralises the boilerplate that UpdateChecker and ISSTracker had each
 * re-implemented: SSL verification, stop_token-driven xferinfo abort,
 * GitHub rate-limit header parsing, and exponential backoff on retry.
 *
 * The design is deliberately stateless — every Get() is independent — so
 * callers can hold a single HttpClient instance for the lifetime of the
 * app and share it across threads.
 */
class HttpClient {
public:
    HttpClient() = default;
    ~HttpClient() = default;

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) = default;
    HttpClient& operator=(HttpClient&&) = default;

    /**
     * @brief Issue a GET request, honouring stop_token cancellation.
     *
     * Retries on HttpStatus::NetworkError up to request.maxRetries times,
     * with exponential backoff (200ms, 400ms, 800ms, ...). Cancellation
     * and rate-limit responses are returned immediately without retry —
     * those aren't transient.
     */
    HttpResponse Get(const HttpRequest& request, const std::stop_token& stopToken) const;

    /// Convenience overload for the common no-cancel case.
    HttpResponse Get(const HttpRequest& request) const;

private:
    HttpResponse PerformOnce(const HttpRequest& request, const std::stop_token& stopToken) const;
};

} // namespace MetaImGUI
