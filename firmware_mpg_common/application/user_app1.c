/**********************************************************************************************************************
File: user_app1.c                                                                

Description:
Provides a Tera-Term driven system to display, read and write an LED command list.

Test1.
Test2 from Engenuics.
Test3.

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:
None.

Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern u8 G_au8DebugScanfBuffer[DEBUG_SCANF_BUFFER_SIZE]; /* From debug.c */
extern u8 G_u8DebugScanfCharCount;                        /* From debug.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions 
*/


void UserApp1_state1()
{
  static bool bstate1OneTimeSetted=FALSE;
  if(bstate1OneTimeSetted==FALSE)
  {
    /*Close the Function of state2*/
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);
    LedOff(LCD_RED);
    LedOff(LCD_GREEN);
    LedOff(LCD_BLUE);
    /*set the function of state1*/
    LedPWM(LCD_RED,LED_PWM_35);
    LedPWM(LCD_GREEN,LED_PWM_20);
    LedPWM(LCD_BLUE,LED_PWM_45);
    DebugPrintf("Entering state 1\n\r");
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(8,"State 1");
    LedOn(WHITE);
    LedOn(PURPLE);
    LedOn(BLUE);
    LedOn(CYAN);
    PWMAudioOff(BUZZER1);
    bstate1OneTimeSetted=TRUE;
  }
  /*Waitting for the command  which shift 1 to 2*/
  if((WasButtonPressed(BUTTON2))||(G_au8DebugScanfBuffer[G_u8DebugScanfCharCount-1]=='\r'&&
                                  G_au8DebugScanfBuffer[G_u8DebugScanfCharCount-2]=='2'))
  {
    ButtonAcknowledge(BUTTON2);
    for(u16 i=0;i<=(G_u8DebugScanfCharCount-1);i++)
    {
      G_au8DebugScanfBuffer[i]='\0';
    }
    bstate1OneTimeSetted=FALSE;
    UserApp1_StateMachine=UserApp1_state2;
  }
   
}/*end UserApp1_state1*/


void UserApp1_state2(void)
{
  static bool bstate2OneTimeSetted=FALSE;
  static u16 u16TimeCounter=0;
  if(bstate2OneTimeSetted==FALSE) 
  {
    /*Close the Function of state2*/
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(LCD_RED);
    LedOff(LCD_GREEN);
    LedOff(LCD_BLUE);
    /*set the function of state2*/
    PWMAudioSetFrequency(BUZZER1,200);
    PWMAudioOn(BUZZER1);
    DebugPrintf("Entering state 2\n\r");
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(8,"State 2");
    LedBlink(GREEN,LED_1HZ);
    LedBlink(YELLOW,LED_2HZ);
    LedBlink(ORANGE,LED_4HZ);
    LedBlink(RED,LED_8HZ);
    LedPWM(LCD_RED,LED_PWM_55);
    LedPWM(LCD_GREEN,LED_PWM_30);
    LedPWM(LCD_BLUE,LED_PWM_15);
    bstate2OneTimeSetted=TRUE;
  }
    u16TimeCounter++;
    if(u16TimeCounter==100)
    {
      PWMAudioOff(BUZZER1);
    }
    if(u16TimeCounter==1000)
    {
      u16TimeCounter=0;
      PWMAudioOn(BUZZER1);
    }
  if((WasButtonPressed(BUTTON1))||(G_au8DebugScanfBuffer[G_u8DebugScanfCharCount-1]=='\r'&&
                                  G_au8DebugScanfBuffer[G_u8DebugScanfCharCount-2]=='1'))
  {
    ButtonAcknowledge(BUTTON1);
    for(u16 i=0;i<=(G_u8DebugScanfCharCount-1);i++)
    {
      G_au8DebugScanfBuffer[i]='\0';
    }
    bstate2OneTimeSetted=FALSE;
    UserApp1_StateMachine=UserApp1_state1;
    
  }
    
}/*end UserApp1_state2*/

   

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp1Initialize(void)
{
  u8 au8UserApp1Start1[] = "LED program task started\n\r";
  
  /* Turn off the Debug task command processor and announce the task is ready */
  DebugSetPassthrough();
  DebugPrintf(au8UserApp1Start1);
  LedOff(WHITE); 
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  LedOff(LCD_RED);
  LedOff(LCD_GREEN);
  LedOff(LCD_BLUE);
  PWMAudioOff(BUZZER1);
    /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_FailedInit;
  }

} /* end UserApp1Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for input */
static void UserApp1SM_Idle(void)
{
  /*set the initial state as state1 */
  UserApp1_StateMachine=UserApp1_state1; 
} /* end UserApp1SM_Idle() */
                      
            
#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp1SM_FailedInit(void)          
{
    
} /* end UserApp1SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
