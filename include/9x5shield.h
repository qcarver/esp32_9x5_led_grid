#ifndef __9X5SHIELD_H__
#define __9X5SHIELD_H__

#include "string.h"
#include "esp_timer.h"
typedef unsigned char byte;

//in microseconds
#define ANIMATION_SPEED 100000

//NOTE: not a lot of time between POV refreshes 
#if defined CONFIG_ESP_TIMER_ISR_AFFINITY_CPU1 == 1
//you can have 10 more microseconds if the timer isr is pegged to core 1
    //#pragma message ( "running timer isr on core 1, which will make display a little brighter :)" )
    #define WAIT_TILL_REFRESH 98
#else
//irregardless, if you use LOGI or something intense watchdog will crash us. So +++increase if debugging w/ logs
    #warning "idf.py menuconfig's config sdk output has timer isr serviced on default core"
    #define WAIT_TILL_REFRESH 108
#endif

#define NUM_ROWS 5
#define NUM_SCREENS  28
#define NUM_COLUMNS_PER_SCREEN  9
#define NUM_COLUMNS  (NUM_SCREENS * NUM_COLUMNS_PER_SCREEN)

void persistance_of_vision_callback(void* arg);
void scroll_left_callback(void * arg);
byte * addLetterToBuffer(byte * buffer, char letter, const byte * BUFFER_END);
void showMessage(byte* message, unsigned int length);

#endif /*__9X5SHIELD_H__*/