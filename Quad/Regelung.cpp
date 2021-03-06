#include "Regelung.h"
#include "ch.h"
#include "hal.h"
#include "Lage.h"
#include "Fernsteuerung.h"
#include "Motoren.h"
#include "tm.h"
float inNickIstLage;
float inNickIstV;
float inNickSollLage;
float inRollIstLage;
float inRollIstV;
float inRollSollLage;
float inSchub;
float inYawIstLage;
float inYawIstV;
float inYawSollLage;
float outMotor1;
float outMotor2;
float outMotor3;
float outMotor4;	 
float Schub_Offset = 0;


float v_Nick_tp1 = 0;   	
float v_Nick_tp1_alt = 0;
float di_Nick = 0;
float pi_Nick = 0;
float ii_Nick = -10;
float ia_Nick = 0;
float pa_Nick = 0;
float ei_Nick = 0;
float ei_Nick_alt = 0;
float ea_Nick = 0;
float Soll_v_Nick = 0;
float aNick;

float v_Roll_tp1 = 0;   	
float v_Roll_tp1_alt = 0;
float di_Roll = 0;
float pi_Roll = 0;
float ii_Roll = 0;
float ia_Roll = 0;
float pa_Roll = 0;
float ei_Roll = 0;
float ei_Roll_alt = 0;
float ea_Roll = 0;
float Soll_v_Roll = 0;
float aRoll; 

float v_Yaw_tp1 = 0;   	
float v_Yaw_tp1_alt = 0;
float di_Yaw = 0;
float pi_Yaw = 0;
float ii_Yaw = 0;
float ia_Yaw = 0;
float pa_Yaw = 0;
float ei_Yaw = 0;
float ei_Yaw_alt = 0;
float ea_Yaw = 0;
float Soll_v_Yaw = 0;
float aYaw;
volatile unsigned short tmp111;
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "Datalogger.h"
#include "ff.h"

	static FIL Fil_regelung;			/* File object */
	FRESULT rc_datalog;				/* Result code */
	
bool_t datalog_regelung_opened = 0;
bool_t datalog_regelung_syncing = 0;
static TimeMeasurement regelungdatalogsync_tmup;
	/*
 * Working area for LageSync
 */
static WORKING_AREA(RegelungSyncThreadWorkingArea, 2048);
/*
 * LageSync
 */
static msg_t RegelungSyncthread(void *arg) {
 
  systime_t time = chTimeNow();     // Tnow
  while (TRUE) 
	{
		if(Datalogger_ready() && !datalog_regelung_opened)
		{
			//time +=  MS2ST(500);           // Next deadline
			//rc = f_mkfs(0,0,0);
				rc_datalog = f_open(&Fil_regelung, ("QuadRege.TXT"), FA_WRITE | FA_CREATE_ALWAYS);
				if(rc_datalog != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: f_open() failed %d\r\n", rc_datalog);
					chThdSleepMilliseconds(10);
				}	
/*				
				rc_datalog = f_sync(&Fil_regelung);
				if(rc_datalog != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: f_sync() failed %d\r\n", rc_datalog);
				}	
				*/
				f_printf(&Fil_regelung, "Time_regelung;inSchub;inNickSollLage;inRollSollLage;inYawSollLage;inNickIstLage;inRollIstLage;inYawIstLage;inNickIstV;inRollIstV;inYawIstV;ea_Nick;ea_Roll;ea_Yaw;ia_Nick;ia_Roll;ia_Yaw;Soll_v_Nick;Soll_v_Roll;Soll_v_Yaw;ii_Nick;ii_Roll;ii_Yaw;pi_Nick;pi_Roll;pi_Yaw;di_Nick;di_Roll;di_Yaw;v_Nick_tp1;v_Roll_tp1;v_Yaw_tp1;aNick;aRoll;aYaw;outMotor1;outMotor2;outMotor3;outMotor4\r\n");
				//f_printf(&Fil_regelung, "Time_reg");
				rc_datalog = f_sync(&Fil_regelung);	
				if(rc_datalog != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: initial f_sync() failed %d\r\n", rc_datalog);
					chThdSleepMilliseconds(10);
				}	
				else
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: opened successfull\r\n");
					chThdSleepMilliseconds(10);
					datalog_regelung_opened = TRUE;	
				}
				chThdSleepMilliseconds(10);
		}
		else if (Datalogger_ready() && datalog_regelung_opened)
		{
			datalog_regelung_syncing = 1;
			rc_datalog = f_sync(&Fil_regelung);
				if(rc_datalog != FR_OK)
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: datalog f_sync() failed %d\r\n", rc_datalog);
				}	
				else
				{
					chprintf((BaseChannel *) &SD2, "SD Quadregelung.TXT: datalog f_sync() succeeded\r\n");
				}
			datalog_regelung_syncing = 0;
			chThdSleepMilliseconds(1000);
		}
		else
		{
			chThdSleepMilliseconds(10);
		}		
  }
}


void datalog_regelung(void)
{
	uint32_t system_time;
uint32_t system_time_after_printf;
		int worst, last, best;
		
	
		if(Datalogger_ready() && datalog_regelung_opened && datalog_regelung_syncing == 0)
		{
			tmStartMeasurement(&regelungdatalogsync_tmup);
			system_time = chTimeNow();
			f_printf(&Fil_regelung, "%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\r\n",system_time,
						(int)(inSchub*100),
						(int)(inNickSollLage*100),
						(int)(inRollSollLage*100),
						(int)(inYawSollLage*100),
						(int)(inNickIstLage*100),
						(int)(inRollIstLage*100),
						(int)(inYawIstLage*100),
						(int)(inNickIstV*100),
						(int)(inRollIstV*100),
						(int)(inYawIstV*100),
						(int)(ea_Nick*100),
						(int)(ea_Roll*100),
						(int)(ea_Yaw*100),
						(int)(ia_Nick*100),
						(int)(ia_Roll*100),
						(int)(ia_Yaw*100),
						(int)(Soll_v_Nick*100),
						(int)(Soll_v_Roll*100),
						(int)(Soll_v_Yaw*100),
						(int)(ii_Nick*100),
						(int)(ii_Roll*100),
						(int)(ii_Yaw*100),
						(int)(pi_Nick*100),
						(int)(pi_Roll*100),
						(int)(pi_Yaw*100),
						(int)(di_Nick*100),
						(int)(di_Roll*100),
						(int)(di_Yaw*100),
						(int)(v_Nick_tp1*100),
						(int)(v_Roll_tp1*100),
						(int)(v_Yaw_tp1*100),
						(int)(aNick*100),
						(int)(aRoll*100),
						(int)(aYaw*100),
						(int)(outMotor1),
						(int)(outMotor2),
						(int)(outMotor3),
						(int)(outMotor4)
					);
					system_time_after_printf = chTimeNow();
			//rc_datalog = f_sync(&Fil_regelung);
				 tmStopMeasurement(&regelungdatalogsync_tmup);
	 best = RTT2MS(regelungdatalogsync_tmup.best);
	last = RTT2MS(regelungdatalogsync_tmup.last);
	worst = RTT2MS(regelungdatalogsync_tmup.worst);
	
	chprintf((BaseChannel *) &SD2, "Data_t:%d, After_pritf_t:%d - RegelungLog L:%d B:%d W:%d\r\n",system_time, system_time_after_printf,last, best,worst);
		}
}

void Regelung(void)
{
	/// Werte �bernehmen

	inSchub = getSchub();
	
	inNickSollLage = 0;
	inRollSollLage = getRoll();
	inYawSollLage  = 0;
	
	inYawIstLage = getEuler_yaw();
	inNickIstLage = getEuler_nick();
	inRollIstLage = getEuler_roll();
		 
	inRollIstV = getRate_roll();
	inNickIstV = getRate_nick();
	inYawIstV = getRate_yaw();

	/////////////////////////// Nick-Regler berechnen //////////////////////////////////////////
   //�u�erer Regler
   ea_Nick = (inNickSollLage) - (inNickIstLage+0.08*inNickIstV);  // Eingang in den �u�eren Regler
   if(inSchub > 0.1 && inSchub <=1 && inNickIstLage < 0.2 && inNickIstLage > -0.2)
   	ia_Nick = 0.01 * ea_Nick + ia_Nick;	//I-Anteil nur nahe der Nulllage ver�ndern  
   //Soll_v_Nick = ea_Nick*1 + ia_Nick*0.02; //Sollgeschwindigkeit des �u�eren Reglers
   //Soll_v_Nick = inNickSollLage*2.F;
   Soll_v_Nick = 0;
   //innerer Regler
   v_Nick_tp1 = 0.9*v_Nick_tp1 + 0.1*inNickIstV; // Tiefpass-gefilterter Gyrowert
   ei_Nick= Soll_v_Nick - v_Nick_tp1;	//Eingang in die innere Regelung
   pi_Nick = ei_Nick * 0.07;	//p-Anteil

   if(inSchub > 0.1 && inSchub <=1 /*&& inNickIstLage < 0.5 && inNickIstLage > -0.5*/)
   	ii_Nick = 0.08 * ei_Nick + ii_Nick;	//I-Anteil nur nahe der Nulllage ver�ndern 

   di_Nick = (ei_Nick - ei_Nick_alt)*10; //d-Anteil  
   if (di_Nick > 1.5)
	   di_Nick = 1.5;
   else if(di_Nick < -1.5)
	   di_Nick = -1.5;	  //Saturierung des D-Anteils

   aNick = (pi_Nick + (ii_Nick)*0.03 + di_Nick)*567; //Ausgang des inneren Reglers	 

	/////////////////////////// Roll-Regler berechnen ////////////////////////////////////////// 

   /*ea_Roll = (inRollSollLage) - (inRollIstLage+0.08*inRollIstV);  // Eingang in den �u�eren Regler
   if(inSchub > 0.1 && inSchub <=1 && inRollIstLage < 0.2 && inRollIstLage > -0.2)
   	ia_Roll = 0.01 * ea_Roll + ia_Roll;	//I-Anteil nur nahe der Nulllage ver�ndern  
   Soll_v_Roll = ea_Roll*2 + ia_Roll*0.02; //Sollgeschwindigkeit des �u�eren Reglers
   //Soll_v_Roll = inRollSollLage;

   //innerer Regler
   v_Roll_tp1 = 0.95*v_Roll_tp1 + 0.05*inRollIstV; // Tiefpass-gefilterter Gyrowert
   ei_Roll= Soll_v_Roll - v_Roll_tp1;	//Eingang in die innere Regelung
   pi_Roll = ei_Roll * 0.5;	//p-Anteil
   if(inSchub > 0.1 && inSchub <=1 && inRollIstLage < 0.2 && inRollIstLage > -0.2)
   	ii_Roll = 0.1 * ei_Roll + ii_Roll;	//I-Anteil nur nahe der Nulllage ver�ndern 

   di_Roll = (ei_Roll - ei_Roll_alt)*20; //d-Anteil  
   if (di_Roll > 1.5)
	   di_Roll = 1.5;
   else if(di_Roll < -1.5)
	   di_Roll = -1.5;	  //Saturierung des D-Anteils

   aRoll = (pi_Roll + (ii_Roll+37)*0.02 + di_Roll)*567; //Ausgang des inneren Reglers		 */

	ea_Roll = (inRollSollLage) - (inRollIstLage+0.08*inRollIstV);  // Eingang in den �u�eren Regler
   if(inSchub > 0.1 && inSchub <=1 && inRollIstLage < 0.2 && inRollIstLage > -0.2)
   	ia_Roll = 0.01 * ea_Roll + ia_Roll;	//I-Anteil nur nahe der Nulllage ver�ndern  
   Soll_v_Roll = ea_Roll*2 + ia_Roll*0.02; //Sollgeschwindigkeit des �u�eren Reglers
   Soll_v_Roll = inRollSollLage;

   //innerer Regler
   v_Roll_tp1 = 0.95*v_Roll_tp1 + 0.05*inRollIstV; // Tiefpass-gefilterter Gyrowert
   ei_Roll= Soll_v_Roll - v_Roll_tp1;	//Eingang in die innere Regelung
   pi_Roll = ei_Roll * 0.1;	//p-Anteil
   if(inSchub > 0.1 && inSchub <=1 /*&& inRollIstLage < 0.2 && inRollIstLage > -0.2*/)
   	ii_Roll = 0.01 * ei_Roll + ii_Roll;	//I-Anteil nur nahe der Nulllage ver�ndern 


   di_Roll = (ei_Roll - ei_Roll_alt)*10; //d-Anteil  
   if (di_Roll > 1.5)
	   di_Roll = 1.5;
   else if(di_Roll < -1.5)
	   di_Roll = -1.5;	  //Saturierung des D-Anteils

   aRoll = (pi_Roll + (ii_Roll)*0.005 + di_Roll)*567; //Ausgang des inneren Reglers

	/////////////////////////// Yaw-Regler berechnen ////////////////////////////////////////// 

   ea_Yaw = (inYawSollLage) - (inYawIstLage+0.08*inYawIstV);  // Eingang in den �u�eren Regler
   if(inSchub > 0.1 && inSchub <=1 && inYawIstLage < 0.2 && inYawIstLage > -0.2)
   	ia_Yaw = 0.01 * ea_Yaw + ia_Yaw;	//I-Anteil nur nahe der Nulllage ver�ndern  
   Soll_v_Yaw = ea_Yaw*0.5 + ia_Yaw*0.002; //Sollgeschwindigkeit des �u�eren Reglers
   //Soll_v_Yaw = inYawSollLage;

   //innerer Regler
   v_Yaw_tp1 = 0.99*v_Yaw_tp1 + 0.01*inYawIstV; // Tiefpass-gefilterter Gyrowert
   ei_Yaw= Soll_v_Yaw - v_Yaw_tp1;	//Eingang in die innere Regelung
   pi_Yaw = ei_Yaw * 0.4;	//p-Anteil
   if(inSchub > 0.1 && inSchub <=1 && inYawIstLage < 0.2 && inYawIstLage > -0.2)
   	ii_Yaw = 0.01 * ei_Yaw + ii_Yaw;	//I-Anteil nur nahe der Nulllage ver�ndern 

   di_Yaw = (ei_Yaw - ei_Yaw_alt)*20; //d-Anteil  
   if (di_Yaw > 1.5)
	   di_Yaw = 1.5;
   else if(di_Yaw < -1.5)
	   di_Yaw = -1.5;	  //Saturierung des D-Anteils

   aYaw = (pi_Yaw + (ii_Yaw)*0.1 + di_Yaw)*567; //Ausgang des inneren Reglers
   
   if (aYaw > 2000.F)
	   aYaw = 2000.F;
   else if(aYaw < -2000.F)
	   aYaw = -2000.F;	  //Saturierung des Yaw-Anteils
  	
	/////////////////////////// Motorwerte setzen //////////////////////////////////////////  
   if(inSchub > 0.1 && inSchub <=1)
   {
     Schub_Offset = 6000 * inSchub;
	 aNick = 0.F;	
	 aYaw = 0.F;

     outMotor3 = Schub_Offset - aRoll + aYaw;
     //outMotor1 = Schub_Offset + aNick - aYaw;
     //outMotor4 = Schub_Offset - aNick - aYaw; 
  	 outMotor1 = 0.F;
 	 outMotor4 = 0.F;
     outMotor2 = Schub_Offset + aRoll + aYaw;
   }
   else 
   {					
  		outMotor1 = 0.F;
  		outMotor2 = 0.F;
		outMotor3 = 0.F;
		outMotor4 = 0.F;
   }						
   
	/////////////////////////// Motorwerte saturieren und �bergeben //////////////////////// 
   if (outMotor1 > 6800.F) 	setMotor_1(6800.F);
   else if(outMotor1 < 0.F) setMotor_1(0.F);
	 else setMotor_1(outMotor1);

   if (outMotor2 > 6800.F)  setMotor_2(6800.F);
   else if(outMotor2 < 0.F) setMotor_2(0.F);
	 else setMotor_2(outMotor2);
	 
   if (outMotor3 > 6800.F)  setMotor_3(6800.F);
   else if(outMotor3 < 0.F) setMotor_3(0.F);
	 else setMotor_3(outMotor3);
	 
   if (outMotor4 > 6800.F)  setMotor_4(6800.F);
   else if(outMotor4 < 0.F) setMotor_4(0.F);	
	 else setMotor_4(outMotor4);	

	/////////////////////////// Alte Werte merken ////////////////////////////////////////// 

   v_Nick_tp1_alt = v_Nick_tp1;
   ei_Nick_alt = ei_Nick;

   v_Roll_tp1_alt = v_Roll_tp1;
   ei_Roll_alt = ei_Roll;

   v_Yaw_tp1_alt = v_Yaw_tp1;
   ei_Yaw_alt = ei_Yaw;

   datalog_regelung();
}
/*
 * Working area for Regelung
 */
static WORKING_AREA(RegelungThreadWorkingArea, 2048);
//int dennis=0; 
/*
 * Regelungsthread
 */
systime_t delta_t;
static msg_t Regelungsthread(void *arg) {
 
  systime_t time = chTimeNow();     // Tnow
  while (TRUE) 
	{
		delta_t = MS2ST(10);
		time += delta_t;            // Next deadline
		Regelung();
		chThdSleepUntil(time);
  }
}




void setup_Regelung()
{
	//Regelungsthread anlegen
	//Thread *tp = chThdCreateFromHeap(NULL, THD_WA_SIZE(128), NORMALPRIO+1, Regelungsthread, NULL);
  //	if (tp == NULL)
  //  	chSysHalt();    /* Memory exausted. */
	tmObjectInit(&regelungdatalogsync_tmup);
	chThdCreateStatic(RegelungSyncThreadWorkingArea, sizeof(RegelungSyncThreadWorkingArea), NORMALPRIO, RegelungSyncthread, NULL);
	chThdCreateStatic(RegelungThreadWorkingArea, sizeof(RegelungThreadWorkingArea), ABSPRIO, Regelungsthread, NULL);
}