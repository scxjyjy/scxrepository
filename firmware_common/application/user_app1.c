/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief Application to search and be search using two ANT channels.  

It is expected that another device is running this code searching for you!  
The other device should be programmed with the same Device ID.  

The user presses BUTTON0 to open the ANT channels and begin searching.  Both a Master
and Slave channel are opened to ensure that devices will find each other.  During search,
the RSSI level of any received message will be reported on the LCD.  If you get close enough
to the matching systems and the RSSI level is strong enough, the other person's name
will be shown on your LCD.

The board's LEDs are used to indicate the relative signal strength.  RED indicates
that no message are being received.  As signal strength increases, more LEDs will turn on.
The White LED will be lit at maximum RSSI (the same level that must be reached to display
the other person's name).

Reception of a message on the Master's channel will also activate the BLUE LCD backlight
Reception of a message on the Slave's channel will also activate the GREEN LCD backlight


------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!<@brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!<@brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */

/* Globals for passing data from the ANT application to the API */
extern u32 G_u32AntApiCurrentMessageTimeStamp;                            // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;            // From ant_api.c
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;                // From ant_api.c


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;                 /*!< @brief The state machine function pointer */
static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */

/* #EIE Task 1
Enter your name below where it says "YOURNAME"  
MAXIMUM 8 CHARACTERS:             12345678               */
static u8 UserApp1_au8MyName[] = "Hello!";

static AntAssignChannelInfoType UserApp1_sMasterChannel;
static AntAssignChannelInfoType UserApp1_sSlaveChannel;
  
static u8 UserApp1_au8LcdStartLine1[] = "Hi \0\0\0\0\0\0\0\0";
static u8 UserApp1_au8LcdStartLine2[] = "Push B0 to search";
static u8 UserApp1_au8MasterName[9]   = "0\0\0\0\0\0\0\0";
static u8 UserApp1_au8LcdInformationMessage[] = "M:-xx dbm  S:-xx dbm";
static u8 u8Distance=0;
static u8 UserApp1_au8LcdInformationMessage1[] = "Distance:x m";
static u16 u16musicalnote[]={C4,D4,E4,F4,G4,A4,B4,C5};
static bool bBuzzerisOn=FALSE;
static u8 INDEX=0;
/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  u8 u8NameSize;

  /* Start with all LEDs off except red blacklight */
  LedOff(RED);
  LedOff(ORANGE);
  LedOff(YELLOW);
  LedOff(GREEN);
  LedOff(CYAN);
  LedOff(BLUE);
  LedOff(PURPLE);
  LedOff(WHITE);
  LedOn(LCD_GREEN);
  LedOff(LCD_BLUE);
  LedOff(LCD_RED);

  /* Check the size of the name */
  u8NameSize = sizeof(UserApp1_au8MyName);
  if(u8NameSize > 8)
  {
    u8NameSize = 8;
  }
  
  /* Update the name message and UserApp1_au8MasterName*/
  for(u8 i = 0; i < u8NameSize; i++)
  {
    UserApp1_au8LcdStartLine1[3 + i] = UserApp1_au8MyName[i];
    UserApp1_au8MasterName[i] = UserApp1_au8MyName[i];
  }
  
  /* Update LCD to starting screen. */
  LCDCommand(LCD_CLEAR_CMD);
  
  /* This delay is usually required after LCDCommand during INIT */
  for(u32 i = 0; i < 100000; i++);
  
  LCDMessage(LINE1_START_ADDR, UserApp1_au8LcdStartLine1);
  LCDMessage(LINE2_START_ADDR, UserApp1_au8LcdStartLine2);
  
  /* Set up the two ANT channels that will be used for the task */
  
  /* Master (Channel 0) */
  UserApp1_sMasterChannel.AntChannel = ANT_CHANNEL_0;
  UserApp1_sMasterChannel.AntChannelType = CHANNEL_TYPE_MASTER;
  UserApp1_sMasterChannel.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
  UserApp1_sMasterChannel.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
  
  UserApp1_sMasterChannel.AntDeviceIdHi = FOO;
  UserApp1_sMasterChannel.AntDeviceIdLo = BAR;
  UserApp1_sMasterChannel.AntDeviceType = EIE_DEVICE_TYPE;
  UserApp1_sMasterChannel.AntTransmissionType = EIE_TRANS_TYPE;
  
  UserApp1_sMasterChannel.AntFrequency = ANT_FREQUENCY_DEFAULT;
  UserApp1_sMasterChannel.AntTxPower = ANT_TX_POWER_DEFAULT;
  UserApp1_sMasterChannel.AntNetwork = ANT_NETWORK_DEFAULT;
  
  /* Slave (Channel 1) */
  UserApp1_sSlaveChannel.AntChannel = ANT_CHANNEL_1;
  UserApp1_sSlaveChannel.AntChannelType = CHANNEL_TYPE_SLAVE;
  UserApp1_sSlaveChannel.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
  UserApp1_sSlaveChannel.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
  
  UserApp1_sSlaveChannel.AntDeviceIdHi = FOO;
  UserApp1_sSlaveChannel.AntDeviceIdLo = BAR;
  UserApp1_sSlaveChannel.AntDeviceType = EIE_DEVICE_TYPE;
  UserApp1_sSlaveChannel.AntTransmissionType = EIE_TRANS_TYPE;
  
  UserApp1_sSlaveChannel.AntFrequency = ANT_FREQUENCY_DEFAULT;
  UserApp1_sSlaveChannel.AntTxPower = ANT_TX_POWER_DEFAULT;
  UserApp1_sSlaveChannel.AntNetwork = ANT_NETWORK_DEFAULT;
  
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    UserApp1_sMasterChannel.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
    UserApp1_sSlaveChannel.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
  
  /* Queue configuration of Master channel */
  AntAssignChannel(&UserApp1_sMasterChannel);
  UserApp1_u32Timeout = G_u32SystemTime1ms;
    
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_AntConfigureMaster;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

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


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_AntConfigureMaster(void)

@brief Wait for Master channel configuration to be completed.
*/
static void UserApp1SM_AntConfigureMaster(void)
{
  /* Wait for the ANT master channel to be configured */
  if(AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_CONFIGURED)
  {
    DebugPrintf("Master channel configured\n\n\r");
    
    /* Queue configuration of Slave channel */
    AntAssignChannel(&UserApp1_sSlaveChannel);
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_AntConfigureSlave;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, ANT_CONFIGURE_TIMEOUT_MS) )
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Master config failed");
    UserApp1_StateMachine = UserApp1SM_Error;    
  }

} /* end UserApp1SM_AntConfigureMaster() */
    

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_AntConfigureSlave(void)

@brief Wait for Slave channel configuration to be completed.
*/
static void UserApp1SM_AntConfigureSlave(void)
{
  /* Wait for the ANT slave channel to be configured */
  if(AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_CONFIGURED)
  {
    DebugPrintf("Slave channel configured\n\n\r");
    
    /* Update the broadcast message data to send the user's name the go to Idle */
    AntQueueBroadcastMessage(ANT_CHANNEL_0, UserApp1_au8MasterName);
    AntQueueBroadcastMessage(ANT_CHANNEL_1, UserApp1_au8MasterName);

    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, ANT_CONFIGURE_TIMEOUT_MS) )
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Slave config failed");
    UserApp1_StateMachine = UserApp1SM_Error;    
  }

} /* end UserApp1SM_AntConfigureSlave() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_Idle(void)

@brief Wait for the user to press BUTTON0 to start search mode
*/
static void UserApp1SM_Idle(void)
{
  /* Write the one line of code to use the BUTTON API to check if BUTTON0 was pressed */ 
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    
    /* Queue the Channel Open messages and then go to wait state */
    AntOpenChannelNumber(ANT_CHANNEL_0);
    AntOpenChannelNumber(ANT_CHANNEL_1);
    
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_OpeningChannels;    
  }

} /* end UserApp1SM_Idle() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_OpeningChannels(void)

@brief Wait for ANT channels to open
*/
static void UserApp1SM_OpeningChannels(void)
{
  /* Ensure that both channels have opened */
  if( (AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_OPEN) &&
      (AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_OPEN) )
  {
    /* Update LCD and go to main Radio monitoring state */
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, UserApp1_au8LcdInformationMessage1);
    LCDMessage(LINE2_START_ADDR, UserApp1_au8MasterName);
    
    UserApp1_StateMachine = UserApp1SM_RadioActive;    
  }

  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, ANT_CONFIGURE_TIMEOUT_MS) )
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Channel open failed");
    UserApp1_StateMachine = UserApp1SM_Error;    
  }

} /* end UserApp1SM_OpeningChannels() */
    

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_RadioActive(void)

@brief Monitor incoming messages.  Watch signal strength of messages and update LCD and LEDs.
*/
static void UserApp1SM_RadioActive(void)
{
  LedNumberType aeLedDisplayLevels[] = {RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, PURPLE, WHITE};
  s8 as8dBmLevels[] = {DBM_LEVEL1, DBM_LEVEL2, DBM_LEVEL3, DBM_LEVEL4, 
                       DBM_LEVEL5, DBM_LEVEL6, DBM_LEVEL7, DBM_LEVEL8};
  u8 u8EventCode;
  u8 au8UserName[9];
    


  /* Monitor ANT messages: looking for any incoming messages
  that indicates a matching device has been located. */
  if( AntReadAppMessageBuffer() )
  {
    /* Check the message class to determine how to process the message */
    if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      
    } /* end if(G_eAntApiCurrentMessageClass == ANT_TICK) */

    
    /* Check for DATA messages */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
     
    }
                        
     
    } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */
} /* end UserApp1SM_RadioActive() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_ClosingChannels(void)

@brief Wait for ANT channels to close.
*/
static void UserApp1SM_ClosingChannels(void)
{
  /* Ensure that both channels have opened */
  if( (AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_CLOSED) &&
      (AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_CLOSED) )
  {
    UserApp1_StateMachine = UserApp1SM_Idle;    
  }

  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, ANT_CONFIGURE_TIMEOUT_MS) )
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Channel close failed");
    UserApp1_StateMachine = UserApp1SM_Error;    
  }

} /* end UserApp1SM_ClosingChannels() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_Error(void)

@brief Handle an error here.  For now, the task is just held in this state. 
*/
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
