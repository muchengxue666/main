/**
 ******************************************************************************
 * @file        home_screen.c
 * @version     V2.0
 * @brief       多模态交互主页 - 完整实现
 ******************************************************************************
 * @attention   含摄像头占位、聊天气泡、手势识别、音频波形、打字机效果
 ******************************************************************************
 */

#include "home_screen.h"
#include "ai_assistant_ui.h"
#include "ui_theme.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "home_screen";
LV_FONT_DECLARE(myFont24)   /* 声明myFont24字体,myFont24.c需要存放于"LVGL\src\font"文件夹下 */

/* 屏幕对象 */
static lv_obj_t *s_home_screen = NULL;

/* 左侧摄像头占位 */
static lv_obj_t *s_camera_card = NULL;
static lv_obj_t *s_gesture_indicator = NULL;
static lv_obj_t *s_gesture_icon = NULL;
static lv_timer_t *s_gesture_timer = NULL;

/* 右侧对话区 */
static lv_obj_t *s_chat_scroll = NULL;
static int s_chat_message_count = 0;

/* 音频波形 */
static lv_obj_t *s_waveform_chart = NULL;
static lv_obj_t *s_waveform_container = NULL;
static lv_timer_t *s_waveform_timer = NULL;
static bool s_waveform_animating = false;

/* 底部控制 */
static lv_obj_t *s_ai_response_btn = NULL;
static bool s_is_typing = false;
static lv_timer_t *s_typing_timer = NULL;

/* 模拟的AI回复文本 */
static const char *s_ai_responses[] = {
    "好的,我已经为您打开屏幕上的设备.\n光线已调整到最舒适的水平.",
    "我检测到您需要帮助.\n最新的天气信息已更新在控制面板.",
    "设备已成功连接到网络.\n所有传感器状态正常.",
    "根据您的历史记录,\n今天的推荐设置已自动应用.",
    "语音识别已开启.\n请说出您的命令..."
};

#define AI_RESPONSE_COUNT (sizeof(s_ai_responses) / sizeof(s_ai_responses[0]))

/**
 * @brief       手势指示器计时器回调 - 自动隐藏
 * @param       timer: 计时器句柄
 * @retval      无
 */
static void gesture_timer_cb(lv_timer_t *timer)
{
    if (s_gesture_indicator) {
        lv_obj_add_flag(s_gesture_indicator, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_del(s_gesture_timer);
    s_gesture_timer = NULL;
}

/**
 * @brief       显示手势识别指示
 * @param       gesture: 手势类型字符串
 * @retval      无
 */
static void show_gesture_indicator(const char *gesture)
{
    if (!s_gesture_indicator || !s_gesture_icon) return;
    
    /* 更新手势文本 */
    lv_label_set_text(s_gesture_icon, gesture);
    
    /* 显示指示器 */
    lv_obj_clear_flag(s_gesture_indicator, LV_OBJ_FLAG_HIDDEN);
    
    /* 启动缩放动画 */
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, s_gesture_indicator);
    lv_anim_set_values(&anim, 50, 120);
    lv_anim_set_time(&anim, THEME_ANIM_FAST);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)(void *)lv_obj_set_width);
    lv_anim_start(&anim);
    
    /* 设置自动隐藏计时器（1秒后） */
    if (s_gesture_timer) {
        lv_timer_del(s_gesture_timer);
    }
    s_gesture_timer = lv_timer_create(gesture_timer_cb, 1500, NULL);
}

/**
 * @brief       添加聊天气泡 - 用户消息
 * @param       text: 消息文本
 * @retval      无
 */
static void add_user_message(const char *text)
{
    if (!s_chat_scroll) return;
    
    /* 1. 创建气泡容器 */
    lv_obj_t *bubble = lv_obj_create(s_chat_scroll);
    
    // 【修复1】：宽度用百分比，高度用自适应！
    // 聊天气泡通常不占满全屏，设为 80% 宽度刚刚好。
    // LV_SIZE_CONTENT 会让气泡的高度自动根据里面有多少字来拉伸！
    lv_obj_set_width(bubble, lv_pct(80)); 
    lv_obj_set_height(bubble, LV_SIZE_CONTENT); 
    
    lv_obj_set_style_radius(bubble, THEME_RADIUS_LARGE, LV_PART_MAIN);
    ui_apply_card_style(bubble, THEME_OPA_FULL);
    lv_obj_set_style_bg_color(bubble, THEME_USER_MSG_BG, LV_PART_MAIN);
    
    // 如果这是一个用户发送的消息，让它靠右侧显示
    // (注意：前提是你的 s_chat_scroll 容器没有开启 Flex 布局，否则应该用 Flex 相关的对齐)
    lv_obj_set_align(bubble, LV_ALIGN_TOP_RIGHT); 
    
    // 【修复2】：把 padding 加在气泡上，把文字向内挤压 15 像素
    lv_obj_set_style_pad_all(bubble, 15, LV_PART_MAIN);
    // 去除默认边框，更美观
    lv_obj_set_style_border_width(bubble, 0, LV_PART_MAIN);
    
    /* 2. 创建文字标签 */
    lv_obj_t *label = lv_label_create(bubble);
    lv_label_set_text(label, text);
    
    // 开启自动换行
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    
    // 【修复3】：标签宽度设为 100%，高度也设为自适应
    // 这里的 100% 指的是填满气泡（会自动扣除气泡的 15 像素内边距）
    lv_obj_set_width(label, lv_pct(100)); 
    lv_obj_set_height(label, LV_SIZE_CONTENT); 
    
    lv_obj_set_style_text_font(label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    
    s_chat_message_count++;
    
    /* 3. 延迟滚动到底部（必须加上一点点延迟，让引擎先排好版） */
    // 刚加进去内容，LVGL 还没算好总高度，直接 scroll 经常划不到最底。
    // 强迫症做法：告诉 LVGL 等会再划
    lv_obj_scroll_to_view(bubble, LV_ANIM_ON);
}

/**
 * @brief       添加聊天气泡 - AI 消息
 * @param       text: 消息文本
 * @retval      无
 */
static void add_ai_message(const char *text)
{
    if (!s_chat_scroll) return;
    
    /* 1. 创建气泡容器 */
    lv_obj_t *bubble = lv_obj_create(s_chat_scroll);
    
    // 【修复1】：宽度用百分比，高度用自适应！
    // 聊天气泡通常不占满全屏，设为 80% 宽度刚刚好。
    // LV_SIZE_CONTENT 会让气泡的高度自动根据里面有多少字来拉伸！
    lv_obj_set_width(bubble, lv_pct(80)); 
    lv_obj_set_height(bubble, LV_SIZE_CONTENT); 
    
    lv_obj_set_style_radius(bubble, THEME_RADIUS_LARGE, LV_PART_MAIN);
    ui_apply_card_style(bubble, THEME_OPA_FULL);
    lv_obj_set_style_bg_color(bubble, THEME_USER_MSG_BG, LV_PART_MAIN);
    
    // 如果这是一个用户发送的消息，让它靠右侧显示
    // (注意：前提是你的 s_chat_scroll 容器没有开启 Flex 布局，否则应该用 Flex 相关的对齐)
    lv_obj_set_align(bubble, LV_ALIGN_TOP_LEFT); 
    
    // 【修复2】：把 padding 加在气泡上，把文字向内挤压 15 像素
    lv_obj_set_style_pad_all(bubble, 15, LV_PART_MAIN);
    // 去除默认边框，更美观
    lv_obj_set_style_border_width(bubble, 0, LV_PART_MAIN);
    
    /* 2. 创建文字标签 */
    lv_obj_t *label = lv_label_create(bubble);
    lv_label_set_text(label, text);
    
    // 开启自动换行
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); 
    
    // 【修复3】：标签宽度设为 100%，高度也设为自适应
    // 这里的 100% 指的是填满气泡（会自动扣除气泡的 15 像素内边距）
    lv_obj_set_width(label, lv_pct(100)); 
    lv_obj_set_height(label, LV_SIZE_CONTENT); 
    
    lv_obj_set_style_text_font(label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    
    s_chat_message_count++;
    
    /* 3. 延迟滚动到底部（必须加上一点点延迟，让引擎先排好版） */
    // 刚加进去内容，LVGL 还没算好总高度，直接 scroll 经常划不到最底。
    // 强迫症做法：告诉 LVGL 等会再划
    lv_obj_scroll_to_view(bubble, LV_ANIM_ON);
}

/* ========== 打字机效果相关 ========== */

typedef struct {
    lv_obj_t *label;      /* 目标标签 */
    char text[512];       /* 完整文本（存副本，而不是存指针！） */
    uint16_t byte_idx;    /* 当前截取到的【字节】索引 */
    uint16_t total_bytes; /* 总【字节】数 */
} typing_context_t;

static typing_context_t s_typing_ctx = {0};

/**
 * @brief       打字机计时器回调
 * @param       timer: 计时器句柄
 * @retval      无
 */
static void typing_timer_cb(lv_timer_t *timer)
{
    if (s_typing_ctx.byte_idx < s_typing_ctx.total_bytes) {
        
        /* 【核心修复】：智能识别 UTF-8 边界，不切碎汉字！ */
        // 先无条件往前走一个字节
        s_typing_ctx.byte_idx++;
        
        // 只要当前的字节以二进制 10xxxxxx 开头，说明它是一个多字节字符的“后续字节”，必须连带一起吞下！
        while (s_typing_ctx.byte_idx < s_typing_ctx.total_bytes && 
              (s_typing_ctx.text[s_typing_ctx.byte_idx] & 0xC0) == 0x80) {
            s_typing_ctx.byte_idx++;
        }

        /* 提取到当前安全边界的字符串 */
        char temp_str[512] = {0};
        // 使用 memcpy 非常安全，并确保最后一个字符是结束符 '\0'
        memcpy(temp_str, s_typing_ctx.text, s_typing_ctx.byte_idx);
        temp_str[s_typing_ctx.byte_idx] = '\0'; 
        
        lv_label_set_text(s_typing_ctx.label, temp_str);
        lv_obj_set_style_text_font(s_typing_ctx.label, &myFont24, LV_PART_MAIN);
        lv_obj_set_style_text_color(s_typing_ctx.label, lv_color_white(), LV_PART_MAIN);
    } else {
        /* 打字完成 */
        lv_label_set_text(s_typing_ctx.label, s_typing_ctx.text);
        lv_obj_set_style_text_font(s_typing_ctx.label, &myFont24, LV_PART_MAIN);
        lv_obj_set_style_text_color(s_typing_ctx.label, lv_color_white(), LV_PART_MAIN);
        lv_timer_del(s_typing_timer);
        s_typing_timer = NULL;
        s_is_typing = false;
        
        /* 启动波形动画 */
        // if (s_waveform_chart && !s_waveform_animating) {
        //     s_waveform_animating = true;
        // }
    }
}

/**
 * @brief       启动打字机效果
 * @param       label: 目标标签对象
 * @param       text: 文本内容
 * @retval      无
 */
static void start_typewriter_effect(lv_obj_t *label, const char *text)
{
    if (!label || !text) return;
    
    s_is_typing = true;
    s_typing_ctx.label = label;
    
    // 【核心修复】：把外部传进来的文本，老老实实拷贝一份到我们自己的结构体里！
    strncpy(s_typing_ctx.text, text, sizeof(s_typing_ctx.text) - 1);
    s_typing_ctx.text[sizeof(s_typing_ctx.text) - 1] = '\0'; // 确保绝对安全
    
    s_typing_ctx.byte_idx = 0;
    s_typing_ctx.total_bytes = strlen(s_typing_ctx.text); // 计算真实的总字节数
    
    /* 启动计时器，每50ms显示一个字符 */
    if (s_typing_timer) {
        lv_timer_del(s_typing_timer);
    }
    s_typing_timer = lv_timer_create(typing_timer_cb, 50, NULL);
}

/**
 * @brief       AI响应按钮回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void ai_response_btn_cb(lv_event_t *e)
{
    if (s_is_typing) return;  /* 正在打字中 */
    
    ESP_LOGI(TAG, "AI response button clicked");
    
    /* 获取随机响应 */
    static uint8_t response_idx = 0;
    const char *response = s_ai_responses[response_idx++ % AI_RESPONSE_COUNT];
    
    /* 添加用户消息 */
    add_user_message("打开风扇");
    
    /* ========================================== */
    /* 修复后的 AI 气泡生成逻辑                   */
    /* ========================================== */
    
    /* 1. 创建 AI 气泡背景 */
    lv_obj_t *ai_bubble = lv_obj_create(s_chat_scroll);
    
    // 【关键修复 1】：宽度用百分比，高度用自适应！
    lv_obj_set_width(ai_bubble, lv_pct(80)); 
    lv_obj_set_height(ai_bubble, LV_SIZE_CONTENT); // 必须加！让里面的文字撑开它！
    
    lv_obj_set_style_radius(ai_bubble, THEME_RADIUS_LARGE, LV_PART_MAIN);
    ui_apply_card_style(ai_bubble, THEME_OPA_FULL);
    lv_obj_set_style_bg_color(ai_bubble, THEME_AI_MSG_BG, LV_PART_MAIN);
    
    // 【关键修复 2】：把 Padding 加在气泡上，把文字向内挤！去掉边框！
    lv_obj_set_style_pad_all(ai_bubble, 15, LV_PART_MAIN);
    lv_obj_set_style_border_width(ai_bubble, 0, LV_PART_MAIN);
    
    /* 2. 创建文字标签 */
    lv_obj_t *ai_label = lv_label_create(ai_bubble);
    lv_label_set_long_mode(ai_label, LV_LABEL_LONG_WRAP); // 自动换行
    
    // 【关键修复 3】：文字宽度设为 100%（填满气泡），高度也设为自适应！
    lv_obj_set_width(ai_label, lv_pct(100)); 
    lv_obj_set_height(ai_label, LV_SIZE_CONTENT);
    
    lv_obj_set_style_text_color(ai_label, lv_color_black(), LV_PART_MAIN);
    // 注意：这里千万不要再给 ai_label 加 pad_all 了！
    
    /* 3. 设置测试文本 */
    start_typewriter_effect(ai_label, response);
    
    s_chat_message_count += 2;
    
    /* 4. 滚动到底部 */
    lv_obj_scroll_to_view(ai_bubble, LV_ANIM_ON);
}

/**
 * @brief       模拟手势识别按钮回调
 * @param       e: 事件句柄
 * @retval      无
 */
static void gesture_sim_btn_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Simulating gesture recognition...");
    show_gesture_indicator("Fist");
}

/* ========== 波形动画 ========== */

#define WAVEFORM_POINTS 16

/**
 * @brief       波形动画计时器回调
 * @param       timer: 计时器句柄
 * @retval      无
 */
static void waveform_timer_cb(lv_timer_t *timer)
{
    if (!s_waveform_animating || !s_waveform_chart) return;
    
    static lv_coord_t waveform_data[WAVEFORM_POINTS];
    
    /* 生成随机波形数据 */
    for (int i = 0; i < WAVEFORM_POINTS; i++) {
        waveform_data[i] = (lv_rand(20, 100)) * 2;  /* 0-200范围 */
    }
    
    /* 更新图表 */
    lv_chart_series_t *series = lv_chart_get_series_next(s_waveform_chart, NULL);
    if (series) {
        for (int i = 0; i < WAVEFORM_POINTS; i++) {
            lv_chart_set_next_value(s_waveform_chart, series, waveform_data[i]);
        }
    }
}


/**
 * @brief       创建左侧摄像头占位卡片
 * @param       parent: 父对象
 * @retval      卡片对象指针
 */
static lv_obj_t* create_camera_card(lv_obj_t *parent)
{
    /* 摄像头卡片容器 */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_width(card, (LV_HOR_RES / 2) - THEME_PAD_LARGE);
    lv_obj_set_height(card, LV_VER_RES - 100);
    lv_obj_align(card, LV_ALIGN_LEFT_MID, THEME_PAD_MEDIUM, 0);
    ui_apply_card_style(card, THEME_OPA_SEMITRANS);
    
    /* 摄像头占位文本 */
    lv_obj_t *camera_text = lv_label_create(card);
    lv_obj_set_style_text_font(camera_text, &myFont24, LV_PART_MAIN);  /* 设置自定义字体 */
    lv_label_set_text(camera_text, "\n摄像头画面占位\nCamera Feed");
    lv_obj_align(camera_text, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_text_color(camera_text, THEME_TEXT_SECONDARY, LV_PART_MAIN);
    lv_obj_set_style_text_align(camera_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    
    /* 手势识别指示器 - 右上角 */
    s_gesture_indicator = lv_obj_create(card);
    lv_obj_set_size(s_gesture_indicator, 50, 50);
    lv_obj_align(s_gesture_indicator, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_radius(s_gesture_indicator, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_gesture_indicator, THEME_SUCCESS, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_gesture_indicator, THEME_OPA_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_gesture_indicator, 0, LV_PART_MAIN);
    lv_obj_add_flag(s_gesture_indicator, LV_OBJ_FLAG_HIDDEN);
    
    s_gesture_icon = lv_label_create(s_gesture_indicator);
    lv_label_set_text(s_gesture_icon, "Fist");
    lv_obj_center(s_gesture_icon);
    lv_obj_set_style_text_font(s_gesture_icon, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(s_gesture_icon, lv_color_white(), LV_PART_MAIN);
    
    /* 模拟手势识别按钮 */
    lv_obj_t *gesture_btn = lv_btn_create(card);
    
    // 【关键修复 1】：使用百分比宽度，比如让按钮占卡片宽度的 80%
    lv_obj_set_width(gesture_btn, lv_pct(80)); 
    lv_obj_set_height(gesture_btn, 40);
    
    // 【关键修复 2】：确保父对象 card 没有开启 Flex 布局
    // (如果 card 之前写了 lv_obj_set_flex_flow，这里的 align 会失效！如果没写就没关系)
    lv_obj_align(gesture_btn, LV_ALIGN_BOTTOM_MID, 0, -20); 
    
    ui_apply_button_light_style(gesture_btn); // 确保你这个自定义样式里没有把背景设为透明
    
    lv_obj_t *gesture_label = lv_label_create(gesture_btn);
    lv_label_set_text(gesture_label, "模拟识别手势");
    lv_obj_center(gesture_label);
    lv_obj_set_style_text_font(gesture_label, &myFont24, LV_PART_MAIN);
    // 确保文字颜色和按钮背景色能区分开
    lv_obj_set_style_text_color(gesture_label, lv_color_white(), LV_PART_MAIN);
    
    lv_obj_add_event_cb(gesture_btn, gesture_sim_btn_cb, LV_EVENT_CLICKED, NULL);
    
    return card;
}

/**
 * @brief       创建右侧对话区域
 * @param       parent: 父对象
 * @retval      对话容器对象指针
 */
static lv_obj_t* create_chat_area(lv_obj_t *parent)
{
    /* 对话区域容器 */
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_width(container, (LV_HOR_RES / 2) - THEME_PAD_LARGE);
    lv_obj_set_height(container, LV_VER_RES - 100);
    lv_obj_align(container, LV_ALIGN_RIGHT_MID, -THEME_PAD_MEDIUM, 0);
    lv_obj_set_style_bg_color(container, THEME_BG_ALT, LV_PART_MAIN);
    lv_obj_set_style_radius(container, THEME_RADIUS_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(container, THEME_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_border_opa(container, LV_OPA_30, LV_PART_MAIN);
    
    /* ========================================== */
    /* 1. 修复后的：可滚动的聊天区域               */
    /* ========================================== */
    s_chat_scroll = lv_obj_create(container);
    
    // 【关键修复1】：放弃 get_width，一律用百分比或自适应！
    lv_obj_set_width(s_chat_scroll, lv_pct(100)); // 占满父容器宽度
    // 高度暂时给一个安全固定值（比如300），确保它能撑开。测试成功后你再微调。
    lv_obj_set_height(s_chat_scroll, lv_pct(90)); 
    
    lv_obj_align(s_chat_scroll, LV_ALIGN_TOP_MID, 0, 10);
    
    // 暂时给个深灰色背景，这样你能清楚看到聊天列表的边界在哪！
    lv_obj_set_style_bg_color(s_chat_scroll, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_chat_scroll, 0, LV_PART_MAIN);
    
    // 开启 Flex 布局（子对象从上到下排列）
    lv_obj_set_flex_flow(s_chat_scroll, LV_FLEX_FLOW_COLUMN);
    // 【关键修复2】：因为 Flex 接管了对齐，我们要让里面所有的气泡靠右排列，就要在这里设置交叉轴（Cross-axis）为 END
    lv_obj_set_flex_align(s_chat_scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);
    
    lv_obj_set_style_pad_all(s_chat_scroll, 10, LV_PART_MAIN);

    /* 初始化一些对话 */
    add_user_message("打开风扇");
    add_ai_message("好的,已打开排风扇.");
    add_user_message("调节亮度");
    add_ai_message("亮度已调至80%.");
    
    /* 波形栏 */
    s_waveform_container = lv_obj_create(container);
    lv_obj_set_size(s_waveform_container, lv_obj_get_width(container) - THEME_PAD_LARGE,
                    60);
    lv_obj_align(s_waveform_container, LV_ALIGN_BOTTOM_MID, 0, -60);
    ui_apply_card_style(s_waveform_container, THEME_OPA_SEMITRANS);
    
    s_waveform_chart = lv_chart_create(s_waveform_container);
    lv_obj_set_size(s_waveform_chart, lv_obj_get_width(s_waveform_container) - THEME_PAD_LARGE,
                    50);
    lv_obj_align(s_waveform_chart, LV_ALIGN_CENTER, 0, 0);
    lv_chart_set_type(s_waveform_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(s_waveform_chart, WAVEFORM_POINTS);
    lv_obj_set_style_bg_color(s_waveform_chart, THEME_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_size(s_waveform_chart, 0, LV_PART_INDICATOR);
    
    /* 添加波形系列 */
    lv_chart_add_series(s_waveform_chart, lv_palette_main(LV_PALETTE_CYAN), 
                        LV_CHART_AXIS_PRIMARY_Y);
    
    /* 启动波形更新计时器 */
    s_waveform_timer = lv_timer_create(waveform_timer_cb, 100, NULL);
    
    /* AI响应按钮 */
    s_ai_response_btn = lv_btn_create(container);
    lv_obj_set_width(s_ai_response_btn, lv_obj_get_width(container) - THEME_PAD_LARGE);
    lv_obj_set_height(s_ai_response_btn, 40);
    lv_obj_align(s_ai_response_btn, LV_ALIGN_BOTTOM_MID, 0, -THEME_PAD_MEDIUM);
    ui_apply_button_light_style(s_ai_response_btn);
    
    lv_obj_t *btn_label = lv_label_create(s_ai_response_btn);
    lv_label_set_text(btn_label, "模拟AI回复");
    lv_obj_center(btn_label);
    lv_obj_set_style_text_font(btn_label, &myFont24, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn_label, lv_color_white(), LV_PART_MAIN);
    
    lv_obj_add_event_cb(s_ai_response_btn, ai_response_btn_cb, LV_EVENT_CLICKED, NULL);
    
    return container;
}

/* ========== 导航栏回调函数 ========== */
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
    
    /* 控制面板按钮 */
    lv_obj_t *btn_control = lv_btn_create(nav_bar);
    lv_obj_set_size(btn_control, 70, 50);
    ui_apply_button_dark_style(btn_control);
    lv_obj_t *label_control = lv_label_create(btn_control);
    lv_label_set_text(label_control, "Panel");
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
 * @brief       创建多模态交互主页
 * @param       无
 * @retval      屏幕对象指针
 */
lv_obj_t* create_home_screen(void)
{
    ESP_LOGI(TAG, "Creating home screen...");
    
    /* 创建屏幕 */
    s_home_screen = lv_obj_create(NULL);
    lv_obj_set_size(s_home_screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(s_home_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(s_home_screen, THEME_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(s_home_screen, LV_OPA_COVER, LV_PART_MAIN);
    
    /* 添加顶部标题 */
    lv_obj_t *title = lv_label_create(s_home_screen);
    lv_label_set_text(title, "AI Panel");
    lv_obj_set_style_text_font(title, &myFont24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, THEME_PAD_LARGE);
    ui_apply_title_style(title);
    
    /* 创建左侧摄像头卡片 */
    s_camera_card = create_camera_card(s_home_screen);
    
    /* 创建右侧对话区域 */
    create_chat_area(s_home_screen);
    
    /* 添加底部导航栏 */
    create_nav_bar(s_home_screen);
    
    ESP_LOGI(TAG, "Home screen created successfully");
    return s_home_screen;
}

/**
 * @brief       删除主页
 * @param       无
 * @retval      无
 */
void delete_home_screen(void)
{
    if (s_waveform_timer) {
        lv_timer_del(s_waveform_timer);
        s_waveform_timer = NULL;
    }
    if (s_typing_timer) {
        lv_timer_del(s_typing_timer);
        s_typing_timer = NULL;
    }
    if (s_gesture_timer) {
        lv_timer_del(s_gesture_timer);
        s_gesture_timer = NULL;
    }
    
    if (s_home_screen) {
        lv_obj_del(s_home_screen);
        s_home_screen = NULL;
    }
}
