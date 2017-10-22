#include <stdio.h>
#include "pcb.h"
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