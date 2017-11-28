/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
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
extern u32 G_u32AntApiCurrentDataTimeStamp;                       /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;        /* From ant_api.c */

extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static DisplayStateType eDisplayState=Empty;
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */
static u8 AntNetworkKey[]={0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45};
static sLine1Menu sLine1menu;
static sLine2Menu sLine2menu;
static u16 u16TimeCounter=0;
static bool bprompt=FALSE;
static bool bExitCommand=FALSE;
static bool bF3PageUp=FALSE;//display menu
static bool bF3PageDown=FALSE;
static bool bPageUp=FALSE;//local menu
static bool bPageDown=FALSE;
static bool bButton2Confirmed=FALSE;
static u8 Uioption=1;//don't change this variable easy

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
  u8 au8WelcomeMessage[] = "loading...";
  sLine1menu.UIoption1_Line1="1.Average rate   <-";
  sLine1menu.UIoption2_Line1="1.Average rate     ";
  sLine1menu.UIoption3_Line1="3.History        <-";
  sLine1menu.UIoption4_Line1="3.History          ";
  sLine2menu.UIoption1_Line2="2.Max and mini     V";
  sLine2menu.UIoption2_Line2="2.Max and mini   <-V";
  sLine2menu.UIoption3_Line2="4.Developer        V";
  sLine2menu.UIoption4_Line2="4.Developer      <-V";
  AntAssignChannelInfoType sAntSetupData;
  
  /* Clear screen and place start messages */
#ifdef EIE1
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8WelcomeMessage);
#endif /* EIE1 */ 
 /* Configure ANT for this application */
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_USERAPP;
  sAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  sAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  
  sAntSetupData.AntDeviceIdLo       = ANT_DEVICEID_LO_USERAPP;
  sAntSetupData.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  sAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  sAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  sAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  sAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;

  sAntSetupData.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    sAntSetupData.AntNetworkKey[i] = AntNetworkKey[i];
  }
    
  /* If good initialization, set state to channelopen */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel assignment is queued so start timer */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    DebugPrintf("channel1 initialized failed");
    UserApp1_StateMachine = UserApp1SM_Error;
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
/* Wait for the ANT channel assignment to finish */
static void UserApp1SM_WaitChannelAssign(void)
{
  /* Check if the channel assignment is complete */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CONFIGURED)
  {
    DebugPrintf("channel1 assign successfully");
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelOpen;
  }
  
  /* Monitor for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, 5000) )
  {
    DebugPrintf("\n\r***Channel assignment timeout***\n\n\r");
    UserApp1_StateMachine = UserApp1SM_Error;
  }
      
} /* end UserApp1SM_WaitChannelAssign() */


  /*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserApp1SM_DisplayIdle(void)
{
  if( AntReadAppMessageBuffer() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      eDisplayState=JudgeDisplayState();
      switch(eDisplayState)
      {
        case Feature1:
          LCDCommand(LCD_CLEAR_CMD); 
          LCDMessage(LINE1_START_ADDR,"Feature1 is Run");
          LCDMessage(LINE2_START_ADDR,"Button3 to exit");
          UserApp1_StateMachine =UserApp1SM_DisplayWorking;
          break;
        case Feature2:
          LCDCommand(LCD_CLEAR_CMD); 
          LCDMessage(LINE1_START_ADDR,"Feature2 is Run");
          LCDMessage(LINE2_START_ADDR,"Button3 to exit");
          UserApp1_StateMachine =UserApp1SM_DisplayWorking;
          break;
        case Feature3:
          LCDCommand(LCD_CLEAR_CMD); 
          LCDMessage(LINE1_START_ADDR,"Feature3 is Run");
          LCDMessage(LINE2_START_ADDR,"Button3 to exit");
          UserApp1_StateMachine =UserApp1SM_DisplayWorking;
          break;
        case Feature4:
          LCDCommand(LCD_CLEAR_CMD); 
          LCDMessage(LINE1_START_ADDR,"Feature4 is Run");
          LCDMessage(LINE2_START_ADDR,"Button3 to exit");
          UserApp1_StateMachine =UserApp1SM_DisplayWorking;
          break;
      }
      if(WasButtonPressed(BUTTON0))
      {
        ButtonAcknowledge(BUTTON0);
        if(Uioption==STARTUIOPTION)
        {
          Uioption=FINALUIOPTION;
        }
        else
        {
          Uioption--;
        }
        PWMAudioSetFrequency(BUZZER1, 500);
        PWMAudioOn(BUZZER1);
        bPageUp=TRUE;
        UserApp1_u32Timeout = G_u32SystemTime1ms;
        UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;  
      }
      
      
      if(WasButtonPressed(BUTTON1))
      {
        ButtonAcknowledge(BUTTON1);
        if(Uioption==FINALUIOPTION)
        {
          Uioption=STARTUIOPTION;
        }
        else
        {
          Uioption++;
        }
        PWMAudioSetFrequency(BUZZER1, 500);
        PWMAudioOn(BUZZER1);
        bPageDown=TRUE;
        UserApp1_u32Timeout = G_u32SystemTime1ms;
        UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;
      }
      
      if(WasButtonPressed(BUTTON2))
      {
        ButtonAcknowledge(BUTTON2);
        PWMAudioSetFrequency(BUZZER1, 500);
        PWMAudioOn(BUZZER1);
        bButton2Confirmed=TRUE;
        UserApp1_u32Timeout = G_u32SystemTime1ms;
        UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;
      } 
    }
  } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */   
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to open */
static void UserApp1SM_WaitChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN)
  {
    if(bprompt==FALSE)
    {
      LCDCommand(LCD_CLEAR_CMD); 
      LCDMessage(LINE1_START_ADDR,"searching");
      LCDMessage(LINE2_START_ADDR,"welcome!");
      DebugPrintf("channel1 open successfully");
      bprompt=TRUE;
    }
    if(u16TimeCounter==1000)
    {
      u16TimeCounter=0;
      UserApp1_StateMachine = UserApp1SM_ChannelOpen;  
    }
    u16TimeCounter++; 
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    DebugPrintf("channel1 open failed");
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
    UserApp1_StateMachine =UserApp1SM_Error;
  }
    
} /* end UserApp1SM_WaitChannelOpen() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Channel is open, so monitor data */
static void UserApp1SM_ChannelOpen(void)
{
  static u8 au8AnswerMessage[] = {0, 0, 0, 0, 0, 0, 0, 0};
  /* Always check for ANT messages */
  if( AntReadAppMessageBuffer() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      eDisplayState=JudgeDisplayState();
      if(eDisplayState == Pared)
      {
        AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8AnswerMessage);
      }
      if(eDisplayState == Idle)
      {
        u8 u8suffix[]={0};
        u8suffix[0]=23;
        LCDCommand(LCD_CLEAR_CMD); 
        LCDMessage(LINE1_START_ADDR,sLine1menu.UIoption1_Line1);
        LCDMessage(19,u8suffix);
        LCDMessage(LINE2_START_ADDR,sLine2menu.UIoption1_Line2);
        UserApp1_StateMachine =UserApp1SM_DisplayIdle;
      }
    } /* end if(bGotNewData) */
  } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */     
} /* end UserApp1SM_ChannelOpen() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
   LedOn(RED);
} /* end UserApp1SM_Error() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* DisplayWorking */
static void UserApp1SM_DisplayWorking(void)
{
  if(AntReadAppMessageBuffer())
  {
    eDisplayState=JudgeDisplayState();
    if(eDisplayState==Idle)
    {
      MenuOption(Uioption);
      UserApp1_StateMachine = UserApp1SM_DisplayIdle;
    }
    
    if(WasButtonPressed(BUTTON3))
    {
      ButtonAcknowledge(BUTTON3);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
      bExitCommand=TRUE;
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;   
    }
    
    if(WasButtonPressed(BUTTON0))
    {
      ButtonAcknowledge(BUTTON0);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
      bF3PageDown=TRUE;
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;     
    }
    
    if(WasButtonPressed(BUTTON1))
    {
      ButtonAcknowledge(BUTTON1);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
      bF3PageUp=TRUE;
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserApp1SM_WaitBuzzerOff;     
    }
    
  }

}
/*-------------------------------------------------------------------------------------------------------------------*/
/* Judge  Display State */
static DisplayStateType JudgeDisplayState(void)
{
  if(G_au8AntApiCurrentMessageBytes[0]==0x00)
  {
    return Pared;
  }
  else if(G_au8AntApiCurrentMessageBytes[0]==0xFF)
  { 
    return Idle;
  }
  else if(G_au8AntApiCurrentMessageBytes[0]==0x01)
  { 
    return Feature1;
  }
  else if(G_au8AntApiCurrentMessageBytes[0]==0x02)
  {
   return Feature2;
  }
  else if(G_au8AntApiCurrentMessageBytes[0]==0x03)
  {
    return Feature3;
  }
  else if(G_au8AntApiCurrentMessageBytes[0]==0x04)
  {
    return Feature4;
  }
  else 
  {
    return Empty;
  }
    
}
/*-------------------------------------------------------------------------------------------------------------------*/
/* MenuOption */
static void MenuOption(u8 UIOption)
{
  u8 u8suffix[]={0};
  u8suffix[0]=23;
  switch(UIOption)
  {
    case 1:
      LCDCommand(LCD_CLEAR_CMD); 
      LCDMessage(LINE1_START_ADDR,sLine1menu.UIoption1_Line1);
      LCDMessage(19,u8suffix);
      LCDMessage(LINE2_START_ADDR,sLine2menu.UIoption1_Line2);
      break;
    case 2:
      LCDCommand(LCD_CLEAR_CMD); 
      LCDMessage(LINE1_START_ADDR,sLine1menu.UIoption2_Line1);
      LCDMessage(19,u8suffix);
      LCDMessage(LINE2_START_ADDR,sLine2menu.UIoption2_Line2);
      break;
    case 3:
      LCDCommand(LCD_CLEAR_CMD); 
      LCDMessage(LINE1_START_ADDR,sLine1menu.UIoption3_Line1);
      LCDMessage(19,u8suffix);
      LCDMessage(LINE2_START_ADDR,sLine2menu.UIoption3_Line2);
      break;
    case 4:
      LCDCommand(LCD_CLEAR_CMD); 
      LCDMessage(LINE1_START_ADDR,sLine1menu.UIoption4_Line1);
      LCDMessage(19,u8suffix);
      LCDMessage(LINE2_START_ADDR,sLine2menu.UIoption4_Line2);
      break;
  }
}
/*-------------------------------------------------------------------------------------------------------------------*/
/* UserApp1SM_WaitBuzzerOff */
static void UserApp1SM_WaitBuzzerOff(void)
{
  static u8 au8CommandExit[]={0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF3PageUp[]={0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF3PageDown[]={0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF1Message[]={0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF2Message[]={0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF3Message[]={0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  static u8 au8CommandF4Message[]={0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  /*clear the display state messages in buffer*/
  AntReadAppMessageBuffer();
  if( IsTimeUp(&UserApp1_u32Timeout, 50) )
  {
    if(bExitCommand==TRUE)
    {
      bExitCommand=FALSE;
      PWMAudioOff(BUZZER1);
      AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandExit);
      UserApp1_StateMachine = UserApp1SM_DisplayWorking;
    }
    if(bF3PageUp==TRUE)
    {
      bF3PageUp=FALSE;
      PWMAudioOff(BUZZER1);
      AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF3PageUp);
      UserApp1_StateMachine = UserApp1SM_DisplayWorking;
    }
    if(bF3PageDown==TRUE)
    {
      bF3PageDown=FALSE;
      PWMAudioOff(BUZZER1);
      AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF3PageDown);
      UserApp1_StateMachine = UserApp1SM_DisplayWorking;
    }
    if(bPageUp==TRUE)
    {
      bPageUp=FALSE;
      PWMAudioOff(BUZZER1);
      MenuOption(Uioption);
      UserApp1_StateMachine =UserApp1SM_DisplayIdle;
    }
    if(bPageDown==TRUE)
    {
      bPageDown=FALSE;
      PWMAudioOff(BUZZER1);
      MenuOption(Uioption);
      UserApp1_StateMachine =UserApp1SM_DisplayIdle;
    }
    if(bButton2Confirmed==TRUE)
    {
      bButton2Confirmed=FALSE;
      PWMAudioOff(BUZZER1);
      switch(Uioption)
      {
        case 1:
         AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF1Message);
         break;
        case 2:
          AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF2Message);
          break;
        case 3:
          AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF3Message);
          break;
        case 4:
          AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8CommandF4Message);
          break;
      }
      UserApp1_StateMachine =UserApp1SM_DisplayIdle;
    }
    
  }
}
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
