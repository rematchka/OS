


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
    laststopTime=0;
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









struct compare
{
    bool operator() (pcb p1,pcb p2)
    {
        return p1.remainingTime > p2.remainingTime;
    }
};

//Priority queue holdinug all the processes that the should be handled by the scheduler

 priority_queue <pcb,vector<pcb>,compare> processTable;

 pcb runningprocess;
 
 bool running=false; //A variable to indicate whether a process is currently running
 //initiate a logger class for all the running processes
int schedulerstate=-1;
int currentprocesscnt=0;
struct processData pD;
int startTime=1000;

queue<processData>tempprocesses;

logger schedulerLogger("schedulerlog.txt");
StatisticsReport CPUStatisticsReport("schedulerstats.txt");


void SigIntHandler(int sig)
{

  int msg=Recmsg(pD);
  printf("received a msg form pg %d\n",msg);
  if(msg==0)
  {
      schedulerstate=state_message_received;
      tempprocesses.push(pD);
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
        printf("process with pid %d has contineued .\n",p );
      }
      else
      {
      if(p!=0) //The process has terminated
      {
        //Decrement the count of currently forked processes
        currentprocesscnt--;
        printf("currentprocesscnt from sigchild handler is %d",currentprocesscnt);
        printf("process with pid %d terminated\n",p);
        //running=false;
        runningprocess.updatepcb(_finished,getClk()); 
        double ta=runningprocess.finishTime-runningprocess.arrivalTime;
        double wta=double(ta)/runningprocess.runtime;
        CPUStatisticsReport.addProcessUsagedata(wta,runningprocess.totalwait,runningprocess.runtime);
        schedulerLogger.logProcess(runningprocess,getClk()); //log the termination of the currently running process

        if(!processTable.empty())
        { 
          //Now the scheduler will choose an alternative process to the one that terminated
          runningprocess=processTable.top();
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
             //increment current process cnt
             currentprocesscnt++;
             
          }
          else 
          {
               printf("Resuming process with pid %d \n",runningprocess.pid);
               runningprocess.updatepcb(_resumed,getClk()); //resumed process
               kill(runningprocess.pid,SIGCONT); //Send resuming signal
             
          }
          
          schedulerLogger.logProcess(runningprocess,getClk()); //log the resuming or the start of a process
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

void fork_process(pcb& newprocess)
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




int main(int argc, char* argv[]) {
    
    initClk();
    initQueue(false);
    
    //Set process termination handler
    signal(SIGCHLD,my_sigchld_handler);
    signal(SIGILL,SigIntHandler);
    bool finished=false;
  

   while(!finished)
   {

      pause();
      printf("Scheduler is in state %d \n",schedulerstate );
      if(schedulerstate==state_message_received)
      {   //Reset scheduler state
          schedulerstate=-1;

              printf("received data\n"); 
              pcb chosenprocess;
              pD=tempprocesses.front();
              tempprocesses.pop();
              chosenprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);
              startTime=(startTime<chosenprocess.arrivalTime)?startTime:chosenprocess.arrivalTime;//To keep the arrival time of the first process

          while(!tempprocesses.empty())
          {
              pcb newprocess;
              pD=tempprocesses.front();
              tempprocesses.pop();
              newprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);

              startTime=(startTime<newprocess.arrivalTime)?startTime:newprocess.arrivalTime;//To keep the arrival time of the first process
              if(newprocess.runtime<chosenprocess.runtime)
              {
                processTable.push(chosenprocess);
                chosenprocess=newprocess;
                
              }
              else
              {
                processTable.push(newprocess);
              }

          }
        
      
          int remainingTime;
          //update remaining time for running process
          if(running)   remainingTime=runningprocess.remainingTime-(getClk()-runningprocess.executionStart);
          if((!(running))||(chosenprocess.remainingTime<remainingTime))
           {
         //STOP THE RUNNING PROCESS AND UPDATE PCB TO STATE 2=>STOPPED
              if(running)
                { 
                  
                  printf("sig stop is sent to process with pid= %d\n",runningprocess.pid);
                  runningprocess.updatepcb(_stopped,getClk());
                  processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
                  schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file
                  kill(runningprocess.pid,SIGTSTP);
                  

                }
                fork_process(chosenprocess);

        //START THE NEWLY ARRIVED PROCESS , SET ITS START OF EXECUTION AND UPDATE NEW PROCESS PCB TO STATE
        //FORK THE NEW PROCESS
            }
         else 
          {
            processTable.push(chosenprocess);

            printf("push the chosen process\n");
          }   
      }
     else
     {
       if(schedulerstate==state_last_message_sent)
       {
          if(currentprocesscnt==0) //All procecesses have terminated
           {// printf("stuck here\n");
             finished=true; //terminate the scheduler as it has finished
             CPUStatisticsReport.setFinishTime(getClk());
             CPUStatisticsReport.setStarttime(startTime);
             CPUStatisticsReport.GenerateStatisticsReport();
           }
        
          printf("currentprocesscnt: %d\n",currentprocesscnt);
       }
     }

   }
   

    return 0;
    
}

