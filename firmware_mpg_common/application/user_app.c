/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

#ifdef MPG2
extern PixelBlockType G_sLcdClearLine7;                /* From lcd_NHD-C12864LZ.c */
#endif /* MPG2 */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  u8 au8SongTitle[] = "Heart and Soul";
  AT91C_BASE_PIOB->PIO_PER|=0x00000008;
  AT91C_BASE_PIOB->PIO_OER|=0x00000008;
  AT91C_BASE_PIOA->PIO_PER|=0x00000800;
  AT91C_BASE_PIOA->PIO_OER|=0x00000800;
#ifdef MPGL1
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8SongTitle);
#endif /* MPGL1 */

#ifdef MPG2
  PixelAddressType sStringLocation; 
  sStringLocation.u16PixelColumnAddress = LCD_CENTER_COLUMN - ( strlen((char const*)au8SongTitle) * (LCD_SMALL_FONT_COLUMNS + LCD_SMALL_FONT_SPACE) / 2 );
  sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE7;
  
  LcdClearPixels(&G_sLcdClearLine7);
  LcdLoadString(au8SongTitle, LCD_FONT_SMALL, &sStringLocation);
#endif /* MPG 2 */

  /* If good initialization, set state to Idle */
  if( 1 /* Add condition for good init */)
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/* LED mapping for right hand:
D5  WHITE
E5  PURPLE
F5  BLUE
G5  CYAN
A5  GREEN
A5S YELLOW
C6  ORANGE
D6  RED

LED mapping for left: 
A3S  PURPLE  
C4   BLUE
D4   CYAN
E4   GREEN
F4   YELLOW
A4   RED
OFF  WHITE
*/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
  static u16 u16TimeCounter=0;
  static bool bLedisOn1=FALSE;
  static bool bLedisOn2=FALSE;
  static bool bBuzzerisOn=FALSE;
   /******************************************************************************
  Description:Numbered musical notation
  musical name:Happy birthday to you!
  */
  u16 u16noteBuzzer1[]={NO,C4,C4,D4,C4,F4,E4,C4,C4,D4,C4,G5,F5,C4,C4,C3,A5,F5,E4,D5,B4,B5,A5,F5,G5,A5,C5,C5,B4};
  u16 u16lengthBuzzer1[]={FN,EN,EN,QN,QN,QN,HN,EN,EN,QN,QN,QN,HN,EN,EN,QN,QN,QN,QN,QN,EN,EN,QN,QN,QN,HN,EN,EN,HN};
  static u8 i=0;//use for the move of note
  /*****************************************************************************
  Description:when the sck input a low level, the leds will blink with the music 
  */
 if((AT91C_BASE_PIOA->PIO_PDSR&0x00008000)==0x00008000)
 {
   /*close the led*/
  if((AT91C_BASE_PIOB->PIO_CODR!=0x00000008)||
  (AT91C_BASE_PIOB->PIO_CODR!=0x00000800))
  {
    AT91C_BASE_PIOB->PIO_CODR|=0x00000008;
    AT91C_BASE_PIOB->PIO_CODR|=0x00000800;
  }
  /*change the corresponding variables*/
  if((bLedisOn1!=FALSE)||(bLedisOn2!=FALSE)||
  (u16TimeCounter!=0))
  {
    bLedisOn1=FALSE;
    bLedisOn2=FALSE;
    u16TimeCounter=0;
  }
 }
 else
 { 
    if(bBuzzerisOn==FALSE)
    {
      PWMAudioOn(BUZZER1);
    }
/*****************************************************************************
  Description:when the time is up blink the led and play the next note
*/
    if(u16TimeCounter==u16lengthBuzzer1[i])
    {
      i++;
      PWMAudioSetFrequency(BUZZER1,u16noteBuzzer1[i]);
      u16TimeCounter=0;
     if(bLedisOn1==FALSE)
     {
      AT91C_BASE_PIOB->PIO_SODR|=0x00000008;//ant0
      bLedisOn1=TRUE;
     }
     else if(bLedisOn1==TRUE)
     {
       AT91C_BASE_PIOB->PIO_CODR|=0x00000008;
       bLedisOn1=FALSE;
     }
   }
 /*****************************************************************************
  Description:blink the other led when time is up t0 200ms
*/  
   if(u16TimeCounter==200)
   {
     if(bLedisOn2==FALSE)
     {
       //u16TimeCounter=0;
       AT91C_BASE_PIOA->PIO_SODR|=0x00000800;//rx
       bLedisOn2=TRUE;
     }
     else if(bLedisOn2==TRUE)
     {
       //u8TimeCounter=0;
       AT91C_BASE_PIOB->PIO_CODR|=0x00000800;
       bLedisOn2=FALSE;
     }
   }
  /*if music play to end , close the music */
  if(i>sizeof(u16lengthBuzzer1))
  {
    PWMAudioOff(BUZZER1);
    bBuzzerisOn=FALSE;
  }
  u16TimeCounter++;
 }
    
} /* end UserAppSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  UserApp_StateMachine = UserAppSM_Idle;
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
