

#include "clkUtilities.h"
#include "queueUtilities.h"
#include <vector>
#include <queue>
#include <sstream>
using namespace std;


class pcb
{
   public:
   int pid;
   int arrivalTime;
   //int executionTime;
   int remainingTime;
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

   void setParameters(int pid,int arrivaltime,int runtime,int priority)
   {
    this->pid=pid;
    this->arrivalTime=arrivaltime;
    this->runtime=runtime;
    this->priority=priority;
    this->remainingTime=runtime;
   }
   
   void updatepcb(int newstate,int curtime)
   {
    if(newstate==0) //ready the process has just started
    {
      executionStart=curtime;
      totalwait+=(executionStart-arrivalTime);
    }

    else if(newstate==1) //resumed process
    {
      executionStart=curtime;
      totalwait+=(executionStart-laststopTime);

    }
    else if(newstate==2) //stopped process
       {laststopTime=curtime;
       remainingTime=curtime-executionStart;
        }
     else
     {
       finishTime=curtime;
     }
    
   }



};



class logger 
{
 FILE * fptr;

public:

    logger(const char* fname)
    {
         fptr=fopen(fname,"r");
    }
    
    void logProcess(pcb process,int curtime)
    {
       // int pid,arrtime,runtime,remain,wait;
         int wait;
         double wta;
         int ta;
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

         if(process.state==3)

        fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2f \n",process.executionStart,process.pid,states[process.state],process.arrivalTime,process.runtime,process.remainingTime,wait,ta,wta);
          else
             fprintf(fptr, "At time %d process %d %s arr %d total %d remain %d wait %d \n",process.executionStart,process.pid,states[process.state],process.arrivalTime,process.runtime,process.remainingTime,wait);

    }

    ~logger()
    {
        fclose(fptr);
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
 

 //initiate a logger class for all the running processes

logger schedulerLogger("schedulerlog.txt");




void my_sigchld_handler(int sig)
{
    pid_t p;
    int status;

    while ((p=waitpid(-1, &status, WNOHANG)) != -1)
    {
       /* Handle the death of pid p */
      
//do some calculation and printing 
      if(p!=0) //The process has terminated
      {

        runningprocess.updatepcb(3,getClk()); 

        schedulerLogger.logProcess(runningprocess,getClk()); //log the termination of the currently running process

        if(!processTable.empty())
        {
          runningprocess=processTable.top();
          processTable.pop();
        

          if(runningprocess.laststopTime==0) //first time to start
          {
            runningprocess.updatepcb(0,getClk());
          }
          else 
          {
            runningprocess.updatepcb(1,getClk()); //resumed process
          }
          
          schedulerLogger.logProcess(runningprocess,getClk()); //log the resuming or the start of a process
        }
      }
             break; //Break after handling process termination
    }
}




int main(int argc, char* argv[]) {
    
    initClk();
    initQueue(false);
    //TODO: implement the scheduler :)
   
    //===================================
    signal(SIGCHLD,my_sigchld_handler);

    bool finished=false;

    struct processData pD;
    while(!finished)
    {   int x= getClk();
        printf("clck%d\n",x); 
      
    //////To receive something from the generator, for example  id 2
     int y=Recmsg(pD);
     printf("y%d\n",y); 
    if(y==-1) printf("failure to receive data\n"); 
    else if(y==0)  //The scheduler received a new process
    {
      printf("received data\n"); 
     pcb newprocess;
     newprocess.setParameters(pD.id,pD.arrivalTime,pD.runTime,pD.priority);
     ///processTable.push(newprocess);
 ///////you have to check if there is initially running processs////////////////////////////////
     //check if the received process has a shorter runtime than the currently runnig process

     if(newprocess.remainingTime<runningprocess.remainingTime)
     {
         //STOP THE RUNNING PROCESS AND UPDATE PCB TO STATE 2=>STOPPED
      kill(runningprocess.pid,SIGTSTP);
      runningprocess.updatepcb(2,getClk());
      processTable.push(runningprocess);  //Return the stopped process to the table of ready processes
      schedulerLogger.logProcess(runningprocess,getClk());  //Log the stopping of the currently running process to the file

        //START THE NEWLY ARRIVED PROCESS , SET ITS START OF EXECUTION AND UPDATE NEW PROCESS PCB TO STATE
        //FORK THE NEW PROCESS
         int newpid=fork();

         if(newpid==-1)
         {
          perror("Scheduler failed to fork the process\n");
         } 
         else if(newpid==0)
         {  printf("forked\n");
          stringstream s;
          s<<runningprocess.remainingTime;
          execl("./process.out","./process.out",s.str().c_str());
         }
    
      newprocess.updatepcb(0,getClk());  //it has just started running
      runningprocess=newprocess;
      schedulerLogger.logProcess(runningprocess,getClk()); //log the start of a new process to the file

     }
     else
     {
         processTable.push(newprocess);

         //TODO: check if you need to log something here 
          


     }

}
else
{
   finished=true;
}
if(y==1){ finished=true;}
      sleep(1);

    //returns -1 on failure; 1 on the end of processes, 0 no processes available yet
  }

    return 0;
    
}
