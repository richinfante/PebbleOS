/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "watchface_settings_app.h"
#include "watchface_settings_menu.h"

#include "applib/app.h"
#include "applib/ui/app_window_stack.h"
#include "resource/resource_ids.auto.h"
#include "services/common/i18n/i18n.h"
#include "services/normal/watchface_settings_service.h"

static void s_main(void) {
  watchface_settings_menu_push();
  app_event_loop();
}

const PebbleProcessMd *watchface_settings_app_get_info(void) {
  static const PebbleProcessMdSystem s_app_md = {
    .common = {
      .main_func = s_main,
      // UUID: a1b2c3d4-e5f6-4789-abcd-ef0123456789
      .uuid = { 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6, 0x47, 0x89,
                0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89 },
      .visibility = ProcessVisibilityShown,
    },
    .name = i18n_noop("WF Settings"),
    .icon_resource_id = RESOURCE_ID_WATCHFACES_APP_GLANCE,
  };
  return (const PebbleProcessMd *)&s_app_md;
}
