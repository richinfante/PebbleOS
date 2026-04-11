/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "applib/ui/window.h"

//! Push a settings menu window for the currently running watchface.
//! The watchface must have previously called watchface_settings_declare().
//! @return The pushed Window, or NULL if no settings are available.
Window *watchface_settings_menu_push(void);
