/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "applib/graphics/gtypes.h"

#include <stdint.h>
#include <stdbool.h>

//! @addtogroup Foundation
//! @{
//!   @addtogroup WatchfaceSettings
//! \brief Allows watchfaces to declare configurable settings that appear in a system menu.
//!
//! A watchface calls watchface_settings_declare() during initialization, passing an array
//! of WatchfaceSetting descriptors. Each descriptor maps a display name and type to a
//! persist storage key. The system will show a "Settings" option in the watchface carousel
//! for watchfaces that have declared settings.
//!
//! Settings values are stored in the watchface's normal persist storage and can be
//! read by the watchface using the standard persist_read_* APIs.
//!   @{

//! Maximum number of settings a watchface can declare
#define WATCHFACE_SETTINGS_MAX 16

//! Maximum length of a setting name (including null terminator)
#define WATCHFACE_SETTING_NAME_MAX 32

//! Types of settings a watchface can declare
typedef enum {
  //! A color value (GColor). Presented as a color picker.
  WatchfaceSettingType_Color = 0,
  //! An integer value. Presented as a NumberWindow with min/max/step.
  WatchfaceSettingType_Number = 1,
} WatchfaceSettingType;

//! Configuration for a number-type setting
typedef struct {
  int32_t min;
  int32_t max;
  int32_t step;
} WatchfaceSettingNumberConfig;

//! Descriptor for a single watchface setting
typedef struct {
  //! Display name shown in the settings menu
  char name[WATCHFACE_SETTING_NAME_MAX];
  //! The persist storage key used to store this setting's value
  uint32_t persist_key;
  //! The type of this setting
  WatchfaceSettingType type;
  //! Type-specific configuration
  union {
    //! Configuration for WatchfaceSettingType_Number
    WatchfaceSettingNumberConfig number;
    //! Default color for WatchfaceSettingType_Color (used if no value persisted yet)
    GColor8 default_color;
  };
} WatchfaceSetting;

//! Declare the settings for this watchface. Call once during watchface initialization.
//! The settings array is copied by the system; it does not need to remain valid after the call.
//! @param settings Array of setting descriptors
//! @param num_settings Number of entries in the array (max WATCHFACE_SETTINGS_MAX)
void watchface_settings_declare(const WatchfaceSetting *settings, uint8_t num_settings);

//!   @} // group WatchfaceSettings
//! @} // group Foundation
