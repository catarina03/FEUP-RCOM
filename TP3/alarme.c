#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "alarme.h"

static int alarmCounter = 0;

static int alarmFlag = 0;



void sigalarm_handler(int signo){
  
  
    alarmFlag=1;
    alarmCounter++;
    printf("Alarm ringing\n");
  

}

int getAlarmFlag(){
  return alarmFlag;
}

int getAlarmCounter(){
  return alarmCounter;
}

void setAlarmFlag(int flag){
  alarmFlag=flag;
}

void setAlarmCounter(int counter){
  alarmCounter=counter;
}

void setAlarm(){
  struct sigaction act_alarm;
  act_alarm.sa_handler = sigalarm_handler;
  sigemptyset(&act_alarm.sa_mask);
  act_alarm.sa_flags = 0;
  
  if (sigaction(SIGALRM,&act_alarm,NULL) < 0)  {        
      fprintf(stderr,"Unable to install SIGALARM handler\n");        
      exit(1);  
  } 
}


void resetAlarm(){
  alarm(0);
  alarmFlag=0;
  alarmCounter=0;
}