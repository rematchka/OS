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
using namespace std;
class logger 
{
 FILE * fptr;
 const char* fname; 

public:

    logger(const char* fn1ame)
    {  fname=fn1ame;
    }
    
    void logProcess(processData process,int curtime)
    {
       // int pid,arrtime,runtime,remain,wait;
         int wait;
         double wta;
         int ta;
         const char * states[4]={"started","resumed","stopped","finished"};
        printf("printing logs\n");
        printf("state %d\n",process.state);
         switch(process.state)
         {
            case 0:
            {
                //state="started";
                wait=process.startOfExecution-process.arrivalTime;
                break;
            }
            case 1:
            {
                //state="resumed";
                 wait=process.startOfExecution-process.finishTime;
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
                printf("in case 3 \n");
                wait=0;
               printf("%d %d\n",process.finishTime,process.arrivalTime);
                ta=process.finishTime-process.arrivalTime;
                wta=double(ta)/process.runTime;
                break;
            }
            default:
             break;
         }

//printf("hello file name is %s\n",fname);
   int remin=process.runTime-(curtime-process.startOfExecution);
     fptr=fopen(fname,"a");
         if(fptr!=NULL&&process.state==3)

      {  
	rewind(fptr);
 fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f \n",process.finishTime,process.pid,states[process.state],process.arrivalTime,process.runTime,remin,wait,ta,wta);

 fclose(fptr);
printf("almost there \n");
}
          else if(fptr!=NULL)
          {printf("almost there 2\n");
 rewind(fptr);
  fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d \n",process.startOfExecution,process.pid,states[process.state],process.arrivalTime,process.runTime,remin,wait);
printf("almost there 2\n");
 fclose(fptr);
}

    }

    ~logger()
    {    
       // fclose(fptr);
       printf("closed el bta3 el file\n");
    }

};


logger schedulerLogger("HPF.txt");
struct processData pD;
 bool running_process=false;
    struct processData current_running;
    priority_queue<processData>pq;

    struct sigaction sa;
void my_sigchld_handler(int sig)
{
    pid_t p;
    int status;

    while ((p=waitpid(-1, &status, WNOHANG)) != -1)
    {
       /* Handle the death of pid p */
     
      current_running.stopTime= getClk();
     
     pD=current_running;
//do some calculation and printing 
      if(p!=0)
      {running_process=false;
        current_running.state=3;
        current_running.finishTime=getClk();  
       printf("child is dead %d\n",p);
       schedulerLogger.logProcess(current_running,getClk());
}
     else { printf("child is waiting /resumed %d\n",p);}
       break;
      

    }
}






int main(int argc, char* argv[]) {
    initQueue(false);
    initClk();
    printf("my PID %d\n",getpid());
   

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_sigchld_handler;

    sigaction(SIGCHLD, &sa, NULL);

    
   
    

  
  int y=Recmsg(pD);
  while(y<=0)
  {     int x= getClk();
      if(running_process){printf("killing my son id %d\n",current_running.id);kill(current_running.pid,SIGSTOP);}
     y=Recmsg(pD);
      while(y==0)
	{  x= getClk();
	  printf("current received data %d priority %d\n",pD.id,pD.priority);
       
	
	  pq.push(pD);printf("waiting in queue\n");
	  y=Recmsg(pD);

		
     }

    if(!running_process&&!pq.empty())
      { x= getClk();
          current_running=pq.top();
          pq.pop(); 
          running_process=true;
           current_running.state=0;
                 int id=fork();
                    if(id==0)
                        {   stringstream strs;
			  strs << current_running.runTime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
				     perror("execl() failure!\n\n"); }
			else 
				{  
				  
                            current_running.pid=id;
				    current_running.state=0;
                                     current_running.startOfExecution=x;
                                     schedulerLogger.logProcess(current_running,getClk());
                                     printf("running child process with id  %d\n",current_running.id);
				}
		
      }
  if(running_process)
   { printf("Resuming my son id %d\n",current_running.id);kill(current_running.pid,SIGCONT);}
   kill(getpid(),SIGSTOP);
   sleep(1);
   printf(" clck%d\n",x); 
  
   
   }
   

  while(!pq.empty())
    {  int x= getClk();
        printf("im in loop with clck%d\n",x);  
        
       
	if(!running_process)
            {     current_running=pq.top();
                  pq.pop();     
                 running_process=true;
                 int id=fork();
                     if(id==0)
                             {  stringstream strs;
			  strs << current_running.runTime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
				     perror("execl() failure!\n\n");
		        	 }
			else  
				{  
				  current_running.pid=id;
				    current_running.state=0;
                                    current_running.startOfExecution=x;
                                    schedulerLogger.logProcess(current_running,getClk()); 
                                    printf("running child process with id  %d\n",current_running.id);
				}
		
            }
    kill(getpid(),SIGSTOP);
    
     sleep(1);

    } 

while(running_process) {kill(getpid(),SIGSTOP);sleep(1);}
 //schedulerLogger.close();
  
 int x= getClk();
 printf("at end clck%d\n",x); 

  

    return 0;
    
}
