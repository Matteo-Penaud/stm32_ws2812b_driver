/*
 * ws2812b.c
 *
 *  Created on: Nov 17, 2022
 *      Author: matt
 */

#include "ws2812b.h"

uint8_t uc_dataSentFlag = 0;

t_s_ws2812b_sequencer mainSequencer =
	{ .entryPoint = NULL, .tailPoint = NULL };

void ws2812b_createStrip( TIM_HandleTypeDef *htim, uint32_t timerChannel, uint32_t stripSize )
{
	static uint32_t stripId;
	t_s_ws2812b_strip *cursor = NULL;

	if(mainSequencer.entryPoint == NULL)
	{
		stripId = 0;
		mainSequencer.entryPoint = (t_s_ws2812b_strip *)malloc(sizeof(t_s_ws2812b_strip));
		mainSequencer.tailPoint = mainSequencer.entryPoint;

		mainSequencer.entryPoint->next = NULL;
		mainSequencer.entryPoint->prev = NULL;

		cursor = mainSequencer.entryPoint;
	}
	else
	{
		cursor = mainSequencer.tailPoint;

		cursor->next = (uint32_t *)malloc(sizeof(t_s_ws2812b_strip));
		cursor = (t_s_ws2812b_strip *)cursor->next;
		cursor->next = (uint32_t *)mainSequencer.entryPoint;
		cursor->prev = (uint32_t *)mainSequencer.tailPoint;
		mainSequencer.tailPoint = cursor;
	}

	cursor->id = stripId;
	cursor->htim = htim;
	cursor->timerChannel = timerChannel;
	cursor->stripSize = stripSize;

	ws2812b_createLeds(stripId);
	ws2812b_sendStrip(stripId);

	stripId++;
}

void ws2812b_createLeds( uint32_t stripId )
{
	t_s_ws2812b_strip	*stripCursor = ws2812b_findStripById(stripId);
	t_s_ws2812b_led 	*ledCursor = NULL;

	uint32_t ledIndex = 0;

	while(ledIndex < stripCursor->stripSize)
	{
		if(ledIndex == 0)
		{
			stripCursor->firstLed = (t_s_ws2812b_led *)malloc(sizeof(t_s_ws2812b_led));
			ledCursor = stripCursor->firstLed;

			ledCursor->next = (uint32_t *)ledCursor;
			ledCursor->prev = (uint32_t *)ledCursor;
		}
		else
		{
			ledCursor = (t_s_ws2812b_led *)stripCursor->firstLed->prev;

			ledCursor->next = (uint32_t *)malloc(sizeof(t_s_ws2812b_led));
			ledCursor = (t_s_ws2812b_led *)ledCursor->next;
			ledCursor->next = (uint32_t *)stripCursor->firstLed;
			ledCursor->prev = (uint32_t *)stripCursor->firstLed->prev;
			stripCursor->firstLed->prev = (uint32_t *)ledCursor;
		}

		ledCursor->ledId = ledIndex;
		ledCursor->rgbaColor.red = 30;
		ledCursor->rgbaColor.green = 0;
		ledCursor->rgbaColor.blue = 0;
		ledCursor->rgbaColor.alpha = 0;

		ledIndex++;
	}
}

void ws2812b_resetLeds( uint32_t stripId )
{
	t_s_ws2812b_strip	*stripCursor = ws2812b_findStripById(stripId);
	t_s_ws2812b_led 	*ledCursor = stripCursor->firstLed;

	while(ledCursor->ledId < stripCursor->stripSize)
	{
		ledCursor->rgbaColor.red = 0;
		ledCursor->rgbaColor.green = 0;
		ledCursor->rgbaColor.blue = 0;
		ledCursor->rgbaColor.alpha = 0;

		ledCursor = (t_s_ws2812b_led *)ledCursor->next;
	}
}

t_s_ws2812b_strip * ws2812b_findStripById( uint32_t stripId )
{
	t_s_ws2812b_strip *cursor;

	if((mainSequencer.tailPoint->id / 2) < stripId)
	{
		cursor = mainSequencer.tailPoint;

		while(cursor->id != stripId)
		{
			cursor = (t_s_ws2812b_strip *)cursor->prev;
		}
	}
	else
	{
		cursor = mainSequencer.entryPoint;

		while(cursor->id != stripId)
		{
			cursor = (t_s_ws2812b_strip *)cursor->next;
		}
	}

	return cursor;
}

void ws2812b_generatePWMArray( uint32_t stripId, uint32_t *result )
{
	t_s_ws2812b_strip strip = *ws2812b_findStripById(stripId);
	t_s_ws2812b_led *cursor = strip.firstLed;

	for(uint32_t led = 0; led < strip.stripSize; led++)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			if(cursor->rgbaColor.green & (1 << (7-i)))
				result[(24*led)+i] = WS2812B_MAX_PWM;
			else
				result[(24*led)+i] = WS2812B_MIN_PWM;
		}

		for(uint8_t i = 0; i < 8; i++)
		{
			if(cursor->rgbaColor.red & (1 << (7-i)))
				result[(24*led)+i+8] = WS2812B_MAX_PWM;
			else
				result[(24*led)+i+8] = WS2812B_MIN_PWM;
		}

		for(uint8_t i = 0; i < 8; i++)
		{
			if(cursor->rgbaColor.blue & (1 << (7-i)))
				result[(24*led)+i+16] = WS2812B_MAX_PWM;
			else
				result[(24*led)+i+16] = WS2812B_MIN_PWM;
		}
	}

	for(uint32_t i=0; i < 50; i++)
		result[i+(24*strip.stripSize)] = 0;
}

void ws2812b_sendStrip( uint32_t stripId )
{
	t_s_ws2812b_strip *stripCursor = ws2812b_findStripById(stripId);
	uint32_t stripToSend[(24*stripCursor->stripSize)+50];

	ws2812b_generatePWMArray(stripId, stripToSend);

	HAL_TIM_PWM_Start_DMA(stripCursor->htim, stripCursor->timerChannel, (uint32_t *)&stripToSend, (24*stripCursor->stripSize)+50);
}
