/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "app_launcher.h"

#include <mooncake.h>
#include <mooncake_log.h>

#include "ui_home.h"

using namespace mooncake;

AppLauncher::AppLauncher()
{
    setAppInfo().name = "AppLauncher";
}

void AppLauncher::onCreate()
{
    mclog::tagInfo(getAppInfo().name, "on create");

    open();
}

void AppLauncher::onOpen()
{
    mclog::tagInfo(getAppInfo().name, "on open");

    ui_home_create(nullptr);
}

void AppLauncher::onRunning()
{
    // Static wireframe has no dynamic updates yet.
}

void AppLauncher::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");

    _view.reset();
}
