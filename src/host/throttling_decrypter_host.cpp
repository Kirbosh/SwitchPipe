// Host-only stub for ThrottlingDecrypter.
//
// The real implementation (src/common/throttling_decrypter.cpp) embeds QuickJS to
// run YouTube's "n" parameter transform. QuickJS is wired in through the Switch
// CMake build only, so the host validation binary (make host) links this no-op
// instead. On host we cannot execute the player JS, so leaving the URL untouched
// matches the real code's own fallback when the transform can't be loaded.
#ifndef __SWITCH__

#include "newpipe/throttling_decrypter.hpp"

namespace newpipe {

ThrottlingDecrypter::ThrottlingDecrypter(HttpClient* client)
    : client_(client ? client : &owned_client_) {}

std::string ThrottlingDecrypter::apply(const std::string& url) {
    return url;
}

void ThrottlingDecrypter::invalidate() {
    cached_player_js_url_.clear();
    cached_function_body_.clear();
}

bool ThrottlingDecrypter::ensure_function_loaded(const std::string&) {
    return false;
}

std::optional<std::string> ThrottlingDecrypter::transform_n(const std::string&) {
    return std::nullopt;
}

std::optional<std::string> ThrottlingDecrypter::fetch_player_js_url(const std::string&) {
    return std::nullopt;
}

std::optional<std::string> ThrottlingDecrypter::fetch_player_js(const std::string&) {
    return std::nullopt;
}

std::optional<std::string> ThrottlingDecrypter::extract_throttle_function(const std::string&) {
    return std::nullopt;
}

}  // namespace newpipe

#endif  // __SWITCH__
