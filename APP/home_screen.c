/**
 ******************************************************************************
 * @file        home_screen.c
 * @version     V3.0
 * @brief       复古"撕页老黄历"主界面 - 适老化智能陪伴机器人
 ******************************************************************************
 * @attention   左侧农历日期+节气，右侧天气图标+温度
 *              大字体设计，适合老年人阅读
 ******************************************************************************
 */

#include "home_screen.h"
#include "ai_assistant_ui.h"
#include "ui_theme.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "home_screen";
LV_FONT_DECLARE(myFont24)   /* 声明myFont24字体 */

/* ==================== 老黄历颜色定义 ==================== */
/* 复古纸张色调 - 继承主题暖色系 */
#define ALMANAC_PAPER_BG        THEME_CARD_BG               /* 暖黄纸张背景 */
#define ALMANAC_PAPER_BORDER    lv_color_hex(0xD4B896)      /* 纸边棕色 */
#define ALMANAC_RED             THEME_ACCENT_RED            /* 中国红（强调色） */
#define ALMANAC_GOLD            THEME_ACCENT_GOLD           /* 金色（吉日标记） */
#define ALMANAC_TEXT_BLACK      THEME_TEXT_PRIMARY          /* 正文深咖色 */
#define ALMANAC_TEXT_GRAY       THEME_TEXT_SECONDARY        /* 副文本暖灰棕 */

/* ==================== 屏幕对象 ==================== */
static lv_obj_t *s_home_screen = NULL;
static lv_obj_t *s_almanac_container = NULL;    /* 老黄历主容器 */
static lv_obj_t *s_left_panel = NULL;           /* 左侧农历面板 */
static lv_obj_t *s_right_panel = NULL;          /* 右侧天气面板 */

/* ==================== 测试用静态数据 ==================== */
/* 【写死的测试数据】后续可替换为真实农历算法 */
static const char *s_lunar_year = "二O二六年";
static const char *s_lunar_month_day = "二月 初五";
static const char *s_solar_date = "2026年3月24日";
static const char *s_weekday = "星期二";
static const char *s_solar_term = "春分";
static const char *s_suitable = "宜:祈福 出行 嫁娶";
static const char *s_avoid = "忌:动土 安葬";

/* 天气数据（模拟） */
static const char *s_weather_temp = "18 C";
static const char *s_weather_desc = "晴朗";
static const char *s_weather_humidity = "湿度 65%";

/* ==================== 导航栏回调函数 ==================== */
static void nav_btn_standby_cb(lv_event_t *e)
{
    ui_switch_to_standby();
}

static void nav_btn_control_cb(lv_event_t *e)
{
    ui_switch_to_control();
}

static void nav_btn_viewer_cb(lv_event_t *e)
{
    ui_switch_to_viewer();
}

/**
 * @brief       模拟唤醒按钮回调 - 触发唤醒词
 * @param       e: 事件句柄
 * @retval      无
 */
static void simulate_wake_btn_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "模拟唤醒词触发，准备切换到小柚子化身界面...");
    ui_switch_to_avatar();
}

/**
 * @brief       模拟手势按钮回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void simulate_gesture_btn_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "模拟手势识别...");
    ui_show_gesture_feedback("<--上一首音乐");
}

/**
 * @brief       创建底部导航栏
 * @param       parent: 父对象
 * @retval      无
 */
static void create_nav_bar(lv_obj_t *parent)
{
    /* 导航栏容器 */
    lv_obj_t *nav_bar = lv_obj_create(parent);
    lv_obj_set_size(nav_bar, LV_HOR_RES, 65);
    lv_obj_align(nav_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(nav_bar, THEME_CARD_BG, LV_PART_MAIN);
    lv_obj_set_style_border_width(nav_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(nav_bar, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_border_opa(nav_bar, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_flex_flow(nav_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(nav_bar, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(nav_bar, 8, LV_PART_MAIN);

    /* 息屏按钮 */
    lv_obj_t *btn_standby = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_standby, 70, 50);
    ui_apply_button_dark_style(btn_standby);
    lv_obj_t *label_standby = lv_label_create(btn_standby);
    lv_label_set_text(label_standby, "Sleep");
    lv_obj_set_style_text_font(label_standby, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_standby);
    lv_obj_add_event_cb(btn_standby, nav_btn_standby_cb, LV_EVENT_CLICKED, NULL);

    /* 主页按钮 - 高亮显示当前界面 */
    lv_obj_t *btn_home = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_home, 70, 50);
    ui_apply_button_light_style(btn_home);
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Home");
    lv_obj_set_style_text_font(label_home, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_home);

    /* 便签板按钮 */
    lv_obj_t *btn_control = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_control, 70, 50);
    ui_apply_button_dark_style(btn_control);
    lv_obj_t *label_control = lv_label_create(btn_control);
    lv_label_set_text(label_control, "Note");
    lv_obj_set_style_text_font(label_control, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_control);
    lv_obj_add_event_cb(btn_control, nav_btn_control_cb, LV_EVENT_CLICKED, NULL);

    /* 查看器按钮 */
    lv_obj_t *btn_viewer = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_viewer, 70, 50);
    ui_apply_button_dark_style(btn_viewer);
    lv_obj_t *label_viewer = lv_label_create(btn_viewer);
    lv_label_set_text(label_viewer, "View");
    lv_obj_set_style_text_font(label_viewer, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_viewer);
    lv_obj_add_event_cb(btn_viewer, nav_btn_viewer_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief       创建左侧农历日期面板
 * @param       parent: 父对象
 * @retval      面板对象指针
 */
static lv_obj_t* create_lunar_panel(lv_obj_t *parent)
{
    /* 左侧面板 - 农历日期区域 */
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(58));  /* 占58%宽度 */
    lv_obj_set_height(panel, lv_pct(100));
    lv_obj_set_style_radius(panel, THEME_RADIUS_LARGE, LV_PART_MAIN);

    /* 复古纸张背景 */
    lv_obj_set_style_bg_color(panel, ALMANAC_PAPER_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, ALMANAC_PAPER_BORDER, LV_PART_MAIN);

    /* 添加阴影效果，模拟纸张质感 */
    lv_obj_set_style_shadow_width(panel, 15, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(panel, THEME_SHADOW_COLOR, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(panel, THEME_SHADOW_OPA, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(panel, 5, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(panel, 5, LV_PART_MAIN);

    /* 使用Flex布局，垂直排列 */
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(panel, THEME_PAD_LARGE, LV_PART_MAIN);
    lv_obj_set_style_pad_row(panel, 8, LV_PART_MAIN);

    /* ========== 农历年份 ========== */
    lv_obj_t *lunar_year_label = lv_label_create(panel);
    lv_label_set_text(lunar_year_label, s_lunar_year);
    lv_obj_set_style_text_font(lunar_year_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(lunar_year_label, ALMANAC_TEXT_GRAY, LV_PART_MAIN);

    /* ========== 农历月日（大字） ========== */
    lv_obj_t *lunar_day_label = lv_label_create(panel);
    lv_label_set_text(lunar_day_label, s_lunar_month_day);
    lv_obj_set_style_text_font(lunar_day_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(lunar_day_label, ALMANAC_TEXT_BLACK, LV_PART_MAIN);

    /* ========== 公历日期 ========== */
    lv_obj_t *solar_label = lv_label_create(panel);
    char solar_str[64];
    snprintf(solar_str, sizeof(solar_str), "%s %s", s_solar_date, s_weekday);
    lv_label_set_text(solar_label, solar_str);
    lv_obj_set_style_text_font(solar_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(solar_label, ALMANAC_TEXT_GRAY, LV_PART_MAIN);

    /* ========== 节气标签（红色强调） ========== */
    lv_obj_t *term_container = lv_obj_create(panel);
    lv_obj_set_width(term_container, lv_pct(80));
    lv_obj_set_height(term_container, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(term_container, ALMANAC_RED, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(term_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(term_container, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_pad_all(term_container, THEME_PAD_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_border_width(term_container, 0, LV_PART_MAIN);

    lv_obj_t *term_label = lv_label_create(term_container);
    lv_label_set_text(term_label, s_solar_term);
    lv_obj_set_style_text_font(term_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(term_label, THEME_TEXT_ON_DARK, LV_PART_MAIN);
    lv_obj_center(term_label);

    /* ========== 分隔线 ========== */
    lv_obj_t *separator = lv_obj_create(panel);
    lv_obj_set_width(separator, lv_pct(90));
    lv_obj_set_height(separator, 2);
    lv_obj_set_style_bg_color(separator, ALMANAC_PAPER_BORDER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(separator, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(separator, 0, LV_PART_MAIN);

    /* ========== 宜 ========== */
    lv_obj_t *suitable_label = lv_label_create(panel);
    lv_label_set_text(suitable_label, s_suitable);
    lv_obj_set_style_text_font(suitable_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(suitable_label, ALMANAC_RED, LV_PART_MAIN);
    lv_label_set_long_mode(suitable_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(suitable_label, lv_pct(90));

    /* ========== 忌 ========== */
    lv_obj_t *avoid_label = lv_label_create(panel);
    lv_label_set_text(avoid_label, s_avoid);
    lv_obj_set_style_text_font(avoid_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(avoid_label, ALMANAC_TEXT_GRAY, LV_PART_MAIN);
    lv_label_set_long_mode(avoid_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(avoid_label, lv_pct(90));

    return panel;
}

/**
 * @brief       创建右侧天气面板
 * @param       parent: 父对象
 * @retval      面板对象指针
 */
static lv_obj_t* create_weather_panel(lv_obj_t *parent)
{
    /* 右侧面板 - 天气区域 */
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_width(panel, lv_pct(40));  /* 占40%宽度 */
    lv_obj_set_height(panel, lv_pct(100));
    lv_obj_set_style_radius(panel, THEME_RADIUS_LARGE, LV_PART_MAIN);

    /* 深色半透明背景 */
    lv_obj_set_style_bg_color(panel, THEME_CARD_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, THEME_OPA_SEMITRANS, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_border_opa(panel, LV_OPA_30, LV_PART_MAIN);

    /* 使用Flex布局，垂直排列居中 */
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(panel, THEME_PAD_LARGE, LV_PART_MAIN);
    lv_obj_set_style_pad_row(panel, 12, LV_PART_MAIN);

    /* ========== 天气图标（使用LVGL Symbol占位） ========== */
    /* 【注意】这里使用 Symbol 作为占位，未来可替换为自定义天气图片 */
    lv_obj_t *weather_icon_container = lv_obj_create(panel);
    lv_obj_set_size(weather_icon_container, 100, 100);
    lv_obj_set_style_radius(weather_icon_container, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(weather_icon_container, ALMANAC_GOLD, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(weather_icon_container, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(weather_icon_container, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(weather_icon_container, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(weather_icon_container, ALMANAC_GOLD, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(weather_icon_container, LV_OPA_50, LV_PART_MAIN);

    /* 太阳符号（晴天） */
    lv_obj_t *icon_label = lv_label_create(weather_icon_container);
    lv_label_set_text(icon_label, "日");  /* 暂用图片符号，未来替换 */
    lv_obj_set_style_text_font(icon_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(icon_label, THEME_TEXT_ON_DARK, LV_PART_MAIN);
    lv_obj_center(icon_label);

    /* ========== 温度（超大字） ========== */
    lv_obj_t *temp_label = lv_label_create(panel);
    lv_label_set_text(temp_label, s_weather_temp);
    lv_obj_set_style_text_font(temp_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, THEME_TEXT_PRIMARY, LV_PART_MAIN);

    /* ========== 天气描述 ========== */
    lv_obj_t *desc_label = lv_label_create(panel);
    lv_label_set_text(desc_label, s_weather_desc);
    lv_obj_set_style_text_font(desc_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(desc_label, THEME_TEXT_SECONDARY, LV_PART_MAIN);

    /* ========== 湿度 ========== */
    lv_obj_t *humidity_label = lv_label_create(panel);
    lv_label_set_text(humidity_label, s_weather_humidity);
    lv_obj_set_style_text_font(humidity_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(humidity_label, THEME_TEXT_HINT, LV_PART_MAIN);

    /* ========== 测试按钮区域 ========== */
    lv_obj_t *btn_container = lv_obj_create(panel);
    lv_obj_set_width(btn_container, lv_pct(100));
    lv_obj_set_height(btn_container, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(btn_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(btn_container, 8, LV_PART_MAIN);

    /* 模拟唤醒按钮 */
    lv_obj_t *wake_btn = lv_btn_create(btn_container);
    lv_obj_set_width(wake_btn, lv_pct(90));
    lv_obj_set_height(wake_btn, 40);
    ui_apply_button_light_style(wake_btn);
    lv_obj_t *wake_label = lv_label_create(wake_btn);
    lv_label_set_text(wake_label, "模拟唤醒词");
    lv_obj_set_style_text_font(wake_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(wake_label, THEME_TEXT_ON_DARK, LV_PART_MAIN);
    lv_obj_center(wake_label);
    lv_obj_add_event_cb(wake_btn, simulate_wake_btn_cb, LV_EVENT_CLICKED, NULL);

    /* 模拟手势按钮 */
    lv_obj_t *gesture_btn = lv_btn_create(btn_container);
    lv_obj_set_width(gesture_btn, lv_pct(90));
    lv_obj_set_height(gesture_btn, 40);
    ui_apply_button_dark_style(gesture_btn);
    lv_obj_t *gesture_label = lv_label_create(gesture_btn);
    lv_label_set_text(gesture_label, "模拟挥手");
    lv_obj_set_style_text_font(gesture_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(gesture_label, THEME_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_center(gesture_label);
    lv_obj_add_event_cb(gesture_btn, simulate_gesture_btn_cb, LV_EVENT_CLICKED, NULL);

    return panel;
}

/**
 * @brief       创建老黄历主界面
 * @param       无
 * @retval      屏幕对象指针
 */
lv_obj_t* create_home_screen(void)
{
    ESP_LOGI(TAG, "Creating almanac home screen...");

    /* 创建屏幕 */
    s_home_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_home_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_home_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_home_screen, THEME_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_home_screen, LV_OPA_COVER, LV_PART_MAIN);

    /* ========== 老黄历主容器 ========== */
    s_almanac_container = lv_obj_create(s_home_screen);
    lv_obj_set_width(s_almanac_container, lv_pct(100));
    /* 高度 = 屏幕高度 - 导航栏高度(65) - 顶部边距 */
    lv_obj_set_height(s_almanac_container, LV_VER_RES - 75);
    lv_obj_align(s_almanac_container, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_opa(s_almanac_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_almanac_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_almanac_container, THEME_PAD_MEDIUM, LV_PART_MAIN);

    /* 使用Flex布局，水平排列 */
    lv_obj_set_flex_flow(s_almanac_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_almanac_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(s_almanac_container, THEME_PAD_MEDIUM, LV_PART_MAIN);

    /* 创建左侧农历面板 */
    s_left_panel = create_lunar_panel(s_almanac_container);

    /* 创建右侧天气面板 */
    s_right_panel = create_weather_panel(s_almanac_container);

    /* 添加底部导航栏 */
    create_nav_bar(s_home_screen);

    ESP_LOGI(TAG, "Almanac home screen created successfully");
    return s_home_screen;
}

/**
 * @brief       删除主页
 * @param       无
 * @retval      无
 */
void delete_home_screen(void)
{
    if (s_home_screen) {
        lv_obj_del(s_home_screen);
        s_home_screen = NULL;
        s_almanac_container = NULL;
        s_left_panel = NULL;
        s_right_panel = NULL;
    }
}
