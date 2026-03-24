/**
 ******************************************************************************
 * @file        control_panel.c
 * @version     V2.0
 * @brief       硬件控制卡片 - 完整实现
 ******************************************************************************
 * @attention   亮度滑块 + 风扇开关 + 状态文本 + MQTT预留函数
 ******************************************************************************
 */

#include "control_panel.h"
#include "ai_assistant_ui.h"
#include "ui_theme.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"

static const char *TAG = "control_panel";

LV_FONT_DECLARE(myFont24);

/* 屏幕对象 */
static lv_obj_t *s_control_screen = NULL;

/* 亮度控制 */
static lv_obj_t *s_brightness_slider = NULL;
static lv_obj_t *s_brightness_label = NULL;
static lv_obj_t *s_brightness_value_label = NULL;

/* 风扇控制 */
static lv_obj_t *s_fan_switch = NULL;
static lv_obj_t *s_fan_label = NULL;
static lv_obj_t *s_fan_status_label = NULL;
static bool s_fan_state = false;

/* ========== 硬件控制的空函数接口（预留未来实现） ========== */

/**
 * @brief       设置屏幕亮度（未来接入硬件）
 * @param       brightness: 亮度值 0-100
 * @retval      无
 * @attention   该函数在未来应扩展为实际的硬件驱动调用
 *              例如：mqtt_publish_topic("device/brightness", brightness)
 */
static void hw_mock_set_brightness(uint8_t brightness)
{
    ESP_LOGI(TAG, "【预留函数】设置屏幕亮度: %d%%", brightness);
    /* 未来在此处发送 MQTT 指令到后端
     * 示例：mqtt_publish("home/device/lcd/brightness", brightness) */
}

/**
 * @brief       设置风扇状态（未来接入硬件）
 * @param       state: true=开启, false=关闭
 * @retval      无
 * @attention   该函数在未来应扩展为实际的硬件驱动调用
 *              例如：mqtt_publish_topic("device/fan", state ? "ON" : "OFF")
 */
static void hw_mock_set_fan(bool state)
{
    ESP_LOGI(TAG, "【预留函数】设置风扇状态: %s", state ? "ON" : "OFF");
    /* 未来在此处发送 MQTT 指令到后端
     * 示例：mqtt_publish("home/device/fan/control", state ? "ON" : "OFF") */
}



/* ========== 亮度滑块事件处理 ========== */

/**
 * @brief       亮度滑块变化回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void brightness_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int16_t brightness = lv_slider_get_value(slider);
    
    /* 更新标签 */
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%%", brightness);
    lv_label_set_text(s_brightness_value_label, buf);
    
    /* 调用硬件接口 */
    hw_mock_set_brightness(brightness);
    
    ESP_LOGI(TAG, "Brightness changed to: %d%%", brightness);
}

/* ========== 风扇开关事件处理 ========== */

/**
 * @brief       风扇开关变化回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void fan_switch_cb(lv_event_t *e)
{
    s_fan_state = lv_obj_has_state(s_fan_switch, LV_STATE_CHECKED);
    
    /* 更新状态文本 */
    lv_label_set_text(s_fan_status_label, s_fan_state ? "ON(运行中)" : "OFF(已关闭)");
    
    /* 更新文本颜色 */
    lv_color_t color = s_fan_state ? THEME_SUCCESS : THEME_ERROR;
    lv_obj_set_style_text_color(s_fan_status_label, color, LV_PART_MAIN);
    
    /* 调用硬件接口 */
    hw_mock_set_fan(s_fan_state);
    
    ESP_LOGI(TAG, "Fan switched to: %s", s_fan_state ? "ON" : "OFF");
}

/* ========== 导航栏回调函数 ========== */
static void nav_btn_standby_cb(lv_event_t *e)
{
    ui_switch_to_standby();
}

static void nav_btn_home_cb(lv_event_t *e)
{
    ui_switch_to_home();
}

static void nav_btn_viewer_cb(lv_event_t *e)
{
    ui_switch_to_viewer();
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
    
    /* 主页按钮 */
    lv_obj_t *btn_home = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_home, 70, 50);
    ui_apply_button_dark_style(btn_home);
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Home");
    lv_obj_set_style_text_font(label_home, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_home);
    lv_obj_add_event_cb(btn_home, nav_btn_home_cb, LV_EVENT_CLICKED, NULL);
    
    /* 控制面板按钮 - 高亮显示当前界面 */
    lv_obj_t *btn_control = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_control, 70, 50);
    ui_apply_button_light_style(btn_control);
    lv_obj_t *label_control = lv_label_create(btn_control);
    lv_label_set_text(label_control, "Panel");
    lv_obj_set_style_text_font(label_control, &myFont24, LV_PART_MAIN);
    lv_obj_center(label_control);
    
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

/* ========== UI 构建函数 ========== */

/**
 * @brief       创建亮度控制卡片
 * @param       parent: 父对象
 * @retval      卡片对象指针
 */
static lv_obj_t* create_brightness_card(lv_obj_t *parent)
{
    /* 卡片容器 */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_HOR_RES - THEME_PAD_LARGE * 2);
    lv_obj_set_height(card, 400);
    ui_apply_card_style(card, THEME_OPA_FULL);
    
    /* 卡片标题 */
    s_brightness_label = lv_label_create(card);
    lv_label_set_text(s_brightness_label, "屏幕亮度");
    lv_obj_align(s_brightness_label, LV_ALIGN_TOP_LEFT, THEME_PAD_MEDIUM, 20);
    ui_apply_title_style(s_brightness_label);
    lv_obj_set_style_text_font(s_brightness_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s_brightness_label, 10, LV_PART_MAIN);
    
    /* 亮度值显示 */
    s_brightness_value_label = lv_label_create(card);
    lv_label_set_text(s_brightness_value_label, "100%");
    lv_obj_align(s_brightness_value_label, LV_ALIGN_TOP_RIGHT, -THEME_PAD_MEDIUM, 20);
    lv_obj_set_style_text_color(s_brightness_value_label, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(s_brightness_value_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s_brightness_value_label, 10, LV_PART_MAIN);
    
    /* 亮度滑块 */
    s_brightness_slider = lv_slider_create(card);
    lv_slider_set_range(s_brightness_slider, 10, 100);
    lv_slider_set_value(s_brightness_slider, 100, LV_ANIM_OFF);
    
    /* 1. 修复尺寸问题：使用百分比宽度 (安全且适配好) */
    lv_obj_set_width(s_brightness_slider, lv_pct(85)); // 宽度占卡片的 85%
    lv_obj_set_height(s_brightness_slider, 60); 
    lv_obj_align(s_brightness_slider, LV_ALIGN_CENTER, 0, 30);
    
    /* 2. 轨道背景 (LV_PART_MAIN) - 设置为深灰色，衬托白色的滑块 */
    lv_obj_set_style_bg_color(s_brightness_slider, lv_color_hex(0x333333), LV_PART_MAIN); // 深灰色
    lv_obj_set_style_bg_opa(s_brightness_slider, LV_OPA_COVER, LV_PART_MAIN); 
    lv_obj_set_style_radius(s_brightness_slider, 30, LV_PART_MAIN); 
    
    /* 去除内边距，让填充区完全贴合 */
    lv_obj_set_style_pad_all(s_brightness_slider, 0, LV_PART_MAIN); 
    
    /* 3. 填充区 (LV_PART_INDICATOR) - 设置为纯白色 */
    lv_obj_set_style_bg_color(s_brightness_slider, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_brightness_slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(s_brightness_slider, 30, LV_PART_INDICATOR);
    
    /* 4. 拖动把手 (LV_PART_KNOB) - 隐藏圆点，全靠条本身滑动 */
    lv_obj_set_style_bg_opa(s_brightness_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(s_brightness_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    
    lv_obj_add_event_cb(s_brightness_slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    return card;
}

/**
 * @brief       创建风扇控制卡片
 * @param       parent: 父对象
 * @retval      卡片对象指针
 */
static lv_obj_t* create_fan_card(lv_obj_t *parent)
{
    /* 卡片容器 */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_HOR_RES - THEME_PAD_LARGE * 2);
    lv_obj_set_height(card, 250);
    ui_apply_card_style(card, THEME_OPA_FULL);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    // 卡片内的元素从上到下排列
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, 
                          LV_FLEX_ALIGN_CENTER);
    
    /* 卡片标题与开关行 (修复：宽度设为 100%) */
    lv_obj_t *title_cont = lv_obj_create(card);
    lv_obj_set_width(title_cont, lv_pct(100)); // <--- 关键修改：自动填满卡片可用宽度
    lv_obj_set_height(title_cont, 70);
    lv_obj_set_style_bg_opa(title_cont, 0, LV_PART_MAIN); // 透明背景
    lv_obj_set_style_border_width(title_cont, 0, LV_PART_MAIN); // 去除边框
    lv_obj_set_style_pad_all(title_cont, 0, LV_PART_MAIN); // 必须去除内边距，否则内部控件会偏移
    
    // Flex 布局：两端对齐 (SPACE_BETWEEN)
    lv_obj_set_flex_flow(title_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, 
                          LV_FLEX_ALIGN_CENTER);
    
    /* 标题文本 */
    s_fan_label = lv_label_create(title_cont);
    lv_label_set_text(s_fan_label, "排风扇");
    lv_obj_set_style_text_color(s_fan_label, lv_color_white(), LV_PART_MAIN); // 确保在深色背景可见
    lv_obj_set_style_text_font(s_fan_label, &myFont24, LV_PART_MAIN);
    
    /* 开关控件 */
    s_fan_switch = lv_switch_create(title_cont);
    lv_obj_set_width(s_fan_switch, 80);  /* 现代UI：稍宽一点的开关 */
    lv_obj_set_height(s_fan_switch, 40); 
    lv_obj_add_event_cb(s_fan_switch, fan_switch_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    /* 风扇状态展示区 (修复：宽度设为 100%) */
    lv_obj_t *status_cont = lv_obj_create(card);
    lv_obj_set_width(status_cont, lv_pct(100)); // <--- 关键修改：自动填满卡片可用宽度
    lv_obj_set_height(status_cont, 100);
    
    // 给状态区一点微弱的背景色，增加层次感
    lv_obj_set_style_bg_color(status_cont, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_cont, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(status_cont, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_cont, 0, LV_PART_MAIN); // 现代风通常不要边框，靠背景色区分
    
    // 状态区内部文本居中排列
    lv_obj_set_flex_flow(status_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(status_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, 
                          LV_FLEX_ALIGN_CENTER);
    
    /* 状态标题 */
    lv_obj_t *status_title = lv_label_create(status_cont);
    lv_label_set_text(status_title, "当前状态");
    lv_obj_set_style_text_color(status_title, lv_color_hex(0xAAAAAA), LV_PART_MAIN); // 浅灰色提示字
    lv_obj_set_style_text_font(status_title, &myFont24, LV_PART_MAIN);
    
    /* 状态文本 */
    s_fan_status_label = lv_label_create(status_cont);
    lv_label_set_text(s_fan_status_label, "OFF(已关闭)");
    // 没开的时候设为红色/灰色，打开的时候可以在 callback 里设为绿色
    lv_obj_set_style_text_color(s_fan_status_label, lv_color_hex(0xFF4C4C), LV_PART_MAIN); 
    lv_obj_set_style_text_font(s_fan_status_label, &myFont24, LV_PART_MAIN);
    
    return card;
}

/**
 * @brief       创建硬件控制卡片页面
 * @param       无
 * @retval      屏幕对象指针
 */
lv_obj_t* create_control_panel(void)
{
    ESP_LOGI(TAG, "Creating control panel...");
    
    /* 创建屏幕 */
    s_control_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_control_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_control_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_control_screen, THEME_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_control_screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_flex_flow(s_control_screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_control_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, 
                          LV_FLEX_ALIGN_CENTER);
    
    /* 顶部标题 */
    lv_obj_t *title = lv_label_create(s_control_screen);
    lv_label_set_text(title, "硬件控制");
    lv_obj_set_style_text_color(title, THEME_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_pad_top(title, THEME_PAD_LARGE, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(title, THEME_PAD_LARGE, LV_PART_MAIN);
    
    /* 创建亮度控制卡片 */
    create_brightness_card(s_control_screen);
    
    /* 创建风扇控制卡片 */
    create_fan_card(s_control_screen);
    
    /* 添加底部导航栏 */
    create_nav_bar(s_control_screen);
    
    ESP_LOGI(TAG, "Control panel created successfully");
    return s_control_screen;
}

/**
 * @brief       删除硬件控制卡片页面
 * @param       无
 * @retval      无
 */
void delete_control_panel(void)
{
    if (s_control_screen) {
        lv_obj_del(s_control_screen);
        s_control_screen = NULL;
    }
}
