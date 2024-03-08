#include <stdint.h>
#include "msp.h"
#include "..\inc\bump.h"
#include "..\inc\Reflectance.h"
#include "..\inc\Clock.h"
//#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\FlashProgram.h"
#include "..\inc\Motorsimple.h"



// Linked data structure
struct State {
  uint32_t port2out;                // 2-bit output
  uint32_t duty;
  uint32_t delay;              // time to delay in 1ms
  const struct State *next[4]; // Next if 2-bit input is 0-3 (4 inputs)
};
typedef const struct State State_t;

#define center    &fsm[0]
#define left      &fsm[1]   //drive right
#define right     &fsm[2]
#define lostmiddle &fsm[3]
#define lostright &fsm[4]
#define lostleft &fsm[5]



#define DARK      0x00
#define RED       0x01
#define GREEN     0x02
#define BLUE      0x04
#define WHITE     0x07
#define YELLOW    0x03
#define SKYBLUE   0x06
#define PINK      0x05

//RIGHT NOW THE DUTY CYCLES ARE CHANGED SO THAT THE ROBOT CAN HANDLE TIGHT TURNS
State_t fsm[6]={
  {GREEN, 3800, 1, {lostmiddle,left,right,center}},  //center
  {RED, 5000, 1, {lostleft,left,right,center}},  //left
  {BLUE, 5000, 1, {lostright,left,right,center}}, //right


  //Lost States
  {WHITE, 6000, 1, {lostmiddle,left,right,center}}, //lost middle
  {PINK, 6000, 1, {lostright,left,right,center}}, //lostright
  {SKYBLUE, 6000, 1, {lostleft,left,right,center}} //lostleft


};

uint32_t Input;
uint32_t Output;
/*Run FSM continuously
1) Output depends on State (LaunchPad LED)
2) Wait depends on State
3) Input (LaunchPad buttons)
4) Next depends on (Input,State)
 */

void WaitForResetPress(void) {
    while (P1->IN & 0x02) {
        // Loop until the button is pressed
        // If the button is active low, this waits until the bit is cleared
    }
}

void Port2_Init(){
    P2->SEL0 = 0x00;
    P2->SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
    P2->DS = 0x07;                          // make P2.2-P2.0 high drive strength
    P2->DIR = 0x07;                         // make P2.2-P2.0 out
    P2->OUT = 0x00;                         // all LEDs off
}


uint8_t reflectancein,bumpread;
void SysTick_Handler(void){ // every 1ms
  // write this as part of Lab 10

    bumpread=Bump_Read();

}

void ResetButton_Init(void) {
    P1->SEL0 &= ~0x02;   // Configure P1.1 as GPIO
    P1->SEL1 &= ~0x02;
    P1->DIR &= ~0x02;    // P1.1 set as input
    P1->REN |= 0x02;     // Enable pull-up/pull-down resistors
    P1->OUT |= 0x02;     // Select pull-up mode
}
uint8_t reflectancein;
int main(void){
    Clock_Init48MHz();
    LaunchPad_Init();
    SysTick_Init();
    //SysTick_Init(); set up interrupts
    Port2_Init();
    Motor_InitSimple();
    SysTick_Init(480000,2);  // set up SysTick for 100 Hz interrupts
    EnableInterrupts();
    Reflectance_Init(); //initialize pins 5.3,9.2,7.0-7
    Bump_Init();
    WaitForResetPress();

    State_t *pt;
    uint8_t leftBits, rightBits, centerBits;
    pt=center; //initial state

    while(1){
        P2->OUT=pt->port2out;
        Clock_Delay1ms(pt->delay);   // wait
            // next depends on input and state

        if(pt==center){
            Motor_ForwardSimple(pt->duty, pt->delay);
        }
        else if(pt==left){
            Motor_StopForTime(1);
            Motor_RightSimple(pt->duty, pt->delay);
        }
        else if (pt == right){
            Motor_StopForTime(1);
           Motor_LeftSimple(pt->duty, pt->delay);
        }
        else if (pt == lostright){
            Motor_StopForTime(5);
           Motor_LeftSimple(pt->duty, pt->delay);
        }
        else if (pt == lostleft){
            Motor_StopForTime(5);
            Motor_RightSimple(pt->duty, pt->delay);
        }
        else{
            Motor_RightSimple(pt->duty, pt->delay);
            Motor_ForwardSimple(pt->duty, pt->delay);
        }

        reflectancein=Reflectance_Read(1000);

        leftBits = ((reflectancein & 0xC0) > 0);
        rightBits = ((reflectancein & 0x03) > 0);
        centerBits = ((reflectancein & 0x3C) > 0);
        if((leftBits>0)&&(rightBits>0)){
            pt=pt->next[2];
        }
        else if((leftBits > 0) || (rightBits > 0))
        {
            pt=pt->next[leftBits*2 + rightBits];
        }
        else if(centerBits > 0){
            pt = pt->next[3];
        }
        else{
            pt=pt->next[leftBits*2 + rightBits];
        }


        if (bumpread != 0){
                    Motor_StopSimple();
                    break;
                }



  }
}
