#ifndef pcb_h
#define pcb_h
class pcb
{
   public:
   int pid;
   int arrivalTime;
   //int executionTime;
   int remainingTime;
   int state;   //0 ready
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

#endif