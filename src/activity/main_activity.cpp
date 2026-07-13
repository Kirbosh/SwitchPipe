#include "activity/main_activity.hpp"

#include <utility>

#include "activity/stream_feed_activity.hpp"
#include "newpipe/i18n.hpp"
#include "newpipe/settings_store.hpp"

namespace {

size_t startup_tab_index(const std::string& tab_id) {
    if (tab_id == "search") {
        return 1;
    }
    if (tab_id == "subscriptions") {
        return 2;
    }
    if (tab_id == "library") {
        return 3;
    }
    if (tab_id == "settings") {
        return 4;
    }
    return 0;
}

}  // namespace

void MainActivity::onContentAvailable() {
    this->registerAction(newpipe::tr("common/info"), brls::ControllerButton::BUTTON_Y, [this](brls::View*) {
        auto* dialog = new brls::Dialog(newpipe::tr("app/info_body", APP_VERSION));
        dialog->addButton(newpipe::tr("hints/ok"), [dialog]() { dialog->close(); });
        dialog->setCancelable(true);
        dialog->open();
        return true;
    });

    // Global search: reachable from every tab with ZL, so search is one press
    // away instead of requiring a trip to the dedicated Search tab.
    this->registerAction(
        newpipe::tr("search/global_action"),
        brls::ControllerButton::BUTTON_LT,
        [this](brls::View*) {
            this->openGlobalSearch();
            return true;
        });

    if (this->tabsFrame) {
        const size_t index = startup_tab_index(newpipe::SettingsStore::instance().settings().startup_tab);
        this->tabsFrame->setDefaultTabIndex(index);
        this->tabsFrame->focusTab(static_cast<int>(index));
    }
}

void MainActivity::openGlobalSearch() {
    brls::Application::getImeManager()->openForText(
        [this](const std::string& text) {
            if (text.empty()) {
                return;
            }

            const auto results = this->service_.search(text);
            if (results.items.empty()) {
                brls::Application::notify(
                    this->service_.error_message().empty()
                        ? newpipe::tr("search/no_results", text)
                        : this->service_.error_message());
                return;
            }

            auto items = results.items;
            brls::Application::pushActivity(
                new StreamFeedActivity(newpipe::tr("search/results_count", text, items.size()), std::move(items)));
        },
        newpipe::tr("search/ime_title"),
        newpipe::tr("search/ime_subtitle"),
        80,
        "");
}
