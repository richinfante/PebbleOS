/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "applib/watchface_settings.h"
#include "util/uuid.h"

#include <stdbool.h>
#include <stdint.h>

//! Initialize the watchface settings service. Called at boot.
void watchface_settings_service_init(void);

//! Store settings declarations for the current app. Called from syscall context.
//! @param settings Array of setting descriptors (already validated as userspace buffer)
//! @param num_settings Number of entries
//! @param uuid UUID of the declaring app
void watchface_settings_service_set(const WatchfaceSetting *settings, uint8_t num_settings,
                                    const Uuid *uuid);

//! Clear any stored settings declarations (called when an app exits).
void watchface_settings_service_clear(void);

//! Check if the currently running watchface has declared settings.
//! @return true if settings are available
bool watchface_settings_service_has_settings(void);

//! Get the number of declared settings.
uint8_t watchface_settings_service_get_count(void);

//! Get the array of declared settings. Returns NULL if none declared.
const WatchfaceSetting *watchface_settings_service_get_settings(void);

//! Get the UUID of the app that declared the settings.
const Uuid *watchface_settings_service_get_uuid(void);
