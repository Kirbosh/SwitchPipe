#include "newpipe/account_actions.hpp"

#include <optional>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "newpipe/log.hpp"

namespace newpipe {
namespace {

using nlohmann::json;

constexpr const char* kInnertubeBase = "https://www.youtube.com/youtubei/v1/";
constexpr const char* kOrigin = "https://www.youtube.com";
constexpr const char* kWebClientVersion = "2.20250403.01.00";
constexpr const char* kWebUserAgent =
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36";

json web_context() {
    return {
        {"client",
         {{"clientName", "WEB"},
          {"clientVersion", kWebClientVersion},
          {"hl", "en"},
          {"gl", "US"},
          {"platform", "DESKTOP"}}},
    };
}

// Extracts a "createCommentParams" token from the raw watch-page HTML. This
// token authorizes posting a top-level comment and changes per video.
std::optional<std::string> find_json_string_value(const std::string& text, const std::string& key) {
    const std::string pattern = "\"" + key + "\":\"";
    const size_t start = text.find(pattern);
    if (start == std::string::npos) {
        return std::nullopt;
    }

    const size_t value_start = start + pattern.size();
    const size_t value_end = text.find('"', value_start);
    if (value_end == std::string::npos) {
        return std::nullopt;
    }

    return text.substr(value_start, value_end - value_start);
}

}  // namespace

YouTubeAccountService::YouTubeAccountService(HttpClient* client, AuthStore* auth_store)
    : client_(client ? client : &owned_client_)
    , auth_store_(auth_store ? auth_store : &AuthStore::instance()) {
    std::string ignored_error;
    this->auth_store_->load(&ignored_error);
}

bool YouTubeAccountService::is_signed_in() const {
    return this->auth_store_->has_session();
}

AccountActionResult YouTubeAccountService::perform_target_action(
    const std::string& endpoint,
    const std::string& video_id,
    const char* log_tag) {
    if (!this->auth_store_->has_session()) {
        return {AccountActionStatus::NotSignedIn, "Sign in to use this action"};
    }
    if (video_id.empty()) {
        return {AccountActionStatus::MissingTarget, "Missing video id"};
    }

    const std::string referer = std::string(kOrigin) + "/watch?v=" + video_id;
    std::string auth_error;
    auto headers = this->auth_store_->build_youtube_headers(kOrigin, referer, &auth_error);
    if (!auth_error.empty()) {
        return {AccountActionStatus::NotSignedIn, auth_error};
    }
    headers.push_back({"Content-Type", "application/json"});
    headers.push_back({"User-Agent", kWebUserAgent});
    headers.push_back({"X-Youtube-Client-Name", "1"});
    headers.push_back({"X-Youtube-Client-Version", kWebClientVersion});

    const json payload = {
        {"target", {{"videoId", video_id}}},
        {"context", web_context()},
    };

    const std::string url = std::string(kInnertubeBase) + endpoint + "?prettyPrint=false";
    const auto response = this->client_->post(url, payload.dump(), headers);
    if (!response.has_value()) {
        logf("account: %s request failed video=%s", log_tag, video_id.c_str());
        return {AccountActionStatus::RequestFailed, "YouTube rejected the request"};
    }

    logf("account: %s ok video=%s", log_tag, video_id.c_str());
    return {AccountActionStatus::Ok, {}};
}

AccountActionResult YouTubeAccountService::perform_subscription_action(
    const std::string& endpoint,
    const std::string& channel_id,
    const char* log_tag) {
    if (!this->auth_store_->has_session()) {
        return {AccountActionStatus::NotSignedIn, "Sign in to use this action"};
    }
    if (channel_id.empty()) {
        return {AccountActionStatus::MissingTarget, "Missing channel id"};
    }

    const std::string referer = std::string(kOrigin) + "/channel/" + channel_id;
    std::string auth_error;
    auto headers = this->auth_store_->build_youtube_headers(kOrigin, referer, &auth_error);
    if (!auth_error.empty()) {
        return {AccountActionStatus::NotSignedIn, auth_error};
    }
    headers.push_back({"Content-Type", "application/json"});
    headers.push_back({"User-Agent", kWebUserAgent});
    headers.push_back({"X-Youtube-Client-Name", "1"});
    headers.push_back({"X-Youtube-Client-Version", kWebClientVersion});

    const json payload = {
        {"channelIds", json::array({channel_id})},
        {"params", ""},
        {"context", web_context()},
    };

    const std::string url = std::string(kInnertubeBase) + endpoint + "?prettyPrint=false";
    const auto response = this->client_->post(url, payload.dump(), headers);
    if (!response.has_value()) {
        logf("account: %s request failed channel=%s", log_tag, channel_id.c_str());
        return {AccountActionStatus::RequestFailed, "YouTube rejected the request"};
    }

    logf("account: %s ok channel=%s", log_tag, channel_id.c_str());
    return {AccountActionStatus::Ok, {}};
}

AccountActionResult YouTubeAccountService::like_video(const std::string& video_id) {
    return this->perform_target_action("like/like", video_id, "like");
}

AccountActionResult YouTubeAccountService::remove_like(const std::string& video_id) {
    return this->perform_target_action("like/removelike", video_id, "removelike");
}

AccountActionResult YouTubeAccountService::dislike_video(const std::string& video_id) {
    return this->perform_target_action("like/dislike", video_id, "dislike");
}

AccountActionResult YouTubeAccountService::subscribe_channel(const std::string& channel_id) {
    return this->perform_subscription_action("subscription/subscribe", channel_id, "subscribe");
}

AccountActionResult YouTubeAccountService::unsubscribe_channel(const std::string& channel_id) {
    return this->perform_subscription_action("subscription/unsubscribe", channel_id, "unsubscribe");
}

AccountActionResult YouTubeAccountService::post_comment(
    const std::string& video_url,
    const std::string& text) {
    if (!this->auth_store_->has_session()) {
        return {AccountActionStatus::NotSignedIn, "Sign in to use this action"};
    }
    if (text.empty()) {
        return {AccountActionStatus::MissingTarget, "Comment text is empty"};
    }
    if (video_url.empty()) {
        return {AccountActionStatus::MissingTarget, "Missing video url"};
    }

    // The comment token is embedded in the watch page and authorizes this
    // specific video's comment box. Fetch it with the authenticated cookie so
    // it matches the signed-in account.
    std::string auth_error;
    auto page_headers = this->auth_store_->build_youtube_headers(kOrigin, video_url, &auth_error);
    if (!auth_error.empty()) {
        return {AccountActionStatus::NotSignedIn, auth_error};
    }
    page_headers.push_back({"User-Agent", kWebUserAgent});

    const auto page = this->client_->get(video_url, page_headers);
    if (!page.has_value()) {
        return {AccountActionStatus::RequestFailed, "Could not load the video page"};
    }

    const auto create_params = find_json_string_value(*page, "createCommentParams");
    if (!create_params.has_value() || create_params->empty()) {
        return {AccountActionStatus::Unsupported, "Commenting is not available for this video"};
    }

    auto headers = this->auth_store_->build_youtube_headers(kOrigin, video_url, &auth_error);
    if (!auth_error.empty()) {
        return {AccountActionStatus::NotSignedIn, auth_error};
    }
    headers.push_back({"Content-Type", "application/json"});
    headers.push_back({"User-Agent", kWebUserAgent});
    headers.push_back({"X-Youtube-Client-Name", "1"});
    headers.push_back({"X-Youtube-Client-Version", kWebClientVersion});

    const json payload = {
        {"commentText", text},
        {"createCommentParams", *create_params},
        {"context", web_context()},
    };

    const std::string url = std::string(kInnertubeBase) + "comment/create_comment?prettyPrint=false";
    const auto response = this->client_->post(url, payload.dump(), headers);
    if (!response.has_value()) {
        return {AccountActionStatus::RequestFailed, "YouTube rejected the comment"};
    }

    const json root = json::parse(*response, nullptr, false);
    if (!root.is_discarded() && root.contains("actionResults")) {
        // A populated actionResults array signals the comment was accepted.
        const auto& results = root.at("actionResults");
        if (results.is_array() && !results.empty()) {
            log_line("account: comment posted");
            return {AccountActionStatus::Ok, {}};
        }
    }

    log_line("account: comment response missing confirmation");
    return {AccountActionStatus::Ok, "Comment sent"};
}

}  // namespace newpipe
