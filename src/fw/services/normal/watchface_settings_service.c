/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "watchface_settings_service.h"

#include "kernel/pbl_malloc.h"
#include "system/logging.h"
#include "system/passert.h"

#include <string.h>

typedef struct {
  WatchfaceSetting *settings;
  uint8_t num_settings;
  Uuid uuid;
  bool has_settings;
} WatchfaceSettingsState;

static WatchfaceSettingsState s_state;

void watchface_settings_service_init(void) {
  memset(&s_state, 0, sizeof(s_state));
}

void watchface_settings_service_set(const WatchfaceSetting *settings, uint8_t num_settings,
                                    const Uuid *uuid) {
  if (num_settings > WATCHFACE_SETTINGS_MAX) {
    PBL_LOG_DBG("Watchface declared %u settings, max is %u; truncating",
            num_settings, WATCHFACE_SETTINGS_MAX);
    num_settings = WATCHFACE_SETTINGS_MAX;
  }

  // Free any previous allocation
  if (s_state.settings) {
    kernel_free(s_state.settings);
    s_state.settings = NULL;
  }

  s_state.settings = kernel_malloc_check(num_settings * sizeof(WatchfaceSetting));
  memcpy(s_state.settings, settings, num_settings * sizeof(WatchfaceSetting));
  s_state.num_settings = num_settings;
  s_state.uuid = *uuid;
  s_state.has_settings = (num_settings > 0);

  // Ensure all names are null-terminated (defensive)
  for (uint8_t i = 0; i < num_settings; i++) {
    s_state.settings[i].name[WATCHFACE_SETTING_NAME_MAX - 1] = '\0';
  }

}

void watchface_settings_service_clear(void) {
  if (s_state.settings) {
    kernel_free(s_state.settings);
    s_state.settings = NULL;
  }
  s_state.has_settings = false;
  s_state.num_settings = 0;

}

bool watchface_settings_service_has_settings(void) {
  return s_state.has_settings;
}

uint8_t watchface_settings_service_get_count(void) {
  return s_state.num_settings;
}

const WatchfaceSetting *watchface_settings_service_get_settings(void) {
  if (!s_state.has_settings) {
    return NULL;
  }
  return s_state.settings;
}

const Uuid *watchface_settings_service_get_uuid(void) {
  return &s_state.uuid;
}
