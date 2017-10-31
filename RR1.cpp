


#include "clkUtilities.h"
#include "queueUtilities.h"
#include <vector>
#include <queue>
#include <sstream>
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











//Priority queue holdinug all the processes that the should be handled by the scheduler

 queue <pcb> processTable;

 int quantum;

 pcb runningprocess;
 
 bool running=false; //A variable to indicate whether a process is currently running
 //initiate a logger class for all the running processes
 int schedulerstate=-1;
 int currentprocesscnt=0;
 int startTime=-1;
 struct processData pD;

logger schedulerLogger("schedulerlog.txt");
StatisticsReport CPUStatisticsReport("schedulerstats.txt");


void fork_process(pcb newprocess)
{
        int newpid=fork();

         if(newpid==-1)
         {
          perror("Scheduler failed to fork the process\n");
         } 
         else if(newpid==0)
         {  printf("forked\n");
          stringstream s;
          printf("forked a process with remainingTime %d \n",newprocess.remainingTime);
          s<<newprocess.remainingTime;
          execl("./process.out","./process.out",s.str().c_str());
         }
      running=true;
      newprocess.pid=newpid;
      newprocess.updatepcb(_started,getClk());  //it has just started running
      runningprocess=newprocess;
      schedulerLogger.logProcess(runningprocess,getClk()); //log the start of a new process to the file
      currentprocesscnt++; //Increment the count of currently forked processes
}

//Define the handlers
void SigIntHandler(int sig)
{

  int msg=Recmsg(pD);
  printf("received a msg form pg %d\n",msg);
  if(msg==0)
  {
      schedulerstate=state_message_received;
  }
  else
  {
    if(msg==1)
    {
      schedulerstate=state_last_message_sent;
    }
  }

}



void my_sigchld_handler(int sig)
{
    pid_t p;
    int status;
    printf("Sig child handler entered\n");
    while ((p=waitpid(-1, &status, WNOHANG|WUNTRACED|WCONTINUED)) != -1)
    {
      
      if(WIFSTOPPED(status))
      {
        printf("process with pid %d has stopped with stop signal %d \n",p,WSTOPSIG(status));
      }
      else if(WIFCONTINUED(status))
      {
        printf("process with pid %d has continued .\n",p );
      }
      else
      {
      if(p!=0) //The process has terminated
      {
        //Decrement the count of currently forked processes
        currentprocesscnt--;
        printf("currentprocesscnt from sigchild handler is %d",currentprocesscnt);
        printf("process with pid %d terminated\n",p);
        runningprocess.updatepcb(_finished,getClk()); 
        
        schedulerLogger.logProcess(runningprocess,getClk()); //log the termination of the currently running process
        double ta=runningprocess.finishTime-runningprocess.arrivalTime;
        double wta=double(ta)/runningprocess.runtime;
        CPUStatisticsReport.addProcessUsagedata(wta,runningprocess.totalwait,runningprocess.runtime);
        if(!processTable.empty())
        { 
          alarm(quantum); //Update alarm for the next process to run
          //Now the scheduler will choose an alternative process to the one that terminated
          runningprocess=processTable.front();
          processTable.pop();
           
        // runningprocess.executionStart=getClk();
          running=true;

          if(runningprocess.laststopTime==0) //first time to start
          {  
               int newpid=fork();
               if(newpid==-1)
                  
               {
                perror("process not created\n");
               }
               else if(newpid==0)
               {
                   stringstream s;
                   s<<runningprocess.remainingTime;
                   execl("./process.out","./process.out",s.str().c_str(),(char*)NULL);
               }

             
             runningprocess.updatepcb(_started,getClk());
             runningprocess.pid=newpid;

          
             schedulerLogger.logProcess(runningprocess,getClk()); //log the resuming or the start of a process
             //increment current process cnt
             currentprocesscnt++;
             
          }
          else 
          {
               printf("Resuming process with pid %d \n",runningprocess.pid);

               runningprocess.updatepcb(_resumed,getClk()); //resumed process


          
                schedulerLogger.logProcess(runningprocess,getClk()); //log the resuming or the start of a process
                kill(runningprocess.pid,SIGCONT); //Send resuming signal
             
          }
        }
        else
        {
           running=false; //The queue is empty so there are no running processes
        }
      }
      }
 
             break; //Break after handling process termination
    }
}

void SigAlarmHandler(int sig)
{
   printf("Sig alarm handler entered\n");
   if(!processTable.empty())
   { if(running)
      {

     if(!((runningprocess.remainingTime-(getClk()-runningprocess.executionStart))==0))
     //stop the currently running process
     {
     kill(runningprocess.pid,SIGTSTP);
     //Save it status
     runningprocess.updatepcb(_stopped,getClk());
     //Return process to the ready state
     schedulerLogger.logProcess(runningprocess,getClk());

     processTable.push(runningprocess);
      }
      }
     //Fork a new process or resume a previos process
     runningprocess=processTable.front();
     processTable.pop();
     if(runningprocess.laststopTime==0)//first time to run
     {
      fork_process(runningprocess);
     }
     else
     {
      //Resume the runnig process
         printf("Resuming process with pid %d \n",runningprocess.pid);
         runningprocess.updatepcb(_resumed,getClk()); //resumed process
         kill(runningprocess.pid,SIGCONT); //Send resuming signal
         schedulerLogger.logProcess(runningprocess,getClk());
         running=true;
     }
     
   }

    //Reset alarm
   alarm(quantum);
}




int main(int argc, char* argv[]) {
    
    initClk();
    initQueue(false);
    quantum=2;//atoi(argv[1]); //set quantum
    printf("quantum: %d\n",quantum);
    //Set process termination handler
    signal(SIGCHLD,my_sigchld_handler);
    signal(SIGILL,SigIntHandler);
    signal(SIGALRM,SigAlarmHandler);
    bool finished=false;
  
   alarm(quantum);
   while(!finished)
   {
      
       //Alarm the scheduler after a given quantum
      pause();

      printf("Scheduler is in state %d \n",schedulerstate );
      if(schedulerstate==state_message_received)
      {   //Reset scheduler state
          schedulerstate=-1;

          printf("received data\n"); 
          pcb newprocess;
          newprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);
          startTime=(startTime<newprocess.arrivalTime)?startTime:newprocess.arrivalTime;
          //Push the new process to the end of queue if a process is running
          if(running)
          {
            processTable.push(newprocess);
          }
          else //nothing is running so run the process for a given quantum
          {
            fork_process(newprocess);
          }

      }
     else
     {
       if(schedulerstate==state_last_message_sent)
       {
          if(currentprocesscnt==0) //All procecesses have terminated
            {
             finished=true; //terminate the scheduler as it has finished
             CPUStatisticsReport.setFinishTime(getClk());
             CPUStatisticsReport.setStarttime(startTime);
             CPUStatisticsReport.GenerateStatisticsReport();
              alarm(0);
            }
          printf("currentprocesscnt: %d\n",currentprocesscnt);
       }
     }
     
   }
   

    return 0;
    
}

