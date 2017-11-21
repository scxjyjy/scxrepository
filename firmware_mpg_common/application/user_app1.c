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

static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */
static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


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
  AntAssignChannelInfoType sAntSetupData;
  
  PWMAudioSetFrequency(BUZZER1,0);
  PWMAudioOn(BUZZER1); 
  
  /* Clear screen */
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR,"Hide and Go Seek!");
  LCDMessage(LINE2_START_ADDR,"Press B0 to Start");

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
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel assignment is queued so start timer */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
   
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
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
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);
    /* Set timer and advance states */
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
static void UserApp1SM_Idle(void)
{
  static bool bReseted = TRUE;
  static bool bReconfiged = FALSE;
  static bool bChannelUnassigned = FALSE;
  static bool bChannelClosed = FALSE;
  static bool bDisplay = FALSE;
  static bool bGotNewData = FALSE;
  
  if(bReseted)
  {
    if( AntReadAppMessageBuffer() )
    {
      if(G_eAntApiCurrentMessageClass == ANT_DATA)
      {
        bGotNewData = FALSE;
        for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
        {
          if(G_au8AntApiCurrentMessageBytes[i] != au8LastAntData[i])
          {
            
            bGotNewData = TRUE;
            au8LastAntData[i] = G_au8AntApiCurrentMessageBytes[i];
          }
        }
      }
      if(bGotNewData)
      {
        bReseted = FALSE;
        UserApp1_StateMachine = UserApp1SM_Seeker;
      }
    }
    
    if(WasButtonPressed(BUTTON0))
    {
      if(!bChannelClosed)
      {
        AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
        
        bChannelClosed = TRUE;
        /* Set timer and advance states */
        UserApp1_u32Timeout = G_u32SystemTime1ms;
        UserApp1_StateMachine = UserApp1SM_WaitChannelClose;
        
      }
      else
      {
        if(!bChannelUnassigned)
        {
          AntUnassignChannelNumber(ANT_CHANNEL_USERAPP);
          
          bChannelUnassigned = TRUE;
          
          UserApp1_u32Timeout = G_u32SystemTime1ms;
          UserApp1_StateMachine = UserApp1SM_WaitChannelUnassign;
        }
        else
        {
          if(!bReconfiged)
          {
            bReconfiged = TRUE;
            
            UserApp1_u32Timeout = G_u32SystemTime1ms;
            UserApp1_StateMachine = UserApp1SM_ReconfigToMaster;
          }
          else
          {
            if(bChannelClosed && bChannelUnassigned && bReconfiged)
            {
              ButtonAcknowledge(BUTTON0);
              bChannelClosed = FALSE;
              bChannelUnassigned = FALSE;
              bReconfiged = FALSE;
              
              bReseted = FALSE;
              UserApp1_StateMachine = UserApp1SM_Hider;
            }
          }
        }
      }
    }
  }
  else
  {
    if(!bDisplay)
    {
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR,"Hide and Go Seek!");
      LCDMessage(LINE2_START_ADDR,"Press B0 to Start");
      bDisplay = TRUE;
    }

    if(!bChannelUnassigned)
    {
      AntUnassignChannelNumber(ANT_CHANNEL_USERAPP);
      
      bChannelUnassigned = TRUE;
      
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserApp1SM_WaitChannelUnassign;
    }
    else
    {
      if(!bReconfiged)
      {
        bReconfiged = TRUE;
        
        UserApp1_StateMachine = UserApp1SM_ReconfigToSlave;
      }
      else
      {
        if(bChannelUnassigned && bReconfiged)
        {
          bChannelClosed = FALSE;
          bChannelUnassigned = FALSE;
          bReconfiged = FALSE;
          
          bReseted = TRUE;
          bDisplay = FALSE;
        }
      }
    }
  }
    
} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to open */
static void UserApp1SM_WaitChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN)
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
    UserApp1_StateMachine = UserApp1SM_Error;
    
  }
    
} /* end UserApp1SM_WaitChannelOpen() */



static void UserApp1SM_Seeker(void)
{
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0, 0, 0, 0};
  static s8 s8RSSILevel = 0;
  static u8 au8Display1[] = "Seeker       ";
  static u8 au8Display2[] = "Here I come!";
  static u8 au8Display3[] = "Ready or not";
  static u16 u16Count = 9999;
  static bool bDisplayed = FALSE;
  static bool bFindSuccess = FALSE;
  static u16 u16BuzzerCount = 0;

  if(u16Count != 0)
  {
    u16Count--;
    if(u16Count % 1000 == 0)
    {
      au8Display1[7] = u16Count / 1000 + '0';
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR,au8Display1);
    }
  }
  else
  {
    if(!bDisplayed)
    {
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR,au8Display3);
      LCDMessage(LINE2_START_ADDR,au8Display2);
      bDisplayed = TRUE;
    }
    /* Always check for ANT messages */
    if( AntReadAppMessageBuffer() )
    {
       /* New data message: check what it is */
      if(G_eAntApiCurrentMessageClass == ANT_DATA)
      {
        for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
        {
          if(G_au8AntApiCurrentMessageBytes[i] != au8LastAntData[i])
          {
            au8LastAntData[i] = G_au8AntApiCurrentMessageBytes[i];
          }
        }

        s8RSSILevel = G_sAntApiCurrentMessageExtData.s8RSSI;
        //au8Display[1] = ((s8RSSILevel*(-1)) % 1000) / 100 + '0';
        //au8Display[2] = ((s8RSSILevel*(-1)) % 100) / 10 + '0';
        //au8Display[3] = ((s8RSSILevel*(-1)) % 10) / 1 + '0';
        //LCDCommand(LCD_CLEAR_CMD);
        //LCDMessage(LINE1_START_ADDR,au8Display);
      
        u16BuzzerCount++;
        PWMAudioSetFrequency(BUZZER1,C4);
        switch((s8RSSILevel*(-1)) / 10)
        {
       case 4 :
         {
            LedOn(WHITE);
            LedOn(PURPLE);
            LedOn(BLUE);
            LedOn(CYAN);
            LedOn(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            PWMAudioOff(BUZZER1);
            u16BuzzerCount = 0;
            bFindSuccess = TRUE;
            break;
          }
        case 5:
          {
            LedOff(WHITE);
            LedOn(PURPLE);
            LedOn(BLUE);
            LedOn(CYAN);
            LedOn(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            PWMAudioOn(BUZZER1);
            u16BuzzerCount = 0;
            break;
          }
        case 6:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOn(BLUE);
            LedOn(CYAN);
            LedOn(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            if(u16BuzzerCount == 250)
            {
              PWMAudioOn(BUZZER1);
            }
            if(u16BuzzerCount > 500)
            {
              PWMAudioOff(BUZZER1);
              u16BuzzerCount = 0;
            }
            break;
          }
        case 7:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOn(CYAN);
            LedOn(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            if(u16BuzzerCount == 500)
            {
              PWMAudioOn(BUZZER1);
            }
            if(u16BuzzerCount > 1000)
            {
              PWMAudioOff(BUZZER1);
              u16BuzzerCount = 0;
            }
            break;
          }
        case 8:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOff(CYAN);
            LedOn(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            if(u16BuzzerCount == 1000)
            {
              PWMAudioOn(BUZZER1);
            }
            if(u16BuzzerCount > 2000)
            {
              PWMAudioOff(BUZZER1);
              u16BuzzerCount = 0;
            }
            break;
          }
        case 9:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOff(CYAN);
            LedOff(GREEN);
            LedOn(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            if(u16BuzzerCount == 2000)
            {
              PWMAudioOn(BUZZER1);
            }
            if(u16BuzzerCount > 4000)
            {
              PWMAudioOff(BUZZER1);
              u16BuzzerCount = 0;
            }
            break;
          }
        case 10:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOff(CYAN);
            LedOff(GREEN);
            LedOff(YELLOW);
            LedOn(ORANGE);
            LedOn(RED);
            if(u16BuzzerCount == 4000)
            {
              PWMAudioOn(BUZZER1);
            }
            if(u16BuzzerCount > 8000)
            {
              PWMAudioOff(BUZZER1);
              u16BuzzerCount = 0;
            }
            break;
          }
        case 11:
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOff(CYAN);
            LedOff(GREEN);
            LedOff(YELLOW);
            LedOff(ORANGE);
            LedOn(RED);
            break;
          }
        default :
          {
            LedOff(WHITE);
            LedOff(PURPLE);
            LedOff(BLUE);
            LedOff(CYAN);
            LedOff(GREEN);
            LedOff(YELLOW);
            LedOff(ORANGE);
            LedOff(RED);
          }
        }
      }
      if(G_eAntApiCurrentMessageClass == ANT_DATA)
      {
        if(bFindSuccess)
        {
          LCDCommand(LCD_CLEAR_CMD);
          LCDMessage(LINE1_START_ADDR,"Found you!");
          AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
          UserApp1_StateMachine = UserApp1SM_Found;
          bFindSuccess = FALSE;
          u16Count = 9999;
          bDisplayed = FALSE;
        }
      }
    }
  } /* end AntReadAppMessageBuffer() */
      
} /* end UserApp1SM_ChannelOpen() */

static void UserApp1SM_Hider(void)
{
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};
  static bool bDisplayed = FALSE;
  
  if(!bDisplayed)
  {
    bDisplayed = TRUE;
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR,"Hide!");
  }
  
  if( AntReadAppMessageBuffer() )
  {
     /* New message from ANT task: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR,"You found me!");
      UserApp1_StateMachine = UserApp1SM_Found;
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
     /* Update and queue the new message data */
      au8TestMessage[7]++;
      if(au8TestMessage[7] == 0)
      {
        au8TestMessage[6]++;
        if(au8TestMessage[6] == 0)
        {
          au8TestMessage[5]++;
        }
      }
      AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
    }
    
  } /* end AntReadData() */
      
}


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to close */
static void UserApp1SM_WaitChannelClose(void)
{
  /* Monitor the channel status to check if channel is closed */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CLOSED)
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    UserApp1_StateMachine = UserApp1SM_Error;
  }
    
} /* end UserApp1SM_WaitChannelClose() */

static void UserApp1SM_ReconfigToMaster(void)
{
  AntAssignChannelInfoType sAntSetupData;
  
  /* Configure ANT for this application */
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = CHANNEL_TYPE_MASTER;
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
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel assignment is queued so start timer */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
   
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    UserApp1_StateMachine = UserApp1SM_Error;
  }

}

static void UserApp1SM_ReconfigToSlave(void)
{
  AntAssignChannelInfoType sAntSetupData;
  
  /* Configure ANT for this application */
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = CHANNEL_TYPE_SLAVE;
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
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel assignment is queued so start timer */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
   
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    UserApp1_StateMachine = UserApp1SM_Error;
  }

}

static void UserApp1SM_Found(void)
{
  static u16 u16Count = 0;
  static bool bCongratulated = FALSE;
  static u16 au16NoteBuzzer1[] = {NO,C6,D6,E6,G6,D6,A5,B5,C6,D6,E6,F6,E6,NO,E6,D6,E6,G6,D6,A5,B5,C6,B5,A5,G5,E5,NO,C6,D6,E6,G6,D6,A5,B5,C6,D6,E6,F6,E6,NO,E6,C6,C6,E6,D6,B5,C6,B5,A5,NO,
                                     C5,D5,E5,G5,D5,A4,B4,C5,D5,E5,F5,E5,NO,E5,D5,E5,G5,D5,A4,B4,C5,B4,A4,G4,E4,NO,C5,D5,E5,G5,D5,A4,B4,C5,D5,E5,F5,E5,NO,E5,C5,C5,E5,D5,B4,C5,B4,A4,NO};
  static u16 au16NoteBuzzer2[] = {NO,A4,NO,G4,NO,F4,NO,C4,NO,A4,NO,G4,NO,F4,G4,C5,NO,A4,NO,G4,NO,F4,NO,C4,NO,D4,NO,E4,NO,F4,NO};
  static u16 au16LengthBuzzer1[] = {FN,EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,QN,EN,EN,FN,QN*3,
                                       EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,EN,EN,EN,EN,EN,EN,QN,QN,EN,EN,EN,EN,QN,QN,EN,EN,QN,QN*2};
  static u16 au16LengthBuzzer2[] = {FN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN,QN*5,QN*3};
  static u16 u16Counter1 = 0;
  static u16 u16Counter2 = 0;
  static u16 u16NoteCount1 = 0;
  static u16 u16NoteCount2 = 0;
  
  u16Counter1++;
  u16Counter2++;
  
  if(u16Counter1 == au16LengthBuzzer1[u16NoteCount1])
  {
    u16Counter1 = 0;
    u16NoteCount1++;
    PWMAudioSetFrequency(BUZZER1,au16NoteBuzzer1[u16NoteCount1]);
  }
  
  if(u16Counter2 == au16LengthBuzzer2[u16NoteCount2])
  {
    u16Counter2 = 0;
    u16NoteCount2++;
    PWMAudioSetFrequency(BUZZER2,au16NoteBuzzer2[u16NoteCount2]);
  }
  
  if(u16NoteCount1 == (sizeof(au16LengthBuzzer1)/2-1))
  {
    u16NoteCount1 = 0;
  }
  
  if(u16NoteCount2 == (sizeof(au16LengthBuzzer2)/2-1))
  {
    u16NoteCount2 = 0;
  }
  
  
  u16Count++;
  if( AntReadAppMessageBuffer() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if(G_au8AntApiCurrentMessageBytes[i] != au8LastAntData[i])
        {
          au8LastAntData[i] = G_au8AntApiCurrentMessageBytes[i];
        }
      }
    }
  }
  if(!bCongratulated)
  {
    bCongratulated = TRUE;
    LedBlink(WHITE,LED_4HZ);
    LedBlink(PURPLE,LED_4HZ);
    LedBlink(BLUE,LED_4HZ);
    LedBlink(CYAN,LED_4HZ);
    LedBlink(GREEN,LED_4HZ);
    LedBlink(YELLOW,LED_4HZ);
    LedBlink(ORANGE,LED_4HZ);
    LedBlink(RED,LED_4HZ);
  }
  if(u16Count == 10000)
  {
    u16Count = 0;
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);
    PWMAudioSetFrequency(BUZZER1,0);
    
    
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
    
    /* Set timer and advance states */
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitChannelClose;
    bCongratulated = FALSE;
  }

}

static void UserApp1SM_WaitChannelUnassign(void)
{
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_UNCONFIGURED)
  {
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, TIMEOUT_VALUE) )
  {
    UserApp1_StateMachine = UserApp1SM_Error;
  }
}



/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  LedOn(GREEN);
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
