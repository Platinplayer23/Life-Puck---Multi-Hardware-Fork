#include "history.h"
#include <lvgl.h>
#include <constants/constants.h>
#include <menu/menu.h>
#include <state/state_store.h>
#include <life/life_counter.h>
#include <life/life_counter2P.h>

extern lv_obj_t *history_menu;

void renderHistoryOverlay()
{
  teardownHistoryOverlay();
  PlayerMode player_mode = (PlayerMode)player_store.getInt(KEY_PLAYER_MODE, PLAYER_MODE_ONE_PLAYER);
  
  history_menu = lv_obj_create(lv_scr_act());
  lv_obj_set_size(history_menu, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(history_menu, BLACK_COLOR, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(history_menu, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(history_menu, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {60, SCREEN_HEIGHT - 140, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(history_menu, col_dsc, row_dsc);

  lv_obj_t *btn_back = lv_btn_create(history_menu);
  lv_obj_set_size(btn_back, 100, 60);
  lv_obj_set_style_bg_color(btn_back, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_grid_cell(btn_back, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_obj_t *lbl_back = lv_label_create(btn_back);
  lv_label_set_text(lbl_back, LV_SYMBOL_LEFT " Back");
  lv_obj_set_style_text_font(lbl_back, &lv_font_montserrat_20, 0);
  lv_obj_center(lbl_back);
  lv_obj_set_style_text_color(lbl_back, lv_color_black(), 0);
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) { renderMenu(MENU_CONTEXTUAL, false); }, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *table = lv_table_create(history_menu);
  lv_obj_set_grid_cell(table, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_obj_set_style_bg_color(table, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_scroll_dir(table, LV_DIR_VER);
  
  if (player_mode == PLAYER_MODE_TWO_PLAYER) {
    // 2 Spalten
    lv_table_set_col_cnt(table, 2);
    lv_table_set_col_width(table, 0, (SCREEN_WIDTH - 20) / 2);
    lv_table_set_col_width(table, 1, (SCREEN_WIDTH - 20) / 2);
    lv_table_set_row_cnt(table, 1);
    lv_table_set_cell_value(table, 0, 0, "P1");
    lv_table_set_cell_value(table, 0, 1, "P2");
    
    std::vector<LifeHistoryEvent> h1 = event_grouper_p1.getHistory();
    std::vector<LifeHistoryEvent> h2 = event_grouper_p2.getHistory();
    
    size_t max_rows = (h1.size() > h2.size()) ? h1.size() : h2.size();
    
    for (size_t i = 0; i < max_rows; ++i) {
      size_t row_idx = i + 1;
      lv_table_set_row_cnt(table, row_idx + 1);
      
      // P1 Spalte
      if (i < h1.size()) {
        const LifeHistoryEvent &evt = h1[i];
        char buf[32];
        if (evt.net_life_change > 0)
          snprintf(buf, sizeof(buf), "+%d [%d]", evt.net_life_change, evt.life_total);
        else
          snprintf(buf, sizeof(buf), "%d [%d]", evt.net_life_change, evt.life_total);
        lv_table_set_cell_value(table, row_idx, 0, buf);
      } else {
        lv_table_set_cell_value(table, row_idx, 0, "");
      }
      
      // P2 Spalte
      if (i < h2.size()) {
        const LifeHistoryEvent &evt = h2[i];
        char buf[32];
        if (evt.net_life_change > 0)
          snprintf(buf, sizeof(buf), "+%d [%d]", evt.net_life_change, evt.life_total);
        else
          snprintf(buf, sizeof(buf), "%d [%d]", evt.net_life_change, evt.life_total);
        lv_table_set_cell_value(table, row_idx, 1, buf);
      } else {
        lv_table_set_cell_value(table, row_idx, 1, "");
      }
    }
  } else {
    // 1 Spalte
    lv_table_set_col_cnt(table, 1);
    lv_table_set_col_width(table, 0, SCREEN_WIDTH - 20);
    lv_table_set_row_cnt(table, 1);
    lv_table_set_cell_value(table, 0, 0, "History");
    
    std::vector<LifeHistoryEvent> history = event_grouper.getHistory();
    
    for (size_t i = 0; i < history.size(); ++i) {
      const LifeHistoryEvent &evt = history[i];
      char buf[32];
      if (evt.net_life_change > 0)
        snprintf(buf, sizeof(buf), "+%d [%d]", evt.net_life_change, evt.life_total);
      else
        snprintf(buf, sizeof(buf), "%d [%d]", evt.net_life_change, evt.life_total);
      
      size_t row_idx = i + 1;
      lv_table_set_row_cnt(table, row_idx + 1);
      lv_table_set_cell_value(table, row_idx, 0, buf);
    }
  }
}

void teardownHistoryOverlay() {
  if (history_menu) {
    lv_obj_del(history_menu);
    history_menu = nullptr;
  }
}
