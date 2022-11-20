/*
 * ws2812b.h
 *
 *  Created on: Nov 17, 2022
 *      Author: matt
 */

#ifndef INC_WS2812B_H_
#define INC_WS2812B_H_

#include "stm32l4xx_hal.h"
#include "malloc.h"

#define WS2812B_MAX_PWM 25
#define WS2812B_MIN_PWM 15

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
}t_s_rgbaColor;

typedef struct {
	uint32_t ledId;
	uint32_t *next;
	uint32_t *prev;
	t_s_rgbaColor rgbaColor;
}t_s_ws2812b_led;

typedef struct {
	uint32_t *prev;
	uint32_t *next;
	TIM_HandleTypeDef *htim;
	uint32_t timerChannel;
	uint32_t id;
	uint32_t stripSize;
	t_s_ws2812b_led *firstLed;
}t_s_ws2812b_strip;

typedef struct {
	t_s_ws2812b_strip *entryPoint;
	t_s_ws2812b_strip *tailPoint;
}t_s_ws2812b_sequencer;

extern uint8_t uc_dataSentFlag;

void ws2812b_Init( void );
void ws2812b_createStrip( TIM_HandleTypeDef *htim, uint32_t timerChannel, uint32_t stripSize );
void ws2812b_createLeds( uint32_t stripId );
void ws2812b_resetLeds( uint32_t stripId );
t_s_ws2812b_led   * ws2812b_findLedById();
t_s_ws2812b_strip * ws2812b_findStripById( uint32_t stripId );
void ws2812b_generatePWMArray( uint32_t stripId, uint32_t *result );
void ws2812b_sendStrip( uint32_t stripId );

#endif /* INC_WS2812B_H_ */
