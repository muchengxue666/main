/**
 ******************************************************************************
 * @file        ui_theme.h
 * @version     V1.0
 * @brief       统一主题与样式定义 - 深色现代化设计
 ******************************************************************************
 * @attention   所有 UI 元素都遵循这套色系和样式规范
 ******************************************************************************
 */

#ifndef __UI_THEME_H__
#define __UI_THEME_H__

#include "lvgl.h"

/* ==================== 颜色定义 ==================== */
/* 背景色 - 深色模式 */
#define THEME_BG_COLOR          lv_color_make(13, 15, 20)      /* 深蓝灰 #0D0F14 */
#define THEME_BG_ALT            lv_color_make(25, 30, 40)      /* 稍亮深蓝灰 #191E28 */
#define THEME_CARD_BG           lv_color_make(35, 40, 50)      /* 卡片背景 #232832 */
#define THEME_CARD_LIGHT        lv_color_make(45, 52, 65)      /* 浅卡片背景 #2D3441 */

/* 主题强调色 - 科技蓝 */
#define THEME_PRIMARY           lv_color_make(0, 178, 255)     /* 科技蓝 #00B2FF */
#define THEME_PRIMARY_DARK      lv_color_make(0, 140, 210)     /* 深科技蓝 #008CD2 */
#define THEME_SECONDARY         lv_color_make(255, 193, 7)     /* 金色强调 #FFC107 */

/* 文字色 */
#define THEME_TEXT_PRIMARY      lv_color_white()               /* 主文本 - 白 */
#define THEME_TEXT_SECONDARY    lv_color_make(200, 200, 200)   /* 副文本 - 灰 #C8C8C8 */
#define THEME_TEXT_HINT         lv_color_make(150, 150, 150)   /* 提示文本 - 深灰 #969696 */

/* 功能色 */
#define THEME_SUCCESS           lv_color_make(76, 175, 80)     /* 成功绿 #4CAF50 */
#define THEME_WARNING           lv_color_make(255, 152, 0)     /* 警告橙 #FF9800 */
#define THEME_ERROR             lv_color_make(244, 67, 54)     /* 错误红 #F44336 */
#define THEME_INFO              lv_color_make(33, 150, 243)    /* 信息蓝 #2196F3 */

/* AI 对话相关 */
#define THEME_AI_MSG_BG         lv_color_make(0, 178, 255)     /* AI 消息背景 - 科技蓝 */
#define THEME_USER_MSG_BG       lv_color_make(76, 175, 80)     /* 用户消息背景 - 绿色 */

/* ==================== 尺寸定义 ==================== */
#define THEME_RADIUS_SMALL      4
#define THEME_RADIUS_MEDIUM     12
#define THEME_RADIUS_LARGE      20
#define THEME_RADIUS_CIRCLE     LV_RADIUS_CIRCLE

#define THEME_SHADOW_WIDTH      8
#define THEME_SHADOW_OPA        LV_OPA_30

#define THEME_PAD_SMALL         4
#define THEME_PAD_MEDIUM        8
#define THEME_PAD_LARGE         16
#define THEME_PAD_XLARGE        24

#define THEME_BORDER_WIDTH      0

/* ==================== 字体大小 ==================== */
#define THEME_FONT_TINY         10
#define THEME_FONT_SMALL        12
#define THEME_FONT_MEDIUM       14
#define THEME_FONT_LARGE        18
#define THEME_FONT_XLARGE       24
#define THEME_FONT_TITLE        32

/* ==================== 动画时长（毫秒）==================== */
#define THEME_ANIM_FAST         100
#define THEME_ANIM_NORMAL       300
#define THEME_ANIM_SLOW         500
#define THEME_ANIM_VERY_SLOW    800

/* ==================== 透明度 ==================== */
#define THEME_OPA_FULL          LV_OPA_100
#define THEME_OPA_SEMITRANS     LV_OPA_80
#define THEME_OPA_HALF          LV_OPA_50
#define THEME_OPA_LIGHT         LV_OPA_30
#define THEME_OPA_VERY_LIGHT    LV_OPA_20

/* ==================== 高级样式辅助函数 ==================== */

/**
 * @brief       应用卡片样式
 * @param       obj: 目标对象
 * @param       opa: 透明度（0-255）
 * @retval      无
 */
static inline void ui_apply_card_style(lv_obj_t *obj, lv_opa_t opa)
{
    lv_obj_set_style_radius(obj, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, THEME_CARD_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, opa, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, THEME_BORDER_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, THEME_SHADOW_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(obj, THEME_SHADOW_OPA, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, THEME_PAD_MEDIUM, LV_PART_MAIN);
}

/**
 * @brief       应用按钮样式（亮色）
 * @param       obj: 目标对象
 * @retval      无
 */
static inline void ui_apply_button_light_style(lv_obj_t *obj)
{
    lv_obj_set_style_radius(obj, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, THEME_OPA_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, THEME_BORDER_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, THEME_SHADOW_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(obj, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(obj, THEME_OPA_LIGHT, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN);
}

/**
 * @brief       应用按钮样式（深色）
 * @param       obj: 目标对象
 * @retval      无
 */
static inline void ui_apply_button_dark_style(lv_obj_t *obj)
{
    lv_obj_set_style_radius(obj, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, THEME_CARD_LIGHT, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, THEME_OPA_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, THEME_TEXT_PRIMARY, LV_PART_MAIN);
}

/**
 * @brief       应用标题文本样式
 * @param       obj: 目标对象
 * @retval      无
 */
static inline void ui_apply_title_style(lv_obj_t *obj)
{
    lv_obj_set_style_text_color(obj, THEME_TEXT_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN);
}

/**
 * @brief       应用正文文本样式
 * @param       obj: 目标对象
 * @retval      无
 */
static inline void ui_apply_body_style(lv_obj_t *obj)
{
    lv_obj_set_style_text_color(obj, THEME_TEXT_SECONDARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN);
}

#endif /* __UI_THEME_H__ */
