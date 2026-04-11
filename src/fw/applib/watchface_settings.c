/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "watchface_settings.h"

#include "process_management/process_manager.h"
#include "services/normal/watchface_settings_service.h"
#include "syscall/syscall.h"
#include "syscall/syscall_internal.h"
#include "system/passert.h"

DEFINE_SYSCALL(void, watchface_settings_declare,
               const WatchfaceSetting *settings, uint8_t num_settings) {
  if (num_settings == 0 || settings == NULL) {
    return;
  }

  if (num_settings > WATCHFACE_SETTINGS_MAX) {
    num_settings = WATCHFACE_SETTINGS_MAX;
  }

  if (PRIVILEGE_WAS_ELEVATED) {
    syscall_assert_userspace_buffer(settings, num_settings * sizeof(WatchfaceSetting));
  }

  const Uuid *uuid = &sys_process_manager_get_current_process_md()->uuid;
  watchface_settings_service_set(settings, num_settings, uuid);
}
