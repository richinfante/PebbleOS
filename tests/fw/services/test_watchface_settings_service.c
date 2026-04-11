/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "clar.h"

#include "services/normal/watchface_settings_service.h"
#include "applib/watchface_settings.h"
#include "util/uuid.h"

// Stubs
#include "stubs_logging.h"
#include "stubs_passert.h"
#include "stubs_pbl_malloc.h"

#define TEST_UUID \
    (UuidMake(0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x47, 0x88, \
              0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00))

#define TEST_UUID_2 \
    (UuidMake(0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x40, 0x11, \
              0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99))

static const Uuid s_test_uuid = TEST_UUID;
static const Uuid s_test_uuid_2 = TEST_UUID_2;

// Setup / Teardown

void test_watchface_settings_service__initialize(void) {
  watchface_settings_service_init();
}

void test_watchface_settings_service__cleanup(void) {
  watchface_settings_service_clear();
}

// Tests: Initial state

void test_watchface_settings_service__init_state(void) {
  cl_assert_equal_b(watchface_settings_service_has_settings(), false);
  cl_assert_equal_i(watchface_settings_service_get_count(), 0);
  cl_assert(watchface_settings_service_get_settings() == NULL);
}

// Tests: Set and get

void test_watchface_settings_service__set_single_color(void) {
  WatchfaceSetting settings[] = {
    {
      .name = "Background",
      .persist_key = 1,
      .type = WatchfaceSettingType_Color,
      .color = {
        .default_color = { .argb = GColorRedARGB8 },
        .palette = WatchfaceSettingColorPalette_Full,
      },
    },
  };

  watchface_settings_service_set(settings, 1, &s_test_uuid);

  cl_assert_equal_b(watchface_settings_service_has_settings(), true);
  cl_assert_equal_i(watchface_settings_service_get_count(), 1);

  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert(result != NULL);
  cl_assert_equal_s(result[0].name, "Background");
  cl_assert_equal_i(result[0].persist_key, 1);
  cl_assert_equal_i(result[0].type, WatchfaceSettingType_Color);
  cl_assert_equal_i(result[0].color.default_color.argb, GColorRedARGB8);
  cl_assert_equal_i(result[0].color.palette, WatchfaceSettingColorPalette_Full);
}

void test_watchface_settings_service__set_single_number(void) {
  WatchfaceSetting settings[] = {
    {
      .name = "Speed",
      .persist_key = 10,
      .type = WatchfaceSettingType_Number,
      .number = { .min = 0, .max = 100, .step = 5 },
    },
  };

  watchface_settings_service_set(settings, 1, &s_test_uuid);

  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert(result != NULL);
  cl_assert_equal_s(result[0].name, "Speed");
  cl_assert_equal_i(result[0].type, WatchfaceSettingType_Number);
  cl_assert_equal_i(result[0].number.min, 0);
  cl_assert_equal_i(result[0].number.max, 100);
  cl_assert_equal_i(result[0].number.step, 5);
}

void test_watchface_settings_service__set_multiple(void) {
  WatchfaceSetting settings[] = {
    {
      .name = "Background",
      .persist_key = 1,
      .type = WatchfaceSettingType_Color,
      .color = {
        .default_color = { .argb = GColorBlackARGB8 },
        .palette = WatchfaceSettingColorPalette_BW,
      },
    },
    {
      .name = "Interval",
      .persist_key = 2,
      .type = WatchfaceSettingType_Number,
      .number = { .min = 1, .max = 60, .step = 1 },
    },
  };

  watchface_settings_service_set(settings, 2, &s_test_uuid);

  cl_assert_equal_i(watchface_settings_service_get_count(), 2);
  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert_equal_s(result[0].name, "Background");
  cl_assert_equal_s(result[1].name, "Interval");
  cl_assert_equal_i(result[1].number.max, 60);
}

void test_watchface_settings_service__uuid_stored(void) {
  WatchfaceSetting settings[] = {
    { .name = "Test", .persist_key = 1, .type = WatchfaceSettingType_Color },
  };

  watchface_settings_service_set(settings, 1, &s_test_uuid);

  const Uuid *result_uuid = watchface_settings_service_get_uuid();
  cl_assert(uuid_equal(result_uuid, &s_test_uuid));
}

// Tests: Overwrite

void test_watchface_settings_service__overwrite_replaces(void) {
  WatchfaceSetting first[] = {
    { .name = "Old", .persist_key = 1, .type = WatchfaceSettingType_Color },
  };
  watchface_settings_service_set(first, 1, &s_test_uuid);

  WatchfaceSetting second[] = {
    { .name = "New A", .persist_key = 10, .type = WatchfaceSettingType_Number },
    { .name = "New B", .persist_key = 11, .type = WatchfaceSettingType_Number },
  };
  watchface_settings_service_set(second, 2, &s_test_uuid_2);

  cl_assert_equal_i(watchface_settings_service_get_count(), 2);
  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert_equal_s(result[0].name, "New A");
  cl_assert_equal_s(result[1].name, "New B");

  const Uuid *result_uuid = watchface_settings_service_get_uuid();
  cl_assert(uuid_equal(result_uuid, &s_test_uuid_2));
}

// Tests: Clear

void test_watchface_settings_service__clear(void) {
  WatchfaceSetting settings[] = {
    { .name = "Test", .persist_key = 1, .type = WatchfaceSettingType_Color },
  };
  watchface_settings_service_set(settings, 1, &s_test_uuid);
  cl_assert_equal_b(watchface_settings_service_has_settings(), true);

  watchface_settings_service_clear();

  cl_assert_equal_b(watchface_settings_service_has_settings(), false);
  cl_assert_equal_i(watchface_settings_service_get_count(), 0);
  cl_assert(watchface_settings_service_get_settings() == NULL);
}

void test_watchface_settings_service__clear_then_set(void) {
  WatchfaceSetting first[] = {
    { .name = "Old", .persist_key = 1, .type = WatchfaceSettingType_Color },
  };
  watchface_settings_service_set(first, 1, &s_test_uuid);
  watchface_settings_service_clear();

  WatchfaceSetting second[] = {
    { .name = "Fresh", .persist_key = 5, .type = WatchfaceSettingType_Number },
  };
  watchface_settings_service_set(second, 1, &s_test_uuid_2);

  cl_assert_equal_b(watchface_settings_service_has_settings(), true);
  cl_assert_equal_i(watchface_settings_service_get_count(), 1);
  cl_assert_equal_s(watchface_settings_service_get_settings()[0].name, "Fresh");
}

void test_watchface_settings_service__double_clear(void) {
  watchface_settings_service_clear();
  watchface_settings_service_clear();
  cl_assert_equal_b(watchface_settings_service_has_settings(), false);
}

// Tests: Truncation

void test_watchface_settings_service__truncates_at_max(void) {
  WatchfaceSetting settings[WATCHFACE_SETTINGS_MAX + 4];
  memset(settings, 0, sizeof(settings));
  for (int i = 0; i < WATCHFACE_SETTINGS_MAX + 4; i++) {
    snprintf(settings[i].name, WATCHFACE_SETTING_NAME_MAX, "S%d", i);
    settings[i].persist_key = (uint32_t)i;
    settings[i].type = WatchfaceSettingType_Number;
  }

  watchface_settings_service_set(settings, WATCHFACE_SETTINGS_MAX + 4, &s_test_uuid);

  cl_assert_equal_i(watchface_settings_service_get_count(), WATCHFACE_SETTINGS_MAX);
  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  // Verify the last stored setting is index MAX-1, not MAX+3
  char expected_name[WATCHFACE_SETTING_NAME_MAX];
  snprintf(expected_name, sizeof(expected_name), "S%d", WATCHFACE_SETTINGS_MAX - 1);
  cl_assert_equal_s(result[WATCHFACE_SETTINGS_MAX - 1].name, expected_name);
}

// Tests: Name null-termination

void test_watchface_settings_service__name_null_terminated(void) {
  WatchfaceSetting settings[] = {
    { .persist_key = 1, .type = WatchfaceSettingType_Color },
  };
  // Fill name completely with non-null bytes
  memset(settings[0].name, 'A', WATCHFACE_SETTING_NAME_MAX);

  watchface_settings_service_set(settings, 1, &s_test_uuid);

  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert_equal_i(result[0].name[WATCHFACE_SETTING_NAME_MAX - 1], '\0');
}

// Tests: Data isolation (settings array is copied, not aliased)

void test_watchface_settings_service__settings_are_copied(void) {
  WatchfaceSetting settings[] = {
    { .name = "Original", .persist_key = 1, .type = WatchfaceSettingType_Color },
  };
  watchface_settings_service_set(settings, 1, &s_test_uuid);

  // Mutate the caller's array after set
  strcpy(settings[0].name, "Mutated");

  // Service should still have the original
  const WatchfaceSetting *result = watchface_settings_service_get_settings();
  cl_assert_equal_s(result[0].name, "Original");
}
