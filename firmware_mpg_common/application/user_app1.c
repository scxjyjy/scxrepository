/**********************************************************************************************************************
File: user_app1.c                                                                

Description:
Provides a Tera-Term driven system to display, read and write an LED command list.

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
extern u8 G_u8DebugScanfCharCount;/* From debug.c */
//extern  LedDisplayListHeadType UserApp2_sUserLedCommandList;/* From user_app2.c */
//extern  LedDisplayListHeadType UserApp2_sDemoLedCommandList;/* From user_app2.c */
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
  u8 au8UserApp1Start1[] = "LED program task started\n\r";
  
  /* Turn off the Debug task command processor and announce the task is ready */
  DebugSetPassthrough();
  DebugPrintf(au8UserApp1Start1);
  
  
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







u32 pow(u8 b,u8 power)
{
  u32 mulresult=1;
  for(u8 i=0;i!=power;i++)
  {
    mulresult=mulresult*b;
  }
  return mulresult;
}



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
   static u8 au8EnterString[DEBUG_SCANF_BUFFER_SIZE]="\0";
   static bool bCmdisLegal=FALSE;
   static bool* bpCmdisLegal=&bCmdisLegal;
   static bool bOutPutCmdLine=TRUE;
   static bool *bpOutPutCmdLine=&bOutPutCmdLine;
   static u8 u8StrLen=0;
   static u8* pu8StrLen=&u8StrLen;
   static u8 u8LedType='\0';
   static u8* pu8LedType=&u8LedType;
   static u8 s2=0;
   static u8* ps2=&s2;
   static u8 s1=0;
   static u8* ps1=&s1;
   static u32 u32Counter=0;
   static u8 au8OnString[10]="\0";
   static u8 au8OffString[10]="\0";
   static bool bMenuPrinted=FALSE;
   static bool *bpMenuPrinted=&bMenuPrinted;
   static bool bChoose1=FALSE;
   static bool*bpChoose1=&bChoose1;
   static bool bChoose2=FALSE;
   static bool bEnterCompleted=TRUE;
   static bool *bpEnterCompleted=&bEnterCompleted;
   //SaveToRepository(pu8LedType,ps1,ps2,au8OnString,au8OffString,pu8StrLen);
   //LedDisplayListNodeType sStartNormalNode;
   //LedDisplayListNodeType *psStartNormalNode;
   //LedDisplayListNodeType sEndNormalNode;
   //sStartNormalNode.eCommand.eLED=RED;
   //sStartNormalNode.eCommand.u32Time=100;
   //sEndNormalNode.eCommand.eLED=RED;
   //sEndNormalNode.eCommand.u32Time=100;
   //sEndNormalNode.psNextNode=NULL;
   //sStartNormalNode.psNextNode=NULL;
   //sStartNormalNode.psNextNode=&sEndNormalNode;
   // psStartNormalNode=&sStartNormalNode;
   //OutPutCmdLineList(pbOutPutCmdLine);
   //DebugPrintNumber(u8LedType);
   //DebugLineFeed();
   //SaveToRepository(pu8LedType,NULL,NULL,NULL,NULL);
   /*if(G_au8DebugScanfBuffer[G_u8DebugScanfCharCount-1]=='\r')
   {
     u8StrLen=DebugScanf(au8EnterString);
     Check_and_Cutout(au8EnterString,pu8StrLen,bpCmdisLegal,pu8LedType,ps1,ps2,au8OnString,au8OffString);
     DebugPrintf(au8OnString);
     DebugPrintf(au8OffString);
   }*/
#if 1
   if(bOutPutCmdLine==TRUE)
   {
     if(bMenuPrinted==TRUE)
     { 
      if(G_au8DebugScanfBuffer[0]=='1')
      {
        if(G_au8DebugScanfBuffer[1]=='\r')
        {
          bChoose1=TRUE;
          G_au8DebugScanfBuffer[0]='\0';
          G_au8DebugScanfBuffer[1]='\0';
          G_u8DebugScanfCharCount=0;
        }
      }
      else if(G_au8DebugScanfBuffer[0]=='2')
      {
        if(G_au8DebugScanfBuffer[1]=='\r')
        {
          bChoose2=TRUE;
          G_au8DebugScanfBuffer[0]='\0';
          G_au8DebugScanfBuffer[1]='\0';
          G_u8DebugScanfCharCount=0;
        }
      }
      if(bEnterCompleted==TRUE)
      {
        if(bChoose1==TRUE)
        {
          bEnterCompleted=FALSE;
          DebugPrintf("\n\r\n\rEnter commands as LED-ONTIME-OFFTIME and press Enter\n\rTime is in milliseconds,max 100 commands\n\rLED colors:R,O,Y,G,C,B,P,W\n\rExample: R-100-200(RED on at 100ms and off at 200ms\n\r\n\r1 : ");   

        }
        else if(bChoose2==TRUE)
        {
          DebugPrintf("\n\r\n\rCurrent USER program:\n\r\nLED\tONTIME\t\tOFFTIME\n\r--------------------------------\n\r");
          bOutPutCmdLine=TRUE;
          OutPutCmdLineList(bpOutPutCmdLine);
          bMenuPrinted=FALSE;
          bChoose2=FALSE;
        }
      }
      else
      {
        CmdLineParser(bpChoose1,bpOutPutCmdLine,au8EnterString,pu8StrLen,bpCmdisLegal,pu8LedType,ps1,ps2,au8OnString,au8OffString,bpMenuPrinted,bpEnterCompleted);
      }
    }
    else
    {
      DebugPrintf("****************************************************\n\r");
      DebugPrintf("LED Programing Interface\n\rPress 1 to program command sequence\n\rPress 2 to show current User program\n\r");
      DebugPrintf("****************************************************\n\r");
      bMenuPrinted=TRUE;
    }
   }
   else
   {
     u32Counter++;
     if(u32Counter==500)
     {
      OutPutCmdLineList(bpOutPutCmdLine);
      u32Counter=0;
     }
   }
#endif
  
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
