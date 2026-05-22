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

#include "HttpClient.h"

#include "Logger.h"

#include <curl/curl.h>

#include <cctype>
#include <memory>
#include <string_view>
#include <thread>

namespace MetaImGUI {

namespace {

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

int XferInfoCallback(void* clientp, curl_off_t /*dltotal*/, curl_off_t /*dlnow*/, curl_off_t /*ultotal*/,
                     curl_off_t /*ulnow*/) {
    const auto* token = static_cast<const std::stop_token*>(clientp);
    return (token != nullptr && token->stop_requested()) ? 1 : 0;
}

struct HeaderState {
    bool rateLimited = false;
};

size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    const size_t total = size * nitems;
    auto* state = static_cast<HeaderState*>(userdata);
    const std::string_view header(buffer, total);

    // GitHub returns "X-RateLimit-Remaining: 0" alongside HTTP 403 when limited.
    constexpr std::string_view kKey = "x-ratelimit-remaining:";
    if (header.size() >= kKey.size()) {
        std::string lower;
        lower.reserve(kKey.size());
        for (size_t i = 0; i < kKey.size() && i < header.size(); ++i) {
            lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(header[i]))));
        }
        if (lower == kKey) {
            std::string_view value = header.substr(kKey.size());
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
                value.remove_prefix(1);
            }
            if (!value.empty() && value.front() == '0') {
                state->rateLimited = true;
            }
        }
    }

    return total;
}

} // namespace

HttpResponse HttpClient::Get(const HttpRequest& request, const std::stop_token& stopToken) const {
    HttpResponse response;

    // Initial attempt + up to maxRetries retries on transient network errors.
    const int totalAttempts = std::max(1, request.maxRetries + 1);
    for (int attempt = 0; attempt < totalAttempts; ++attempt) {
        if (stopToken.stop_requested()) {
            response.status = HttpStatus::Cancelled;
            return response;
        }

        response = PerformOnce(request, stopToken);

        // Only NetworkError is retryable. RateLimited and Cancelled aren't
        // transient; Ok needs no retry.
        if (response.status != HttpStatus::NetworkError) {
            return response;
        }

        if (attempt + 1 >= totalAttempts) {
            break;
        }

        // Exponential backoff: 200ms, 400ms, 800ms, ... — but bail early if
        // a stop is requested mid-sleep.
        const auto backoff = std::chrono::milliseconds(200 << attempt);
        const auto deadline = std::chrono::steady_clock::now() + backoff;
        while (std::chrono::steady_clock::now() < deadline) {
            if (stopToken.stop_requested()) {
                response.status = HttpStatus::Cancelled;
                return response;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    return response;
}

HttpResponse HttpClient::Get(const HttpRequest& request) const {
    return Get(request, std::stop_token{});
}

HttpResponse HttpClient::PerformOnce(const HttpRequest& request, const std::stop_token& stopToken) const {
    HttpResponse response;

    const std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
        LOG_ERROR("HttpClient: Failed to initialise CURL handle");
        response.status = HttpStatus::NetworkError;
        return response;
    }

    HeaderState headerState;

    curl_easy_setopt(curl.get(), CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &headerState);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, request.userAgent.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, request.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, static_cast<long>(request.timeout.count()));
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);

    // Wire up the abort-on-stop progress callback so a stop_token actually
    // interrupts an in-flight transfer instead of waiting on CURLOPT_TIMEOUT.
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, XferInfoCallback);
    const std::stop_token xferStop = stopToken;
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &xferStop);

    const CURLcode res = curl_easy_perform(curl.get());

    if (res == CURLE_ABORTED_BY_CALLBACK) {
        response.body.clear();
        response.status = HttpStatus::Cancelled;
        return response;
    }

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.httpCode);

    if (res != CURLE_OK) {
        LOG_ERROR("HttpClient: Request failed: {}", curl_easy_strerror(res));
        response.body.clear();
        response.status = HttpStatus::NetworkError;
        return response;
    }

    if (response.httpCode == 403 && headerState.rateLimited) {
        response.body.clear();
        response.status = HttpStatus::RateLimited;
        return response;
    }

    if (response.httpCode >= 400) {
        LOG_ERROR("HttpClient: HTTP {}", response.httpCode);
        response.body.clear();
        response.status = HttpStatus::NetworkError;
        return response;
    }

    response.status = HttpStatus::Ok;
    return response;
}

} // namespace MetaImGUI
