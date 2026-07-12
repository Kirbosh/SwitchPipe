#pragma once

#include <borealis.hpp>

#include "newpipe/youtube_catalog_service.hpp"
#include "view/auto_tab_frame.hpp"

class MainActivity : public brls::Activity {
public:
    CONTENT_FROM_XML_RES("activity/main.xml");

    void onContentAvailable() override;

private:
    void openGlobalSearch();

    newpipe::YouTubeCatalogService service_;

    BRLS_BIND(AutoTabFrame, tabsFrame, "main/tabs");
};
