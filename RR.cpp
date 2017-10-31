#include "clkUtilities.h"
#include "queueUtilities.h"
#include <unistd.h>
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <stdio.h>
#include <sys/wait.h>
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
     // printf( "At time %d process %d with pid= %d %s arr %d  total %d remain %d  \n",executionStart,id,pid,"started",arrivalTime,runtime,remainingTime);
    }

    else if(newstate==1) //resumed process
    {
      executionStart=curtime;
      totalwait+=(executionStart-laststopTime);
      //printf( "At time %d process %d with pid= %d %s arr %d total %d remain %d \n",executionStart,id,pid,"resumed",arrivalTime,runtime,remainingTime);

    }
    else if(newstate==2) //stopped process
    {
       laststopTime=curtime;

       remainingTime-=(curtime-executionStart);

      /// printf( "At time %d process %d with pid= %d %s arr %d total %d remain %d and laststopTime= %d \n",curtime,id,pid,"stopped",arrivalTime,runtime,remainingTime,laststopTime);
    }
    else //finished
    {
       finishTime=curtime;
       remainingTime=0;
      // printf("At time %d process %d with pid= %d %s arr %d total %d remain %d \n",finishTime,id,pid,"finished",arrivalTime,runtime,remainingTime);
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



logger schedulerLogger("RR.txt");
StatisticsReport CPUStatisticsReport("schedulerstatsRR.txt");
struct processData pD;
 queue <pcb> processTable;

 pcb runningprocess;
 bool running_process=false;
    struct processData current_running;
   /// priority_queue<processData>pq;

    struct sigaction sa;
int quant;
void my_sigchld_handler(int sig)
{
    pid_t p;
    int status;

    while ((p=waitpid(-1, &status, WNOHANG)) != -1)
    {
      
     
      current_running.stopTime= getClk();     
       pD=current_running;
      //do some calculation and printing 
      if(p!=0)
      {running_process=false;
       // current_running.state=3;
        //current_running.finishTime=getClk();  
       //printf("child is dead %did %d\n",p,current_running.id);
      // schedulerLogger.logProcess(current_running,getClk());
        runningprocess.updatepcb(_finished,getClk()); 
        double ta=runningprocess.finishTime-runningprocess.arrivalTime;
        double wta=double(ta)/runningprocess.runtime;
        CPUStatisticsReport.addProcessUsagedata(wta,runningprocess.totalwait,runningprocess.runtime);
        schedulerLogger.logProcess(runningprocess,getClk());
}
     else { printf("child is waiting /resumed %d id %d\n",p,runningprocess.id);
           }
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

    quant=atoi(argv[1]);
    
   int prevclock;
    
   printf("run ronbin\n");
  
  int y=Recmsg(pD);
  while(y<=0)
  {  
        int x= getClk();
        
       
       y=Recmsg(pD);
       while(y==0)
	{  x= getClk();
	  printf("current received data %d priority %d\n",pD.id,pD.priority);
       
	 pcb chosenprocess;
          chosenprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);
          processTable.push(chosenprocess); printf("waiting in queue\n");
	  y=Recmsg(pD);
		
      }
    
    

    if(!running_process&&!processTable.empty())
      {  // printf("clock %d\n",getClk());
        //printf("state %d\n",running_process);
          runningprocess=processTable.front();
          //printf("new process runnibg\n");
          processTable.pop(); 
          running_process=true;
            if(runningprocess.state==2)
             {  
                runningprocess.updatepcb(_resumed,getClk()); //resumed process
              // kill(runningprocess.pid,SIGCONT); //Send resuming signal
               kill(runningprocess.pid,SIGCONT);
               
   		
             
             }
         else  
             { 

                  // current_running.state=0;
                    int id=fork();
                    if(id==0)
                        {   stringstream strs;
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
                                     //printf("running child process with id  %d\n",current_running.id);
				}
		}
      }
  //printf("here\n");
 if(!running_process)sleep(1);
  if(running_process)
  {    int prevclk=getClk();
           printf("quant %d\n",quant); 
        //   printf("clock in lseep %d\n",getClk());
	   sleep(quant);
   int y=getClk();
  //printf("remain time %d\n",quant+prevclk);
  //printf("clock %d\n",y);
          if(running_process&&y==quant+prevclk)
            { 
	        {   kill(runningprocess.pid,SIGTSTP);
                  running_process=false;
             
                   runningprocess.updatepcb(_stopped,getClk());
                  processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
                  schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file
                 
		   
	        
                 }	
}else if(running_process){sleep(quant+prevclk-y); kill(runningprocess.pid,SIGTSTP);running_process=false;
                   runningprocess.updatepcb(_stopped,getClk());
                  processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
                  schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file
                 } 
        

    
	  
            
    }
 
  
  //printf("clock %d\n",getClk());
   }
   

  while(running_process||!processTable.empty())
    {  int x= getClk();
       // printf("im in loop with clck%d\n",x);  
        
       
	if(!running_process)
            {    runningprocess=processTable.front();
          //printf("new process runnibg\n");
          processTable.pop(); 
          running_process=true;
            if(runningprocess.state==2)
             {  //kill(runningprocess.pid,SIGCONT);
                runningprocess.updatepcb(_resumed,getClk()); //resumed process
               kill(runningprocess.pid,SIGCONT); //Send resuming signal
               
   		
             
             }
                  else{   int id=fork();
                            if(id==0)
                        {   stringstream strs;
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
                                     //printf("running child process with id  %d\n",current_running.id);
				}
                    }
		
            }
      
   if(running_process)
  {    int prevclk=getClk();
           
	   sleep(quant);
   int y=getClk();
          if(y==quant+prevclk)
            {   if(running_process)
	        { kill(runningprocess.pid,SIGTSTP); running_process=false; 
                   runningprocess.updatepcb(_stopped,getClk());
                  processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
                  schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file
		   
	        
                 }	
}else if(running_process){sleep(quant+prevclk-y);if(running_process){ kill(runningprocess.pid,SIGTSTP);
                   runningprocess.updatepcb(_stopped,getClk());
                    running_process=false;
                  processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
                  schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file	
} 
        
}		                                
    
	  
            
    } 
   
    } 


 int x= getClk();
 printf("at end clck%d\n",x); 

    CPUStatisticsReport.setFinishTime(getClk());
             CPUStatisticsReport.setStarttime(startTime);
             CPUStatisticsReport.GenerateStatisticsReport();


    return 0;
    
}