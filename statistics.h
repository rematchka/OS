#include <vector>
#include <stdio.h>
#include <cmath>
using namespace std;

class StatisticsReport
{
  private:

  void findAvgWTA()
  {
  	for(int i=0;i<processesWTA.size();i++)
  	{
  		AvgWTA+=processesWTA[i];
  	}
  	AvgWTA/=processesWTA.size();
  }
  
  void findStandardDeviation()
  {
  	 double squaresum=0;
  	 for(int i=0;i<processesWTA.size();i++)
  	 {
  	 	squaresum+=((processesWTA[i]-AvgWTA)*(processesWTA[i]-AvgWTA));
  	 }
  	 squaresum/=processesWTA.size();

  	 standardDev=sqrt(squaresum);

  }

  void calculateCPUutilisation()
  {
     cpuUtil=((totalrunTime)/double(finishTime))*100;
     cpuUtil=(cpuUtil<=100)?cpuUtil:100;
  }

  void findAverageWaitTime()
  {
  	AvgWait=totalWait/processesWTA.size();
  }

  public:
  FILE* fptr;

  double cpuUtil=0;
  int totalrunTime=0;
  int totalWait=0;
  double AvgWTA=0;
  double AvgWait=0;
  double standardDev=0;
  vector <double>processesWTA;
  int finishTime;
  int startTime;



  StatisticsReport(const char* fname)
  {
  	fptr=fopen(fname,"w");
  	cpuUtil=0;
    totalrunTime=0;
    totalWait=0;
    AvgWTA=0;
    AvgWait=0;
    standardDev=0;
  }

  void GenerateStatisticsReport()
  {

     calculateCPUutilisation();
     findAvgWTA();
     findStandardDeviation();
     findAverageWaitTime();

     if(fptr!=NULL)
     {
     	fprintf(fptr, "CPU utilization=%.2f%% \n",cpuUtil );
     	fprintf(fptr,"Avg WTA= %.2f\n",AvgWTA);
     	fprintf(fptr,"Avg Waiting= %.2f\n",AvgWait);
     	fprintf(fptr, "Std WTA= %.2f\n",standardDev );
     }

  }



 void addProcessUsagedata(double WTA,int wait,int runtime)
 {
 	processesWTA.push_back(WTA);
 	totalWait+=wait;
 	totalrunTime+=runtime;
  printf("totalrunTime:%d\n",totalrunTime );

 }


void setFinishTime(int ftime)
{
	finishTime=ftime;
  printf("finishTime:%d\n",finishTime);

}

void setStarttime(int stime)
{
  startTime=stime;
  printf("startTime:%d\n",startTime );
}

~StatisticsReport()
{
	fclose(fptr);
}



};