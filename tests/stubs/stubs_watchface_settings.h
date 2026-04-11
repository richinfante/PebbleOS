/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "services/normal/watchface_settings_service.h"

void watchface_settings_service_init(void) {}

void watchface_settings_service_set(const WatchfaceSetting *settings, uint8_t num_settings,
                                    const Uuid *uuid) {}

void watchface_settings_service_clear(void) {}

bool watchface_settings_service_has_settings(void) { return false; }

uint8_t watchface_settings_service_get_count(void) { return 0; }

const WatchfaceSetting *watchface_settings_service_get_settings(void) { return NULL; }

const Uuid *watchface_settings_service_get_uuid(void) { return NULL; }
