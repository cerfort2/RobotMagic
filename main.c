#include <stdint.h>
#include "msp.h"
#include "../inc/clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/Texas.h"


// Linked data structure
struct State {
  uint32_t port2out;                // 2-bit output
  uint32_t delay;              // time to delay in 1ms
  const struct State *next[3]; // Next if 2-bit input is 0-3 (4 inputs)
};

typedef const struct State State_t;
#define center    &fsm[0]
#define left      &fsm[1]   //drive right
#define right     &fsm[2]   //drive left

#define DARK      0x0
#define RED       0x01      //S5=error
#define GREEN     0x02      //S4=purchase
#define BLUE      0x04      //s3=nickel
#define WHITE     0x07      //s1=zero
#define YELLOW    0x03
#define SKYBLUE   0x06
#define PINK      0x05      //s2=dime

State_t fsm[3]={
  {WHITE, 3000, {center,left,right}},  // center
  {RED, 3000, {center,left,right}},  // left
  {BLUE, 3000, {center,left,right}},   // right
};

uint32_t Input;
uint32_t Output;
/*Run FSM continuously
1) Output depends on State (LaunchPad LED)
2) Wait depends on State
3) Input (LaunchPad buttons)
4) Next depends on (Input,State)
 */
void Port2_Init(){
    P2->SEL0 = 0x00;
    P2->SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
    P2->DS = 0x07;                          // make P2.2-P2.0 high drive strength
    P2->DIR = 0x07;                         // make P2.2-P2.0 out
    P2->OUT = 0x00;                         // all LEDs off
}


int main(void){
  Clock_Init48MHz();
  LaunchPad_Init();
  SysTick_Init();
  Clock_Init48MHz();
  Port2_Init();

  State_t *pt; 
  pt=center;          //initial state

  while(1){
    P2->OUT=pt->port2out;
    Clock_Delay1ms(pt->delay);   // wait

    input1 = ((P1->IN>>1)&~0xFE)^0x1;  //read input
    input2 = ((P1->IN>>4)^0xF)&~0xE;
    input = (input2 <<1) | input1;

    pt = pt->next[input];       // next depends on input and state

  }
}

// Color    LED(s) Port2
// dark     ---    0
// red      R--    0x01
// blue     --B    0x04
// green    -G-    0x02
// yellow   RG-    0x03
// sky blue -GB    0x06
// white    RGB    0x07
// pink     R-B    0x05
