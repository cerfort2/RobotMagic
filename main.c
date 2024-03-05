#include <stdint.h>
#include "../inc/Texas.h"

#include "msp.h"
#include "..\inc\bump.h"
#include "..\inc\Reflectance.h"
#include "..\inc\Clock.h"
//#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\FlashProgram.h"
#include "..\inc\Motorsimple.h"

uint8_t reflectancein;

#define DARK      0x0
#define RED       0x01      //S5=error
#define GREEN     0x02      //S4=purchase
#define BLUE      0x04      //s3=nickel
#define WHITE     0x07      //s1=zero
#define YELLOW    0x03
#define SKYBLUE   0x06
#define PINK      0x05      //s2=dime

struct State {
  uint8_t port2out;                // 2-bit output
  uint32_t duty; // time to delay in 1ms
  uint8_t function;
};



//1=FORWARD
//2=BACKWARD
//3=RIGHT
//4=LEFT
//ELSE=STOP
typedef const struct State State_t;
State_t fsm[21]={
  {DARK, 4000, 5},
  {DARK, 4000, 5},
  {RED, 4000, 3},
  {BLUE, 4000, 4},
  {PINK, 4000, 4},
  {GREEN, 4000, 1},
  {SKYBLUE, 4000, 3},
  {PINK, 4000, 4},
  {SKYBLUE, 4000, 3},
  {BLUE, 4000, 4},
  {BLUE, 4000, 5},
  {BLUE, 4000, 3},
  {BLUE, 4000, 4},
  {BLUE, 4000, 3},
  {BLUE, 4000, 5},
  {BLUE, 4000, 5},
  {BLUE, 4000, 5},  //states 16-20 are sums that could possibly be reached. still need to implement
  {BLUE, 4000, 5},
  {BLUE, 4000, 5},
  {BLUE, 4000, 5},
  {BLUE, 4000, 5}
};



#define lost    &fsm[0] //lost
#define non1      &fsm[1] //nothing
#define veryLeft     &fsm[2] //
#define veryRight &fsm[3]
#define middleRight     &fsm[4] //
#define middle    &fsm[5] //
#define middleLeft    &fsm[6] //
#define kindaRight    &fsm[7] //
#define kindaLeft    &fsm[8] //
#define kindaRightMiddle &fsm[9] //
#define non2     &fsm[10] //
#define kindaLeftMiddle     &fsm[11] //
#define bigRight     &fsm[12] //
#define bigLeft     &fsm[13] //
#define non3     &fsm[14] //
#define bigMiddle     &fsm[15] //
#define big16 &fsm[16]
#define big17 &fsm[17]
#define big18 &fsm[18]
#define big19 &fsm[19]
#define big20 &fsm[20]

State_t* next[21] = {lost,non1,veryLeft,veryRight,middleRight,middle,middleLeft,kindaRight,kindaLeft,kindaRightMiddle,non2,kindaLeftMiddle,bigRight,bigLeft,non3,bigMiddle,big16,big17,big18,big19,big20};




uint8_t vl,ml,mid,mr,vr,index,bumpread;
void SysTick_Handler(void){ // every 1ms
  // write this as part of Lab 10


    vl = ((reflectancein & 0x03) > 0);
    ml = ((reflectancein & 0x0c) > 0);
    mid= ((reflectancein & 0x18) > 0);
    mr = ((reflectancein & 0x30) > 0);
    vr = ((reflectancein & 0xc0) > 0);

    index=vl*2 + ml*6 + mid*5 + mr*4 + vr*3;

    bumpread=Bump_Read();

}



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


int32_t position=0;

int main(void){
  Clock_Init48MHz();
  //SysTick_Init(); set up interrupts
    Port2_Init();
    Motor_InitSimple();
    SysTick_Init(480000,2);  // set up SysTick for 100 Hz interrupts
    EnableInterrupts();
    Reflectance_Init(); //initialize pins 5.3,9.2,7.0-7
    Bump_Init();

  State_t *pt;
  pt=middle;          //initial state

  while(1){

    P2->OUT=pt->port2out;
    Clock_Delay1ms(1);   // wait


    reflectancein=Reflectance_Read(1000);
    position=Reflectance_Position(reflectancein);





    if(bumpread>0)
        Motor_StopSimple();

    switch(pt->function){
    case 1:
        Motor_ForwardSimple(pt->duty,1);
    break;
    case 2:
        Motor_BackwardSimple(pt->duty,1);
        break;
    case 3:
        Motor_RightSimple(pt->duty,1);
        break;
    case 4:
        Motor_LeftSimple(pt->duty,1);
        break;
    default:
        Motor_StopSimple();}



    pt=next[index];




  }
}
