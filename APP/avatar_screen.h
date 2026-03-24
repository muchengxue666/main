/**
 ******************************************************************************
 * @file        avatar_screen.h
 * @version     V1.0
 * @brief       小柚子化身界面 - 头文件
 ******************************************************************************
 * @attention   唤醒后显示的可爱拟人化形象"小柚子"
 *              预留 Lottie 动画接口，暂用静态占位 + 浮动动画
 ******************************************************************************
 */

#ifndef __AVATAR_SCREEN_H
#define __AVATAR_SCREEN_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief       创建小柚子化身界面
 * @param       无
 * @retval      屏幕对象指针
 */
lv_obj_t* create_avatar_screen(void);

/**
 * @brief       删除小柚子化身界面
 * @param       无
 * @retval      无
 */
void delete_avatar_screen(void);

/**
 * @brief       停止化身动画（切换前调用）
 * @param       无
 * @retval      无
 */
void stop_avatar_animations(void);

/**
 * @brief       重新启动化身动画
 * @param       无
 * @retval      无
 */
void restart_avatar_animations(void);

#ifdef __cplusplus
}
#endif

#endif /* __AVATAR_SCREEN_H */
