/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "lis302dl.h"
#include "chprintf.h"
#include "Lage.h"
#include "Fernsteuerung.h"
#include "Motoren.h"
#include "Regelung.h"
#include "Datalogger.h"
#include "ExternalInterrupt.h"
#include "ff.h"


	static FIL Fil_Main;			/* File object */
	FRESULT rc_main;				/* Result code */

bool_t datalog_main_opened = 0;

void datalog(void)
{
		float nick, roll, yaw;
	uint32_t system_time;
	 		//update_IMU();	 //Ersetzen durch Interrupt Handler!!!!!!
			nick = getEuler_nick();
			roll = getEuler_roll();
			yaw = getEuler_yaw();
		
		if(Datalogger_ready() && !datalog_main_opened)
		{
				//rc_main = f_mkfs(0,0,0);
				rc_main = f_open(&Fil_Main, "QuadMain.TXT", FA_WRITE | FA_CREATE_ALWAYS);
				if(rc_main != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD QuadMain.TXT: f_open() failed %d\r\n", rc_main);
					return;
				}	
				//rc_main = f_printf(&Fil, "moin\r\n");	 
				rc_main = f_sync(&Fil_Main);
				if(rc_main != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD QuadMain.TXT: f_sync() failed %d\r\n", rc_main);
					return;
				}	
				chprintf((BaseChannel *) &SD2, "SD QuadMain.TXT: opened successfull\r\n");
				f_printf(&Fil_Main, "Time_Main; Nick_Main; Roll_Main; Yaw_Main\r\n");
				f_sync(&Fil_Main);
				datalog_main_opened = TRUE;
		}
		if(Datalogger_ready() && datalog_main_opened)
		{
			system_time = chTimeNow();
			f_printf(&Fil_Main, "%d;%d;%d;%d\r\n",system_time,(int)(nick*100),(int)(roll*100),(int)(yaw*100));
			rc_main = f_sync(&Fil_Main);
		}
}


/*
 * Application entry point.
 */
int main(void) 
{
	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/
	halInit();	   
	chSysInit();
	
	/*
	* Activates the serial driver 2 using the driver default configuration.
	* PA2(TX) and PA3(RX) are routed to USART2.
	*/
	sdStart(&SD2, NULL);
	palSetPadMode(GPIOD, 5, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 6, PAL_MODE_ALTERNATE(7));
	
	setup_IMU();
	setup_ExternalInterrupt();
	setup_Fernsteuerung();
	setup_Motoren();
	setup_Regelung();
	setup_Datalogger(); 
	/*
	* Normal main() thread activity, in this demo it does nothing except
	* sleeping in a loop and check the button state, when the button is
	* pressed the test procedure is launched with output on the serial
	* driver 2.
	*/
	while (TRUE) 
	{
		update_IMU();
		//datalog();
		//chThdSleepMilliseconds(10);
  }
}
