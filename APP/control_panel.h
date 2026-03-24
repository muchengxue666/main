/**
 ******************************************************************************
 * @file        control_panel.h
 * @version     V1.0
 * @brief       硬件控制抽屉 - 头文件
 ******************************************************************************
 * @attention   包含屏幕亮度滑块和排风扇开关控件
 ******************************************************************************
 */

#ifndef __CONTROL_PANEL_H
#define __CONTROL_PANEL_H

#include "lvgl.h"
#include "ai_assistant_ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 函数声明 */
lv_obj_t* create_control_panel(void);
void delete_control_panel(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_PANEL_H */