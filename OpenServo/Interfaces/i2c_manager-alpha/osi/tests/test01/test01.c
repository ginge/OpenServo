/*
 * Author       : Kevin Black
 * Date written : 29 June 2007
 * Licensing    :
 *
 * This software is released under the MIT license.
 *
    Copyright (c) 2007 Kevin Black

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

 *
 * Synopsis     :
 *
 * I2C Manager and OpenServo (software, via I2C Manager) Interface Development/Test/Demonstration
 *
 * This application take 0, 1 or 2 arguments, if a first argument is present, then it is
 * assumed to be the ID of an OpenServo with which the application should communicate,
 * taken from the indices that the application lists. If no argument is given, then the
 * application lists I2C devices and identified OpenServos only. The second argument can
 * be -run, this will (assuming no other errors) cause the application to call the
 * run_OpenServo function to run the servo continuously sweeping between 150 and 850.
 */
#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include "../../osi.h"

#ifdef __GNUC__
#define _snprintf snprintf
#define Sleep(X) usleep(X*1000)
#endif

/*
 *
 */
static OPENSERVOREGSET l_regs;

/*
 * Structure for retrieving and dumping OpenServo registers to a stream
 */
#define ITEM(ID,SIGNED,VARIABLE,FORMAT,OUTFORMAT) { OSI_REGID_##ID, SIGNED, sizeof(VARIABLE), & VARIABLE, -1, FORMAT, OUTFORMAT }
typedef struct REGINDEXITEM
{
   OSI_REGID m_nOSID;        /* OSI register ID */
   bool_t m_bIsSigned;       /* TRUE if the value is signed */
   int m_nSize;              /* Size of the item */
   void *m_pValue;           /* Location in which the value should be stored */
   int_t m_rc;               /* The rc from reading the register */
   const char *m_pValFormat; /* Format to be used to convert the value to string form */
   const char *m_pOutFormat; /* Format to be used to output the formatted string with labels, etc. */
} REGINDEXITEM;
#define DECLREGINDEXITEM(ID,SIGNED,VARIABLE,FORMAT,OUTFORMAT) { OSI_REGID_##ID, SIGNED, sizeof(VARIABLE), & VARIABLE, -1, FORMAT, OUTFORMAT }

/*
 * Define a basic set of register retrievals for dumping to stream
 */
REGINDEXITEM l_GenRegIndex[]=
{
   ITEM(DEVICETYPE,       FALSE,  l_regs.m_DeviceType,      "%d",     "Device type %s." ),
   ITEM(DEVICESUBTYPE,    FALSE,  l_regs.m_DeviceSubtype,   "%d",     "%s, version " ),
   ITEM(VERSIONMAJOR,     FALSE,  l_regs.m_VersionMajor,    "%d",     "%s." ),
   ITEM(VERSIONMINOR,     FALSE,  l_regs.m_VersionMinor,    "%d",     "%s\n" ),
   ITEM(TWI_ADDRESS,      FALSE,  l_regs.m_I2CAddress,      "0x%02X", "Address=%-10s" ),
   ITEM(TIMER,            FALSE,  l_regs.m_Timer,           "%u",     "ADC tmr=%-10s" ),
   ITEM(FLAGS,            FALSE,  l_regs.m_Flags,           "0x%04X", "Flags=%-12s" ),
   ITEM(BANKSELECT,       FALSE,  l_regs.m_BankSelect,      "0%02X",  "Cur bank=%-9s\n" ),
   ITEM(POSITION,         FALSE,  l_regs.m_Position,        "%d",     "Position=%-9s" ),
   ITEM(VELOCITY,          TRUE,  l_regs.m_Velocity,        "%d",     "Velocity=%-9s" ),
   ITEM(SEEK,             FALSE,  l_regs.m_Seek,            "%d",     "Seek=%-13s" ),
   ITEM(SEEKVELOCITY,      TRUE,  l_regs.m_SeekVelocity,    "%d",     "SeekVel=%-10s\n" ),
   ITEM(BATTVOLTAGE,      FALSE,  l_regs.m_BattVoltage,     "%d",     "Batt. Volts.=%-5s" ),
   ITEM(CURRENT,          FALSE,  l_regs.m_Current,         "%d",     "Power=%-12s" ),
   ITEM(TEMPERATURE,      FALSE,  l_regs.m_Temperature,     "%d",     "Temperature=%-6s\n" ),
   ITEM(PID_DEADBAND,     FALSE,  l_regs.m_PID.m_Deadband,  "%d",     "Deadband=%-9s" ),
   ITEM(PID_PGAIN,        FALSE,  l_regs.m_PID.m_PGain,     "%d",     "P-Gain=%-11s" ),
   ITEM(PID_IGAIN,        FALSE,  l_regs.m_PID.m_IGain,     "%d",     "I-Gain=%-11s" ),
   ITEM(PID_DGAIN,        FALSE,  l_regs.m_PID.m_DGain,     "%d",     "D-Gain=%-11s\n" ),
   ITEM(PWM_FREQ_DIVIDER, FALSE,  l_regs.m_PWM_FreqDivider, "%d",     "PWM freq dev=%-5s" ),
   ITEM(PWM_CW,           FALSE,  l_regs.m_PWM_CW,          "%d",     "PWM CW=%-11s" ),
   ITEM(PWM_CCW,          FALSE,  l_regs.m_PWM_CCW,         "%d",     "PWM CCW=%-10s\n" ),
   ITEM(MINSEEK,          FALSE,  l_regs.m_MinSeek,         "%d",     "Min Seek=%-9s" ),
   ITEM(MAXSEEK,          FALSE,  l_regs.m_MaxSeek,         "%d",     "Max Seek=%-9s" ),
   ITEM(REVERSESEEK,      FALSE,  l_regs.m_ReverseSeek,     "%d",     "Reverse Seek=%-5s\n" ),
   ITEM(ALERT_CURR_MAXLIMIT, TRUE,l_regs.m_Alert_MaxCurrent,"%d",     "Alert Max I=%-6s" ),
   ITEM(ALERT_VOLT_MINLIMIT, TRUE,l_regs.m_Alert_MinVoltage,"%d",     "Alert Min V=%-6s" ),
   ITEM(ALERT_VOLT_MAXLIMIT, TRUE,l_regs.m_Alert_MaxVoltage,"%d",     "Alert Max V=%-6s\n" ),
};

/*
 * Prototypes
 */
int_t init_I2CManager();
int_t init_OSI();
int_t dump_OpenServo(int_t nServoID, REGINDEXITEM *pindex, int_t nitems);
int_t run_OpenServo(int_t nServoID);

/*
 * Signal trapping
 */
bool_t l_bAbort=FALSE;
static void siginthandler(int sig)
{
   if(sig==SIGINT || sig==SIGTERM)
   {
      l_bAbort=TRUE;
   }
}

/*
 * Main
 */
int main(int argc, char *argv[])
{
   int rc=1;

/*
 * Basic check on args
 */
   if(argc<1 || argc>3 || 
      argc==3 && strcmp(argv[2],"-run")!=0)
   {
      fprintf(stderr,"Usage: test01 [device_ID] [-run]\n");
      fflush(stderr);
   } else
   {
      bool_t bRun=argc==3 && strcmp(argv[2],"-run")==0;

      signal(SIGINT,siginthandler);
      signal(SIGTERM,siginthandler);

/*
 * Attempt to initialise the I2C Manager, then list I2C buses and devices
 */
      rc=init_I2CManager();
      if(rc==I2CM_SUCCESS)
      {

/*
 * Attempt to initialise the OpenServo (software) interface, then list any OpenServos
 */
         rc=init_OSI();
         if(rc==OSI_SUCCESS)
         {

/*
 * If there is a command line argument, take it as specifying an OpenServo from the list
 * shown above, retrieve and display (to stdout) its status
 */
            if(argc>=2)
            {
               char tmpc;
               int_t nServoID;
               if(sscanf(argv[1],"%d %c",&nServoID,&tmpc)!=1)
               {
                  fprintf(stderr,"Argument syntax error, please specify an integer ID of a OpenServo listed above\n");
                  fflush(stderr);
                  rc=-1;
               } else
               {
                  rc=dump_OpenServo(nServoID,l_GenRegIndex,sizeof(l_GenRegIndex)/sizeof(l_GenRegIndex[0]));

/*
 * Run the servo until the user quits the application
 */
                  if(rc==OSI_SUCCESS && bRun)
                  {
                     rc=run_OpenServo(nServoID);
                  }
               }
            }
         }

/*
 * Close down I2C Manager
 */
         I2CM_Shutdown();
      }
   }
   return rc==0 ? 0 : 1;
}

/*******************************************************************************
 *
 * int_t init_I2CManager()
 *
 * Initialise I2C Manager, list buses and devices to stdout.
 */
int_t init_I2CManager()
{
   int_t rc=I2CM_Init();
   if(rc!=I2CM_SUCCESS)
   {
      fprintf(stderr,"Failed to initialise I2C Manager, error=%d\n",rc);
      fflush(stderr);
   } else
   {
      int_t nbuses=I2CM_GetBusCount();
      if(nbuses<=0)
      {
         fprintf(stderr,"No I2C buses found\n");
         fflush(stderr);
      } else
      {
         if(I2CM_GetDeviceCount()<=0)
         {
            fprintf(stderr,"No I2C devices found\n");
            fflush(stderr);
         } else
         {
            int_t ibus;
            char busname[32];
            rc=0;
            for(ibus=0; ibus<nbuses && rc==0; ibus++)
            {
               rc=I2CM_GetBusName(ibus,busname,sizeof(busname));
               if(rc!=I2CM_SUCCESS)
               {
                  fprintf(stderr,"Failed to retrieve name of bus %d, error=%d\n",ibus,rc);
                  fflush(stderr);
                  rc=-1;
               } else
               {
                  int_t ndev,idev;
                  fprintf(stdout,"Found I2C bus: \"%s\"\n",busname);
                  fprintf(stdout,"   ");
                  fflush(stdout);
                  ndev=I2CM_GetBusDeviceCount(ibus);
                  if(ndev<=0)
                  {
                     fprintf(stdout,"No devices found on this bus");
                     fflush(stdout);
                  } else
                  {
                     fprintf(stdout,"Devices:");
                     fflush(stdout);
                     for(idev=0; idev<ndev && rc==0; idev++)
                     {
                        int_t devaddr=I2CM_GetDeviceAddress(ibus,idev);
                        if(devaddr<0)
                        {
                           fprintf(stderr,"\nFailed to retrieve address of device %d, error=%d\n",idev,I2CM_GetLastError());
                           fflush(stderr);
                           rc=-1;
                        } else
                        {
                           fprintf(stdout,"%s0x%02X",idev==0 ? " " : ", ",devaddr);
                           fflush(stdout);
                        }
                     }
                     fprintf(stdout,"\n");
                  }
               }
            }
         }
      }
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t init_OSI()
 *
 * Initialise the OpenServo (software) Interface, display a list of OpenServos
 * that are available
 */
int_t init_OSI()
{
   int_t rc=OSI_Init(TRUE,TRUE);
   if(rc!=I2CM_SUCCESS)
   {
      fprintf(stderr,"Failed to initialise I2C Manager, error=%d\n",rc);
      fflush(stderr);
   } else
   {
      int_t nservos=OSI_GetDeviceCount();
      if(nservos<=0)
      {
         fprintf(stderr,"No OpenServos found\n");
         fflush(stderr);
      } else
      {
         int_t iservo;
         for(iservo=0; iservo<nservos && rc==0; iservo++)
         {
            OPENSERVO *posid=OSI_GetDeviceIDByPOS(iservo);
            if(posid==NULL)
            {
               fprintf(stderr,"Failed to get handle for servo %d, error=%d\n",iservo,OSI_GetLastError());
               fflush(stderr);
               rc=-1;
            } else
            {
               char busname[32];
               rc=I2CM_GetBusName(posid->m_nBus,busname,sizeof(busname));
               if(rc!=I2CM_SUCCESS)
               {
                  fprintf(stderr,"Failed to retrieve name of bus %d, error=%d\n",posid->m_nBus,rc);
                  fflush(stderr);
                  rc=-1;
               } else
               {
static const char *OSTYPES[]=
{
   "Error",
   "21_11",
   "21_BANK",
   "V3_11",
   "V3_BANK"
};
                  if(iservo==0)
                  {
                     fprintf(stdout,"OpenServo directory:\nID Type     Addr Bus               Vreg  R3/R4  R4/R6 Xtal kHz\n");
                  }
                  fprintf(stdout,"%2d %-8s 0x%02X %-16s  %4.2f %6d %6d %8d\n",
                     iservo,
                     posid->m_nDType==-1 ? "UNKNOWN" : OSTYPES[posid->m_nDType<1 || posid->m_nDType>=sizeof(OSTYPES)/sizeof(OSTYPES[0]) ? 0 : posid->m_nDType],
                     posid->m_nAddress,busname,
                     (double)posid->m_conf.m_VREG/1000.0,
                     posid->m_conf.m_R3R4,posid->m_conf.m_R4R6,
                     posid->m_conf.m_nClock);
                  fflush(stdout);
               }
            }
         }
      }
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t dump_OpenServo(int_t nServoID, REGINDEXITEM *pIndex, int_t nIndex)
 *
 * Dump the status (i.e. read and display the registers) of the specified OpenServo.
 */
int_t dump_OpenServo(int_t nServoID, REGINDEXITEM *pIndex, int_t nIndex)
{
   int rc=-1;

/*
 * Initialise, open the servo, read the registers, then close
 *
 * NOTE: Open/Close usage may be disabled for testing and development purposes.
 */
   OPENSERVO *posid=OSI_GetDeviceIDByPOS(nServoID);
   if(posid==NULL)
   {
      fprintf(stderr,"Failed to get handle for servo %d, error=%d\n",nServoID,OSI_GetLastError());
      fflush(stderr);
      rc=-1;
   } else
   {
      rc=OSI_Open(posid,0);
      if(rc==OSI_SUCCESS)
      {
         int_t i;
         for(i=0; i<nIndex; i++)
         {
            memset(pIndex[i].m_pValue,0xFF,pIndex[i].m_nSize);
            pIndex[i].m_rc=OSI_ReadRegister(posid,pIndex[i].m_nOSID,pIndex[i].m_pValue,pIndex[i].m_nSize);
            if(pIndex[i].m_rc!=OSI_SUCCESS && pIndex[i].m_rc!=OSI_ERR_NOREGISTER)
            {
               rc=-1;
            }
         }
         if(OSI_Close(posid)!=OSI_SUCCESS) /* NOTE: Need to preserve non-zero rc from above loop */
         {
            rc=-1;
         }

/*
 * Display what has been read
 */
         for(i=0; i<nIndex; i++)
         {
            char buffer[32];
            if(pIndex[i].m_rc==OSI_ERR_NOREGISTER)
            {
               strcpy(buffer,"NOREG");
            } else
            {
               if(pIndex[i].m_rc!=OSI_SUCCESS)
               {
                  strcpy(buffer,"ERROR");
               } else
               {
                  int_t value;
                  if(pIndex[i].m_bIsSigned)
                  {
                     value=pIndex[i].m_nSize==1 ? (int_t)*(char *)(pIndex[i].m_pValue) : (int_t)*((int16_t *)pIndex[i].m_pValue);
                  } else
                  {
                     value=pIndex[i].m_nSize==1 ? (int_t)*(byte_t *)(pIndex[i].m_pValue) : (int_t)*((uint16_t *)pIndex[i].m_pValue);
                  }
                  _snprintf(buffer,sizeof(buffer)-1,pIndex[i].m_pValFormat,
                     value);
                  buffer[sizeof(buffer)-1]='\0';
               }
            }
            fprintf(stdout,pIndex[i].m_pOutFormat,buffer);
            fflush(stdout);
         }
      }
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t run_OpenServo(int_t nServoID)
 *
 * Run the specified servo between 150 to 850, with some simple diagnostic output.
 */
int_t run_OpenServo(int_t nServoID)
{
   int_t rc=-1;

/*
 * NOTE: Open/Close usage may be disabled for testing and development purposes.
 */
   OPENSERVO *posid=OSI_GetDeviceIDByPOS(nServoID);
   if(posid==NULL)
   {
      fprintf(stderr,"Failed to get handle for servo %d, error=%d\n",nServoID,OSI_GetLastError());
      fflush(stderr);
      rc=-1;
   } else
   {

/*
 * Simple setup...
 */
      OPENSERVOPIDDATA pid;
      if(OSI_Command(posid,OSI_CMDID_PWM_DISABLE)!=OSI_SUCCESS ||
         OSI_SetMinSeek(posid,100)!=OSI_SUCCESS ||
         OSI_SetMaxSeek(posid,900)!=OSI_SUCCESS ||
         OSI_GetPID(posid, &pid)!=OSI_SUCCESS)
      {
         fprintf(stderr,"Setup error for servo %d, error=%d\n",nServoID,OSI_GetLastError());
         fflush(stderr);
      } else
      {
         if(pid.m_DGain==0 || pid.m_PGain==0)
         {
            pid.m_PGain=1500;
            pid.m_DGain=2000;
         }
         if(OSI_SetPID(posid, &pid)!=OSI_SUCCESS ||
            OSI_SetSeek(posid,150)!=OSI_SUCCESS ||
            OSI_Command(posid,OSI_CMDID_PWM_ENABLE)!=OSI_SUCCESS)
         {
            fprintf(stderr,"Setup error for servo %d, error=%d\n",nServoID,OSI_GetLastError());
            fflush(stderr);
         } else
         {
            bool_t bDone=FALSE;
#define ITEM(ID,SIGNED,VARIABLE,FORMAT,OUTFORMAT) { OSI_REGID_##ID, SIGNED, sizeof(VARIABLE), & VARIABLE, -1, FORMAT, OUTFORMAT }
REGINDEXITEM ri[]=
{
   ITEM(TIMER,            FALSE,  l_regs.m_Timer,           "%u",      "timer=%-5s" ),
   ITEM(POSITION,         FALSE,  l_regs.m_Position,        "%d",      " pos=%-3s" ),
   ITEM(VELOCITY,          TRUE,  l_regs.m_Velocity,        "%d",      " vel=%-3s" ),
   ITEM(SEEK,             FALSE,  l_regs.m_Seek,            "%d",      " seek=%-3s" ),
   ITEM(BATTVOLTAGE,      FALSE,  l_regs.m_BattVoltage,     "%d",      " V=%3s" ),
   ITEM(CURRENT,          FALSE,  l_regs.m_Current,         "%d",      " I=%4s" ),
   ITEM(TEMPERATURE,      FALSE,  l_regs.m_Temperature,     "%d",      " T=%3s" ),
   ITEM(PWM_CW,           FALSE,  l_regs.m_PWM_CW,          "%d",      " PWM=%s" ),
   ITEM(PWM_CCW,          FALSE,  l_regs.m_PWM_CCW,         "%d",      ", %s  \r" ),
};
            do
            {
               rc=dump_OpenServo(nServoID,ri,sizeof(ri)/sizeof(ri[0]));
               if(rc==OSI_SUCCESS)
               {
                  if(l_regs.m_Position<151)
                  {
                     rc=OSI_SetSeek(posid,850);
                  } else
                  {
                     if(l_regs.m_Position>849)
                     {
                        rc=OSI_SetSeek(posid,150);
                     }
                  }
               }
               if(!l_bAbort)
               {
// TODO: More recent firmware may not require the command to be sent
//                  rc=OSI_Command(posid,OSI_CMDID_BATTVOLTAGE_RESAMPLE);
                  Sleep(50);
               }
            } while(!l_bAbort);
         }
      }
   }
   return rc;
}
