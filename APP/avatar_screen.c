/**
 ******************************************************************************
 * @file        avatar_screen.c
 * @version     V1.0
 * @brief       小柚子化身界面 - 实现文件
 ******************************************************************************
 * @attention   唤醒后显示的可爱拟人化形象"小柚子"
 *              - 黑色背景上的圆形占位图
 *              - 上下浮动动画
 *              - 点击或 3 秒后自动跳转到 home_screen
 *              - 预留 lv_rlottie 接口用于未来 Lottie 动画
 ******************************************************************************
 */

#include "avatar_screen.h"
#include "ai_assistant_ui.h"
#include "ui_theme.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"

static const char *TAG = "avatar_screen";
LV_FONT_DECLARE(myFont24)   /* 声明自定义字体 */

/* ==================== 屏幕和控件对象 ==================== */
static lv_obj_t *s_avatar_screen = NULL;
static lv_obj_t *s_avatar_container = NULL;     /* 化身容器（用于整体动画） */
static lv_obj_t *s_avatar_circle = NULL;        /* 化身圆形占位 */
static lv_obj_t *s_avatar_face = NULL;          /* 化身表情/图片占位 */
static lv_obj_t *s_greeting_label = NULL;       /* 问候语标签 */
static lv_obj_t *s_hint_label = NULL;           /* 提示标签 */

/* ==================== 定时器和动画 ==================== */
static lv_timer_t *s_auto_transition_timer = NULL;  /* 自动跳转定时器 */
static bool s_avatar_animating = false;

/* ==================== 化身尺寸定义 ==================== */
#define AVATAR_CIRCLE_SIZE      180
#define AVATAR_INNER_SIZE       160
#define FLOAT_AMPLITUDE         15      /* 浮动振幅（像素） */
#define FLOAT_DURATION          1500    /* 浮动周期（毫秒） */
#define AUTO_TRANSITION_MS      5000    /* 自动跳转时间（毫秒） */

/* 小柚子配色 */
#define AVATAR_GOLD             lv_color_hex(0xCFAF47)  /* 金色（柚子色） */

/* ==================== 问候语数组 ==================== */
static const char *s_greetings[] = {
    "你好呀!我是小柚子~",
    "嘿嘿,又见面啦!",
    "今天心情怎么样？",
    "我在这里陪着你!",
    "有什么想聊的吗？"
};
#define GREETING_COUNT (sizeof(s_greetings) / sizeof(s_greetings[0]))

/**
 * @brief       浮动动画回调 - Y轴位移
 * @param       obj: 目标对象
 * @param       v: 当前动画值
 * @retval      无
 */
static void float_anim_cb(void *obj, int32_t v)
{
    if (s_avatar_container) {
        /* 中心位置 + 浮动偏移 */
        lv_obj_align(s_avatar_container, LV_ALIGN_CENTER, 0, v - FLOAT_AMPLITUDE);
    }
}

/**
 * @brief       呼吸光晕动画回调 - 透明度
 * @param       obj: 目标对象
 * @param       v: 当前动画值 (0-255)
 * @retval      无
 */
static void glow_anim_cb(void *obj, int32_t v)
{
    if (s_avatar_circle) {
        lv_obj_set_style_shadow_opa(s_avatar_circle, v, LV_PART_MAIN);
    }
}

/**
 * @brief       启动化身动画
 * @param       无
 * @retval      无
 */
static void start_avatar_animations(void)
{
    if (s_avatar_animating) return;
    s_avatar_animating = true;

    /* ========== 上下浮动动画 ========== */
    lv_anim_t float_anim;
    lv_anim_init(&float_anim);
    lv_anim_set_var(&float_anim, s_avatar_container);
    lv_anim_set_exec_cb(&float_anim, float_anim_cb);
    lv_anim_set_values(&float_anim, 0, FLOAT_AMPLITUDE * 2);
    lv_anim_set_time(&float_anim, FLOAT_DURATION);
    lv_anim_set_playback_time(&float_anim, FLOAT_DURATION);
    lv_anim_set_repeat_count(&float_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&float_anim, lv_anim_path_ease_in_out);
    lv_anim_start(&float_anim);

    /* ========== 外圈呼吸光晕动画 ========== */
    lv_anim_t glow_anim;
    lv_anim_init(&glow_anim);
    lv_anim_set_var(&glow_anim, s_avatar_circle);
    lv_anim_set_exec_cb(&glow_anim, glow_anim_cb);
    lv_anim_set_values(&glow_anim, LV_OPA_30, LV_OPA_80);
    lv_anim_set_time(&glow_anim, 1200);
    lv_anim_set_playback_time(&glow_anim, 1200);
    lv_anim_set_repeat_count(&glow_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&glow_anim);
}

/**
 * @brief       自动跳转定时器回调
 * @param       timer: 定时器句柄
 * @retval      无
 */
static void auto_transition_timer_cb(lv_timer_t *timer)
{
    ESP_LOGI(TAG, "Auto transition to home screen");
    ui_switch_to_home();
}

/**
 * @brief       屏幕点击事件回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void avatar_click_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Avatar clicked, switching to home screen");

    /* 点击时立即跳转，取消自动跳转定时器 */
    if (s_auto_transition_timer) {
        lv_timer_del(s_auto_transition_timer);
        s_auto_transition_timer = NULL;
    }

    ui_switch_to_home();
}

/**
 * @brief       创建小柚子化身界面
 * @param       无
 * @retval      屏幕对象指针
 */
lv_obj_t* create_avatar_screen(void)
{
    ESP_LOGI(TAG, "Creating avatar screen...");

    /* 创建屏幕对象 */
    s_avatar_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_avatar_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_avatar_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* 移除所有继承的样式，设置纯黑背景 */
    lv_obj_remove_style_all(s_avatar_screen);
    lv_obj_set_style_bg_color(s_avatar_screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_avatar_screen, LV_OPA_COVER, LV_PART_MAIN);

    /* 添加点击事件 */
    lv_obj_add_event_cb(s_avatar_screen, avatar_click_cb, LV_EVENT_CLICKED, NULL);

    /* ========== 化身容器（用于整体浮动动画） ========== */
    s_avatar_container = lv_obj_create(s_avatar_screen);
    lv_obj_set_size(s_avatar_container, AVATAR_CIRCLE_SIZE + 60, AVATAR_CIRCLE_SIZE + 150);
    lv_obj_align(s_avatar_container, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_bg_opa(s_avatar_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_avatar_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(s_avatar_container, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_avatar_container, LV_OBJ_FLAG_SCROLLABLE);

    /* 使用Flex布局，垂直排列居中 */
    lv_obj_set_flex_flow(s_avatar_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(s_avatar_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(s_avatar_container, 15, LV_PART_MAIN);

    /* ========== 外圈光晕圆 ========== */
    s_avatar_circle = lv_obj_create(s_avatar_container);
    lv_obj_set_size(s_avatar_circle, AVATAR_CIRCLE_SIZE, AVATAR_CIRCLE_SIZE);
    lv_obj_set_style_radius(s_avatar_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_avatar_circle, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_avatar_circle, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_avatar_circle, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_avatar_circle, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_border_opa(s_avatar_circle, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s_avatar_circle, 30, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(s_avatar_circle, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(s_avatar_circle, LV_OPA_50, LV_PART_MAIN);
    lv_obj_clear_flag(s_avatar_circle, LV_OBJ_FLAG_SCROLLABLE);

    /* ========== 内圈（表情/图片占位） ========== */
    /* 【预留接口】未来可替换为 lv_img_create + Lottie 动画 */
    s_avatar_face = lv_obj_create(s_avatar_circle);
    lv_obj_set_size(s_avatar_face, AVATAR_INNER_SIZE, AVATAR_INNER_SIZE);
    lv_obj_center(s_avatar_face);
    lv_obj_set_style_radius(s_avatar_face, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_avatar_face, AVATAR_GOLD, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_avatar_face, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_avatar_face, 0, LV_PART_MAIN);
    lv_obj_clear_flag(s_avatar_face, LV_OBJ_FLAG_SCROLLABLE);

    /* 简单的表情符号作为占位 */
    lv_obj_t *face_label = lv_label_create(s_avatar_face);
    lv_label_set_text(face_label, "^_^");  /* 简单笑脸占位 */
    lv_obj_set_style_text_font(face_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(face_label, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_center(face_label);

    /* 【Lottie 预留接口】
     * 未来接入方式:
     * lv_obj_t *lottie = lv_rlottie_create_from_file(s_avatar_circle, width, height, "path/to/animation.json");
     * lv_obj_center(lottie);
     */

    /* ========== 问候语 ========== */
    s_greeting_label = lv_label_create(s_avatar_container);
    /* 随机选择一条问候语 */
    uint32_t idx = lv_rand(0, GREETING_COUNT - 1);
    lv_label_set_text(s_greeting_label, s_greetings[idx]);
    lv_obj_set_style_text_font(s_greeting_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_greeting_label, THEME_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_greeting_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    /* ========== 底部提示 ========== */
    s_hint_label = lv_label_create(s_avatar_screen);
    lv_label_set_text(s_hint_label, "点击屏幕继续");
    lv_obj_align(s_hint_label, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_text_font(s_hint_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_hint_label, THEME_TEXT_HINT, LV_PART_MAIN);

    /* ========== 启动动画 ========== */
    start_avatar_animations();

    /* ========== 设置自动跳转定时器 ========== */
    s_auto_transition_timer = lv_timer_create(auto_transition_timer_cb, AUTO_TRANSITION_MS, NULL);
    lv_timer_set_repeat_count(s_auto_transition_timer, 1);  /* 只执行一次 */

    ESP_LOGI(TAG, "Avatar screen created successfully");
    return s_avatar_screen;
}

/**
 * @brief       删除小柚子化身界面
 * @param       无
 * @retval      无
 */
void delete_avatar_screen(void)
{
    /* 先停止动画和定时器 */
    stop_avatar_animations();

    if (s_avatar_screen) {
        lv_obj_del(s_avatar_screen);
        s_avatar_screen = NULL;
        s_avatar_container = NULL;
        s_avatar_circle = NULL;
        s_avatar_face = NULL;
        s_greeting_label = NULL;
        s_hint_label = NULL;
    }
}

/**
 * @brief       停止化身动画
 * @param       无
 * @retval      无
 */
void stop_avatar_animations(void)
{
    /* 停止浮动动画 */
    lv_anim_del(s_avatar_container, float_anim_cb);
    /* 停止光晕动画 */
    lv_anim_del(s_avatar_circle, glow_anim_cb);

    /* 停止自动跳转定时器 */
    if (s_auto_transition_timer) {
        lv_timer_del(s_auto_transition_timer);
        s_auto_transition_timer = NULL;
    }

    s_avatar_animating = false;
}

/**
 * @brief       重新启动化身动画
 * @param       无
 * @retval      无
 */
void restart_avatar_animations(void)
{
    stop_avatar_animations();
    start_avatar_animations();

    /* 重新设置自动跳转定时器 */
    s_auto_transition_timer = lv_timer_create(auto_transition_timer_cb, AUTO_TRANSITION_MS, NULL);
    lv_timer_set_repeat_count(s_auto_transition_timer, 1);

    ESP_LOGI(TAG, "Avatar animations restarted");
}
