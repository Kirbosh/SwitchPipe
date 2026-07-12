#pragma once

#include <string>

#include "newpipe/auth_store.hpp"
#include "newpipe/http_client.hpp"

namespace newpipe {

enum class AccountActionStatus {
    Ok,
    NotSignedIn,
    MissingTarget,
    RequestFailed,
    Unsupported,
};

struct AccountActionResult {
    AccountActionStatus status = AccountActionStatus::RequestFailed;
    std::string message;

    bool ok() const { return status == AccountActionStatus::Ok; }
};

// Authenticated write actions against YouTube's innertube API. These require a
// stored login session (cookies + SAPISIDHASH) from AuthStore. Read-only feeds
// live in YouTubeCatalogService; this class only performs account mutations.
class YouTubeAccountService {
public:
    explicit YouTubeAccountService(HttpClient* client = nullptr, AuthStore* auth_store = nullptr);

    bool is_signed_in() const;

    AccountActionResult like_video(const std::string& video_id);
    AccountActionResult remove_like(const std::string& video_id);
    AccountActionResult dislike_video(const std::string& video_id);
    AccountActionResult subscribe_channel(const std::string& channel_id);
    AccountActionResult unsubscribe_channel(const std::string& channel_id);
    AccountActionResult post_comment(const std::string& video_url, const std::string& text);

private:
    AccountActionResult perform_target_action(
        const std::string& endpoint,
        const std::string& video_id,
        const char* log_tag);
    AccountActionResult perform_subscription_action(
        const std::string& endpoint,
        const std::string& channel_id,
        const char* log_tag);

    HttpsHttpClient owned_client_;
    HttpClient* client_ = nullptr;
    AuthStore* auth_store_ = nullptr;
};

}  // namespace newpipe
