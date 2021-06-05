#pragma once

#include "Common.h"

enum IC_UsageMenu
{
    IC_USAGE_BASIC = 0,
    IC_USAGE_DRAW,
    IC_USAGE_TEXT,
    IC_USAGE_BRUSH,
    IC_USAGE_COLOR,
    IC_USAGE_BOUNDARY,
    IC_USAGE_BOUNDARY_TRIANGLE,
    IC_USAGE_MERGE,
    IC_USAGE_ACTION,
    IC_USAGE_MENU_AMOUNT,
};

struct UsagePageData
{
    std_tstring description;
    int image_resource_id;
};

extern std::vector<std::vector<UsagePageData> > g_all_usage_data;
