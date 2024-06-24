#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "9x5shield.h"
#include "chars.h"

static const char *TAG = "9x5shield";

//height of led screen
#define NUM_ROWS 5
//width of led screens
#define NUM_COLUMNS_PER_SCREEN  9
//arbitrary 'offscreen' buffer scalar. (can adjust)
#define NUM_SCREENS  28
//actual size of buffer (on&offscreen) to allocated
#define NUM_COLUMNS  (NUM_SCREENS * NUM_COLUMNS_PER_SCREEN)
//num usecs pause before scrolling columns left
#define CRAWL_SPEED 300000

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

//Assign NPN switch gates to GPIO#s on the esp
gpio_num_t column[NUM_COLUMNS_PER_SCREEN] =  {GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19};
gpio_num_t row[NUM_ROWS] = {  GPIO_NUM_2,  /*  top-left                                                                                                           */
                              GPIO_NUM_4,  /*                                                                                                                     */ 
                              GPIO_NUM_5,  /*                                                       middle                                                        */                                                          
                              GPIO_NUM_12, /*                                                                                                                     */ 
                              GPIO_NUM_13};/*                                                                                                         bottom-right*/

//render buffer
byte columnData[NUM_COLUMNS];

//handy reference to scrolling insert point
const byte * BUFFER_END = &columnData[NUM_COLUMNS-1];

//set up time for scrolling
unsigned short int seconds = 00;

//init values structure for timer to make
const esp_timer_create_args_t pov_timer_args = {
        .callback = &persistance_of_vision_callback,
        .name = "incremement_time"
};

const esp_timer_create_args_t scroll_timer_args = {
        .callback = &scroll_left_callback,
        .name = "scroll_left"
};
//timer created w/ esp_timer_create_args_t
esp_timer_handle_t pov_timer = NULL;
esp_timer_handle_t scroll_timer;

void persistance_of_vision_callback(void * arg){
  //draw data
  for (int r = 0; r < NUM_ROWS; r++) {
    gpio_set_level(row[r], 1);
    for (int c = 0; c < NUM_COLUMNS_PER_SCREEN; c++)
    {
      bool lit = (columnData[c] >> r) & 0x1;
      gpio_set_level(column[c], lit);
    }
    vTaskDelay(pdMS_TO_TICKS(3));
    gpio_set_level(row[r], 0);
  }
}

void scroll_left_callback(void * arg){
  memmove(columnData,&columnData[1],sizeof(columnData)-1);
  byte * last_byte = &columnData[sizeof(columnData)-1];
  if (++seconds & 0x1){
    //ESP_LOGI(TAG,"tick"); //remember, can't log (no time) w/ useful POV times 
    *last_byte = 0xAA;
  }else{
    //ESP_LOGI(TAG,"tock"); //remember, can't log (no time) w/ useful POV times 
    *last_byte = 0x55;
  }
}

void hw_init_led_gpios(){
  //ESP_LOGI(TAG,"hw initting LED GPIOS");
  for (int r = 0; r < NUM_ROWS; r++){
    gpio_reset_pin(row[r]);
    gpio_set_direction(row[r], GPIO_MODE_OUTPUT);
  }
  for (int c = 0; c < NUM_COLUMNS_PER_SCREEN; c++){
    gpio_reset_pin(column[c]);
    gpio_set_direction(column[c], GPIO_MODE_OUTPUT);
  }
}

void showMessage(byte* message, unsigned int length){
  //first time? get leds ready, start timer
  if (!pov_timer) {
    hw_init_led_gpios();        
    ESP_LOGI(TAG,"Creating Timers");
    ESP_ERROR_CHECK(esp_timer_create(&pov_timer_args, &pov_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(pov_timer, WAIT_TILL_REFRESH)); 
    ESP_ERROR_CHECK(esp_timer_create(&scroll_timer_args, &scroll_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(scroll_timer, CRAWL_SPEED));
  }

  //ESP_LOGI(TAG,"9x5shield Message: %s", message); //remember, can't log (no time) w/ useful POV times 

  //put in a couple of spaces before the start
  byte * start = addLetterToBuffer(addLetterToBuffer(columnData,' ',BUFFER_END),' ',BUFFER_END);
  for (int i = 0; i < length; i++) {
      //remember, can't log (no time) w/ useful POV times
      //ESP_LOGI(TAG,"add \"%c\" at address: 0x%X",(char)message[i],(unsigned int)start);
      start = addLetterToBuffer(start,(char)message[i],BUFFER_END);
  }
  //put a couple of space at the end
  start = addLetterToBuffer(start,' ',BUFFER_END);
  start = addLetterToBuffer(start,' ',BUFFER_END);

  return;
}

byte * addLetterToBuffer(byte * buffer, char letter, const byte * BUFFER_END){
  //In considering WORST case don't add anything that would cause overflow
  if (buffer + sizeof(W) + sizeof(KERN) > BUFFER_END) letter = 0;
  //Hash the symbol to it's bitpattern, add it to buffer
  switch (letter)
  {
    case(0):break;
    case('.'): memcpy(buffer, &DECIMAL, sizeof(DECIMAL));buffer+=sizeof(DECIMAL);break;
    case('/'): memcpy(buffer, &DIVIDE,sizeof(DIVIDE));buffer+=sizeof(DIVIDE);break;
    case('0'): memcpy(buffer, &ZERO,sizeof(ZERO));buffer+=sizeof(ZERO);break;
    case('1'): memcpy(buffer, &ONE,sizeof(ONE));buffer+=sizeof(ONE);break;
    case('2'): memcpy(buffer, &TWO, sizeof(TWO));buffer+=sizeof(TWO);break;
    case('3'): memcpy(buffer, &THREE, sizeof(THREE));buffer+=sizeof(THREE);break;
    case('4'): memcpy(buffer, &FOUR, sizeof(FOUR));buffer+=sizeof(FOUR);break;
    case('5'): memcpy(buffer, &FIVE, sizeof(FIVE));buffer+=sizeof(FIVE);break;
    case('6'): memcpy(buffer, &SIX, sizeof(SIX));buffer+=sizeof(SIX);break;
    case('7'): memcpy(buffer, &SEVEN, sizeof(SEVEN));buffer+=sizeof(SEVEN);break;
    case('8'): memcpy(buffer, &EIGHT, sizeof(EIGHT));buffer+=sizeof(EIGHT);break;
    case('9'): memcpy(buffer, &NINE, sizeof(NINE));buffer+=sizeof(NINE);break;
    case('A'): memcpy(buffer, &A, sizeof(A));buffer+=sizeof(A);break;
    case('B'): memcpy(buffer, &B, sizeof(B));buffer+=sizeof(B);break;
    case('C'): memcpy(buffer, &C, sizeof(C));buffer+=sizeof(C);break;
    case('D'): memcpy(buffer, &D, sizeof(D));buffer+=sizeof(D);break;
    case('E'): memcpy(buffer, &E, sizeof(E));buffer+=sizeof(E);break;
    case('F'): memcpy(buffer, &F, sizeof(F));buffer+=sizeof(F);break;
    case('G'): memcpy(buffer, &G, sizeof(G));buffer+=sizeof(G);break;
    case('H'): memcpy(buffer, &H, sizeof(H));buffer+=sizeof(H);break;
    case('I'): memcpy(buffer, &I, sizeof(I));buffer+=sizeof(I);break;
    case('J'): memcpy(buffer, &J, sizeof(J));buffer+=sizeof(J);break;
    case('K'): memcpy(buffer, &K, sizeof(K));buffer+=sizeof(K);break;
    case('L'): memcpy(buffer, &L, sizeof(L));buffer+=sizeof(L);break;
    case('M'): memcpy(buffer, &M, sizeof(M));buffer+=sizeof(M);break;
    case('N'): memcpy(buffer, &N, sizeof(N));buffer+=sizeof(N);break;
    case('O'): memcpy(buffer, &O, sizeof(O));buffer+=sizeof(O);break;
    case('P'): memcpy(buffer, &P, sizeof(P));buffer+=sizeof(P);break;
    case('Q'): memcpy(buffer, &Q, sizeof(Q));buffer+=sizeof(Q);break;
    case('R'): memcpy(buffer, &R, sizeof(R));buffer+=sizeof(R);break;
    case('S'): memcpy(buffer, &S, sizeof(S));buffer+=sizeof(S);break;
    case('T'): memcpy(buffer, &T, sizeof(T));buffer+=sizeof(T);break;
    case('U'): memcpy(buffer, &U, sizeof(U));buffer+=sizeof(U);break;
    case('V'): memcpy(buffer, &V, sizeof(V));buffer+=sizeof(V);break;
    case('W'): memcpy(buffer, &W, sizeof(W));buffer+=sizeof(W);break;
    case('X'): memcpy(buffer, &X, sizeof(X));buffer+=sizeof(X);break; 
    case('Y'): memcpy(buffer, &Y, sizeof(Y));buffer+=sizeof(Y);break;
    case('Z'): memcpy(buffer, &Z, sizeof(Z));buffer+=sizeof(Z);break; 
    case(','): memcpy(buffer, &COMMA, sizeof(COMMA));buffer+=sizeof(COMMA);break;
    case('-'): memcpy(buffer, &DASH, sizeof(DASH));buffer+=sizeof(DASH);break;
    case('_'): memcpy(buffer, &UNDERSCORE, sizeof(UNDERSCORE));buffer+=sizeof(UNDERSCORE);break;
    case(':'): memcpy(buffer, &COLON, sizeof(COLON));buffer+=sizeof(COLON);break;
    case(' '): memcpy(buffer, &SPACE, sizeof(SPACE));buffer+=sizeof(SPACE);break;
    case('@'): memcpy(buffer, &AT, sizeof(AT));buffer+=sizeof(AT);break;
    case('?'): memcpy(buffer, &QUESTION, sizeof(QUESTION));buffer+=sizeof(QUESTION);break;
    case(248): memcpy(buffer, &DEGREE, sizeof(DEGREE));buffer+=sizeof(DEGREE);break;
    default: memcpy(buffer, &NA, sizeof(NA));buffer+=sizeof(NA);break;
  }
  //add a column space in between letters for readability
  if (letter){
    memcpy(buffer, &KERN, sizeof(KERN));
    buffer += sizeof(KERN);
  }
  //byte has been incremented by the size of what was added, the new end.
  return buffer;
}