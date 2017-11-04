#include "clkUtilities.h"
#include "queueUtilities.h"
#include <unistd.h>
#include <queue>
//#include <bits/stdc++.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <stdio.h>
#include <sys/wait.h>
#include <sstream>
#include<stdlib.h>
#include "statistics.h"
using namespace std;


#define _started 0
#define _resumed 1
#define _stopped 2
#define _finished 3
#define state_message_received 0
#define state_last_message_sent 1



class pcb
{
   public:
   int pid; //id as given by the operating system
   int arrivalTime;
   //int executionTime;
   int remainingTime;
   int id; //id as indicated in the file 
   int state;   //0 running for first time
                //1 resumed
                //2 stopped
                //3 finished
   int runtime;
   int priority;
   int totalwait;
   int laststopTime;
   int executionStart;
   int finishTime;

   pcb()
   {
    totalwait=0;
   }

   void setParameters(int id,int arrivaltime,int runtime,int priority)
   {
    this->id=id;
    this->arrivalTime=arrivaltime;
    this->runtime=runtime;
    this->priority=priority;
    this->remainingTime=runtime;
    laststopTime=0;
   }
   bool operator < (const pcb&  pd) const { return priority>pd.priority; }
   void updatepcb(int newstate,int curtime)
   {

    //update the state of the currently running process
    state=newstate;
    if(newstate==0) //ready the process has just started
    {
      executionStart=curtime;
      totalwait+=(executionStart-arrivalTime);
      printf( "At time %d process %d with pid= %d %s arr %d  total %d remain %d  \n",executionStart,id,pid,"started",arrivalTime,runtime,remainingTime);
    }

    else if(newstate==1) //resumed process
    {
      executionStart=curtime;
      totalwait+=(executionStart-laststopTime);
      printf( "At time %d process %d with pid= %d %s arr %d total %d remain %d \n",executionStart,id,pid,"resumed",arrivalTime,runtime,remainingTime);

    }
    else if(newstate==2) //stopped process
    {
       laststopTime=curtime;

       remainingTime-=(curtime-executionStart);

       printf( "At time %d process %d with pid= %d %s arr %d total %d remain %d and laststopTime= %d \n",curtime,id,pid,"stopped",arrivalTime,runtime,remainingTime,laststopTime);
    }
    else //finished
    {
       finishTime=curtime;
       remainingTime=0;
       printf("At time %d process %d with pid= %d %s arr %d total %d remain %d \n",finishTime,id,pid,"finished",arrivalTime,runtime,remainingTime);
    }
    
   }



};

class logger 
{
 FILE * fptr;
 const char* fname;

public:

    logger(const char* fname)
    {
         this->fname=fname;
         printf("%s\n",this->fname);
         fptr=fopen(fname,"w");

    }
    
    void logProcess(pcb process,int curtime)
    {
       // int pid,arrtime,runtime,remain,wait;
         int wait;
         double wta;
         int ta;

          printf("curtime: %d \n",curtime);

         const char * states[4]={"started","resumed","stopped","finished"};
         switch(process.state)
         {
            case 0:
            {
                //state="started";
                
                wait=process.executionStart-process.arrivalTime;
                break;
            }
            case 1:
            {
                //state="resumed";

                 wait=process.executionStart-process.laststopTime;
                break;
            }
            case 2:
            {
                //state="stopped";
                wait=0; //TODO: REVISE THAT CONDITION
                break;
            }
            case 3:
            {
                //state="finished";
                wait=process.totalwait;
                ta=process.finishTime-process.arrivalTime;
                wta=double(ta)/process.runtime;
                break;
            }
            default:
             break;
         }
       // fptr=fopen(fname,"a");
         if(fptr!=NULL)
         {
            if(process.state==_finished)

               { //rewind(fptr);
                 fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f \n",curtime,process.id,states[process.state],process.arrivalTime,process.runtime,process.remainingTime,wait,ta,wta);
//fclose(fptr);
               }
            else //if(fptr!=NULL)
            {  //rewind(fptr);
              fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d \n",curtime,process.id,states[process.state],process.arrivalTime,process.runtime,process.remainingTime,wait);
 //fclose(fptr);
            }
        }
        else
        {
          printf("Failure to open log file\n");
        }

    }

    ~logger()
    {
          fclose(fptr);
          printf("closed el bta3 el file\n");
    }
};



logger schedulerLogger("scheduler.log");
StatisticsReport CPUStatisticsReport("scheduler.perf");

struct processData pD;
 priority_queue <pcb> processTable;

 pcb runningprocess;
 bool running_process=false;
    struct processData current_running;
    priority_queue<processData>pq;

    struct sigaction sa;
void my_sigchld_handler(int sig)
{
    pid_t p;
    int status;
   // raise(SIGCONT);
   //kill(getpid(),SIGCONT);
  //printf("child is dead \n");
    while ((p=waitpid(-1, &status, WNOHANG)) != -1)
    {
       /* Handle the death of pid p */
     
     // current_running.stopTime= getClk();
     
     //pD=current_running;
//do some calculation and printing 
      if(p!=0)
      {running_process=false;
        runningprocess.updatepcb(_finished,getClk()); 
        double ta=runningprocess.finishTime-runningprocess.arrivalTime;
        double wta=double(ta)/runningprocess.runtime;
        CPUStatisticsReport.addProcessUsagedata(wta,runningprocess.totalwait,runningprocess.runtime);
        schedulerLogger.logProcess(runningprocess,getClk());
      
}
     else {  printf("child is waiting /resumed %d\n",p);}
       break;
      

    }
}

int startTime=1000;

void SigIntHandler(int sig)
{

  //int msg=Recmsg(pD);
 // printf("received a msg form pg %d\n",msg);
    startTime=getClk();
    printf("start time %d\n",startTime);
    signal(SIGILL, SIG_IGN);
  

}



int main(int argc, char* argv[]) {
    initQueue(false);
    initClk();
    printf("my PID %d\n",getpid());
   

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_sigchld_handler;

    sigaction(SIGCHLD, &sa, NULL);
    signal(SIGILL,SigIntHandler);

    
   
    

  
  int y=Recmsg(pD);
  while(y<=0)
  {     int x= getClk();
     // if(running_process){printf("killing my son id %d\n",current_running.id);kill(current_running.pid,SIGSTOP);}
     y=Recmsg(pD);
      while(y==0)
	{  x= getClk();
	  printf("current received data %d priority %d\n",pD.id,pD.priority);
       
	  pcb chosenprocess;
          chosenprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);
          processTable.push(chosenprocess);
               
	 // pq.push(pD);
          printf("waiting in queue\n");
	  y=Recmsg(pD);

		
     }

    if(!running_process&&!processTable.empty())
      {    x= getClk();
           runningprocess=processTable.top();
          //printf("new process runnibg\n");
          processTable.pop(); 
          running_process=true;
            
                 int id=fork();
                    if(id==0)
                        {   stringstream strs;
			  strs << runningprocess.runtime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
				     perror("execl() failure!\n\n"); }
			else 
				{  
				  
                            runningprocess.pid=id;
                            runningprocess.updatepcb(_started,getClk()); 
                                   schedulerLogger.logProcess(runningprocess,getClk());
                                   //  printf("running child process with id  %d\n",current_running.id);
				}
		
      }
  if(running_process)
     sleep(runningprocess.runtime);
  //if(running_process)
  //{ printf("Resuming my son id %d\n",current_running.id);kill(current_running.pid,SIGCONT); }
 //  kill(getpid(),SIGSTOP);
   // sleep(1);
   //printf(" clck%d\n",x); 
  
   
   }
   

  while(!processTable.empty())
    {  int x= getClk();
        printf("im in loop with clck%d\n",x);  
        
       
	if(!running_process)
            {     runningprocess=processTable.top();
          processTable.pop(); 
          //running_process=true; 
                 running_process=true;
                 int id=fork();
                     if(id==0)
                             {  stringstream strs;
			  strs << runningprocess.runtime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
				     perror("execl() failure!\n\n");
		        	 }
			else  
				{  
				  runningprocess.pid=id;
                            runningprocess.updatepcb(_started,getClk()); 
                                   schedulerLogger.logProcess(runningprocess,getClk());
                                   // printf("running child process with id  %d\n",current_running.id);
				}
		
            }
   // kill(getpid(),SIGSTOP);
   if(running_process)
     sleep(runningprocess.runtime);
    

    } 

while(running_process) {//kill(getpid(),SIGSTOP);
     int x= getClk();
     int remin=runningprocess.remainingTime-(getClk()-runningprocess.executionStart);
   sleep(remin);}
 //schedulerLogger.close();
   int x= getClk();
              CPUStatisticsReport.setFinishTime(getClk());
             CPUStatisticsReport.setStarttime(startTime);
             CPUStatisticsReport.GenerateStatisticsReport();

 printf("at end clck%d\n",x); 

  

    return 0;
    
}
