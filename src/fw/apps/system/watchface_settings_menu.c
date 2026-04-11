/* SPDX-FileCopyrightText: 2026 Core Devices LLC */
/* SPDX-License-Identifier: Apache-2.0 */

#include "watchface_settings_menu.h"

#include "applib/graphics/graphics.h"
#include "applib/ui/app_window_stack.h"
#include "applib/ui/menu_layer.h"
#include "applib/ui/number_window.h"
#include "applib/ui/option_menu_window.h"
#include "applib/ui/status_bar_layer.h"
#include "applib/ui/window.h"
#include "applib/watchface_settings.h"
#include "kernel/pbl_malloc.h"
#include "services/common/i18n/i18n.h"
#include "services/normal/persist.h"
#include "services/normal/settings/settings_file.h"
#include "services/normal/watchface_settings_service.h"
#include "shell/prefs.h"
#include "system/logging.h"
#include "system/passert.h"
#include "util/size.h"

#include <string.h>

// Color palette for the color picker
typedef struct {
  const char *name;
  GColor8 color;
} ColorEntry;

static const ColorEntry s_color_palette[] = {
  { "Black",          { .argb = GColorBlackARGB8 } },
  { "White",          { .argb = GColorWhiteARGB8 } },
  { "Red",            { .argb = GColorRedARGB8 } },
  { "Dark Red",       { .argb = GColorDarkCandyAppleRedARGB8 } },
  { "Orange",         { .argb = GColorOrangeARGB8 } },
  { "Chrome Yellow",  { .argb = GColorChromeYellowARGB8 } },
  { "Yellow",         { .argb = GColorYellowARGB8 } },
  { "Green",          { .argb = GColorGreenARGB8 } },
  { "Dark Green",     { .argb = GColorDarkGreenARGB8 } },
  { "Cyan",           { .argb = GColorCyanARGB8 } },
  { "Tiffany Blue",   { .argb = GColorTiffanyBlueARGB8 } },
  { "Cerulean",       { .argb = GColorVividCeruleanARGB8 } },
  { "Blue",           { .argb = GColorBlueARGB8 } },
  { "Duke Blue",      { .argb = GColorDukeBlueARGB8 } },
  { "Purple",         { .argb = GColorPurpleARGB8 } },
  { "Magenta",        { .argb = GColorMagentaARGB8 } },
  { "Pink",           { .argb = GColorBrilliantRoseARGB8 } },
  { "Sunset Orange",  { .argb = GColorSunsetOrangeARGB8 } },
  { "Lavender",       { .argb = GColorLavenderIndigoARGB8 } },
  { "Mint Green",     { .argb = GColorMintGreenARGB8 } },
  { "Light Gray",     { .argb = GColorLightGrayARGB8 } },
  { "Dark Gray",      { .argb = GColorDarkGrayARGB8 } },
};

#define NUM_COLORS ARRAY_LENGTH(s_color_palette)

// Context for the main settings menu
typedef struct {
  Window window;
  StatusBarLayer status_bar;
  MenuLayer menu_layer;
  const WatchfaceSetting *settings;
  uint8_t num_settings;
  Uuid uuid;
} WatchfaceSettingsMenuData;

// Context for the color picker sub-menu
typedef struct {
  WatchfaceSettingsMenuData *parent;
  uint8_t setting_index;
} ColorPickerContext;

// Context for the number picker
typedef struct {
  WatchfaceSettingsMenuData *parent;
  uint8_t setting_index;
} NumberPickerContext;

// Helper: read a persist value for the watchface by UUID and key
static status_t prv_persist_read(const Uuid *uuid, uint32_t key, void *val, size_t val_size) {
  SettingsFile *store = persist_service_lock_and_get_store(uuid);
  status_t result = settings_file_get(store, &key, sizeof(key), val, val_size);
  persist_service_unlock_store(store);
  return result;
}

// Helper: write a persist value for the watchface by UUID and key
static status_t prv_persist_write(const Uuid *uuid, uint32_t key,
                                  const void *val, size_t val_size) {
  SettingsFile *store = persist_service_lock_and_get_store(uuid);
  status_t result = settings_file_set(store, &key, sizeof(key), val, val_size);
  persist_service_unlock_store(store);
  return result;
}

static int prv_color_to_palette_index(GColor8 color) {
  for (size_t i = 0; i < NUM_COLORS; i++) {
    if (s_color_palette[i].color.argb == color.argb) {
      return (int)i;
    }
  }
  return 0; // default to first entry
}

/////////////////////////////
// Color picker callbacks
/////////////////////////////

static void prv_color_select(OptionMenu *option_menu, int selection, void *context) {
  ColorPickerContext *ctx = context;
  WatchfaceSettingsMenuData *data = ctx->parent;
  const WatchfaceSetting *setting = &data->settings[ctx->setting_index];

  GColor8 color = s_color_palette[selection].color;
  prv_persist_write(&data->uuid, setting->persist_key, &color, sizeof(color));

  app_window_stack_remove(&option_menu->window, true);
}

static uint16_t prv_color_get_num_rows(OptionMenu *option_menu, void *context) {
  return NUM_COLORS;
}

static void prv_color_draw_row(OptionMenu *option_menu, GContext *ctx, const Layer *cell_layer,
                                const GRect *text_frame, uint32_t row, bool selected,
                                void *context) {
  option_menu_system_draw_row(option_menu, ctx, cell_layer, text_frame,
                              s_color_palette[row].name, selected, context);
}

static void prv_color_unload(OptionMenu *option_menu, void *context) {
  ColorPickerContext *ctx = context;
  option_menu_destroy(option_menu);
  task_free(ctx);
}

static void prv_color_selection_will_change(OptionMenu *option_menu, uint16_t new_row,
                                             uint16_t old_row, void *context) {
  if (new_row != old_row) {
    GColor color = (GColor)s_color_palette[new_row].color;
    option_menu_set_highlight_colors(option_menu, color, gcolor_legible_over(color));
  }
}

static void prv_push_color_picker(WatchfaceSettingsMenuData *data, uint8_t setting_index) {
  const WatchfaceSetting *setting = &data->settings[setting_index];

  // Read current value
  GColor8 current_color = setting->default_color;
  prv_persist_read(&data->uuid, setting->persist_key, &current_color, sizeof(current_color));

  int current_index = prv_color_to_palette_index(current_color);

  OptionMenu *option_menu = option_menu_create();
  if (!option_menu) {
    return;
  }

  GColor highlight_color = (GColor)s_color_palette[current_index].color;
  const OptionMenuConfig config = {
    .title = setting->name,
    .content_type = OptionMenuContentType_SingleLine,
    .choice = current_index,
    .status_colors = { GColorWhite, GColorBlack },
    .highlight_colors = { highlight_color, gcolor_legible_over(highlight_color) },
    .icons_enabled = true,
  };
  option_menu_configure(option_menu, &config);

  ColorPickerContext *ctx = task_malloc_check(sizeof(ColorPickerContext));
  *ctx = (ColorPickerContext) {
    .parent = data,
    .setting_index = setting_index,
  };

  OptionMenuCallbacks callbacks = {
    .select = prv_color_select,
    .get_num_rows = prv_color_get_num_rows,
    .draw_row = prv_color_draw_row,
    .unload = prv_color_unload,
    .selection_will_change = prv_color_selection_will_change,
  };
  option_menu_set_callbacks(option_menu, &callbacks, ctx);

  app_window_stack_push(&option_menu->window, true);
}

/////////////////////////////
// Number picker callbacks
/////////////////////////////

static void prv_number_selected(NumberWindow *nw, void *context) {
  NumberPickerContext *ctx = context;
  WatchfaceSettingsMenuData *data = ctx->parent;
  const WatchfaceSetting *setting = &data->settings[ctx->setting_index];

  int32_t value = number_window_get_value(nw);
  prv_persist_write(&data->uuid, setting->persist_key, &value, sizeof(value));

  Window *nw_window = number_window_get_window(nw);
  app_window_stack_remove(nw_window, true);
  number_window_destroy(nw);
  task_free(ctx);
}

static void prv_push_number_picker(WatchfaceSettingsMenuData *data, uint8_t setting_index) {
  const WatchfaceSetting *setting = &data->settings[setting_index];

  // Read current value
  int32_t current_value = setting->number.min;
  prv_persist_read(&data->uuid, setting->persist_key, &current_value, sizeof(current_value));

  NumberPickerContext *ctx = task_malloc_check(sizeof(NumberPickerContext));
  *ctx = (NumberPickerContext) {
    .parent = data,
    .setting_index = setting_index,
  };

  NumberWindow *nw = number_window_create(setting->name, (NumberWindowCallbacks) {
    .selected = prv_number_selected,
  }, ctx);

  number_window_set_min(nw, setting->number.min);
  number_window_set_max(nw, setting->number.max);
  number_window_set_step_size(nw, setting->number.step > 0 ? setting->number.step : 1);
  number_window_set_value(nw, current_value);

  app_window_stack_push(number_window_get_window(nw), true);
}

/////////////////////////////
// Main settings menu callbacks
/////////////////////////////

static uint16_t prv_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index,
                                       void *context) {
  WatchfaceSettingsMenuData *data = context;
  // Always show at least one row (info row when no settings declared)
  return (data->num_settings > 0) ? data->num_settings : 1;
}

static void prv_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index,
                               void *context) {
  WatchfaceSettingsMenuData *data = context;

  if (data->num_settings == 0) {
    menu_cell_basic_draw(ctx, cell_layer,
                         "No settings available",
                         "Watchface has not declared any",
                         NULL);
    return;
  }

  const WatchfaceSetting *setting = &data->settings[cell_index->row];

  // Build subtitle showing current value
  char subtitle[32] = "";
  if (setting->type == WatchfaceSettingType_Color) {
    GColor8 color = setting->default_color;
    prv_persist_read(&data->uuid, setting->persist_key, &color, sizeof(color));
    int idx = prv_color_to_palette_index(color);
    strncpy(subtitle, s_color_palette[idx].name, sizeof(subtitle) - 1);
  } else if (setting->type == WatchfaceSettingType_Number) {
    int32_t value = setting->number.min;
    prv_persist_read(&data->uuid, setting->persist_key, &value, sizeof(value));
    snprintf(subtitle, sizeof(subtitle), "%"PRId32, value);
  }

  menu_cell_basic_draw(ctx, cell_layer, setting->name, subtitle, NULL);
}

static void prv_menu_select_click(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  WatchfaceSettingsMenuData *data = context;
  if (data->num_settings == 0) {
    return;
  }
  const WatchfaceSetting *setting = &data->settings[cell_index->row];

  switch (setting->type) {
    case WatchfaceSettingType_Color:
      prv_push_color_picker(data, cell_index->row);
      break;
    case WatchfaceSettingType_Number:
      prv_push_number_picker(data, cell_index->row);
      break;
  }
}

#if PBL_ROUND
static int16_t prv_menu_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index,
                                         void *context) {
  return menu_layer_is_index_selected(menu_layer, cell_index) ?
         MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
}
#endif

/////////////////////////////
// Window lifecycle
/////////////////////////////

static void prv_window_load(Window *window) {
  WatchfaceSettingsMenuData *data = window_get_user_data(window);

  // Status bar
  StatusBarLayer *status_bar = &data->status_bar;
  status_bar_layer_init(status_bar);
  status_bar_layer_set_colors(status_bar, GColorWhite, GColorBlack);
  layer_add_child(&window->layer, status_bar_layer_get_layer(status_bar));

  // Menu layer
  GRect bounds = window->layer.bounds;
  bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
  bounds.size.h -= STATUS_BAR_LAYER_HEIGHT;

  MenuLayer *menu = &data->menu_layer;
  menu_layer_init(menu, &bounds);
  menu_layer_set_callbacks(menu, data, &(MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)prv_menu_get_num_rows,
    .draw_row = (MenuLayerDrawRowCallback)prv_menu_draw_row,
    .select_click = (MenuLayerSelectCallback)prv_menu_select_click,
#if PBL_ROUND
    .get_cell_height = (MenuLayerGetCellHeightCallback)prv_menu_get_cell_height,
#endif
  });

  GColor highlight = shell_prefs_get_theme_highlight_color();
  menu_layer_set_highlight_colors(menu, highlight, gcolor_legible_over(highlight));
  menu_layer_set_click_config_onto_window(menu, window);
  layer_add_child(&window->layer, menu_layer_get_layer(menu));
}

static void prv_window_appear(Window *window) {
  WatchfaceSettingsMenuData *data = window_get_user_data(window);
  // Reload to refresh displayed values after sub-menus
  menu_layer_reload_data(&data->menu_layer);
}

static void prv_window_unload(Window *window) {
  WatchfaceSettingsMenuData *data = window_get_user_data(window);
  menu_layer_deinit(&data->menu_layer);
  status_bar_layer_deinit(&data->status_bar);
  // Release the watchface's persist store that we opened
  if (data->num_settings > 0) {
    persist_service_client_close(&data->uuid);
  }
  task_free(data);
}

Window *watchface_settings_menu_push(void) {
  WatchfaceSettingsMenuData *data = task_zalloc_check(sizeof(WatchfaceSettingsMenuData));

  if (watchface_settings_service_has_settings()) {
    data->settings = watchface_settings_service_get_settings();
    data->num_settings = watchface_settings_service_get_count();
    data->uuid = *watchface_settings_service_get_uuid();
    // Open the watchface's persist store so we can read/write its settings
    persist_service_client_open(&data->uuid);
  } else {
    data->settings = NULL;
    data->num_settings = 0;
  }

  Window *window = &data->window;
  window_init(window, WINDOW_NAME("WF Settings"));
  window_set_user_data(window, data);
  window_set_window_handlers(window, &(WindowHandlers) {
    .load = prv_window_load,
    .appear = prv_window_appear,
    .unload = prv_window_unload,
  });
  app_window_stack_push(window, true);
  return window;
}
