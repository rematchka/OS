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
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include<stdlib.h>
using namespace std;


vector<processData>v;
void ClearResources(int);
void readData(){

string line;
int cnt=1;
  ifstream myfile ("processes.txt");
  if (myfile.is_open())
  {   
   getline (myfile,line) ;
    
	      printf("%s\n",line.c_str());
	    string id,prior,arr,run;
	     while(myfile>>id>>arr>>run>>prior)
	 {  cout<<id<<" " <<" "<<arr<<" "<<run<<" "<<prior<<endl;
         processData  pdd;
	 pdd.id=atoi( id.c_str() );
	pdd.arrivalTime=atoi( arr.c_str() );
	  pdd.priority=atoi( prior.c_str() );
	pdd.runTime=atoi( run.c_str() );
	v.push_back(pdd);
        }


    
     
    
    myfile.close();
  }

  else {printf("Unable to open file\n");} 
//printf("size %d\n",v.size());



}



int main() {
    
    initQueue(true);
    //TODO: 
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
      printf("Please choose Algorithm:\n 1- HPf\n 2-RR\n 3-SRTN\n");
     int x;
  scanf("%d",&x);
    // 2-Initiate and create Scheduler and Clock processes.
   int idclk=fork();
  if(idclk==0)
	{  
	execl("./clock.out","./clock.out",  (char*)NULL);
        perror("execl() failure!\n\n");

           
	}
int idsheduler;

if(x==1)//for HPF
                {   idsheduler=fork();  if(idsheduler==0){execl("./sch.out","./sch.out",  (char*)NULL);
        	     perror("execl() failure!\n\n");}
                  }


  else if(x==2)
           {printf("Please enter quantum nunber"); 
             int quant;
             cin>>quant;
           //  scanf("%d",&quant);
               idsheduler=fork();
           if(idsheduler==0){
              stringstream strs;
              strs << quant;
                string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                   execl("./RR.out","./RR.out",  pchar);
        	     perror("execl() failure!\n\n");
               }
}

else {   idsheduler=fork();
           if(idsheduler==0){
                      execl("./SRTN2.out","./SRTN2.out");
        	     perror("execl() failure!\n\n");
}
        }

   //int idsheduler=fork();
    
		
    

    // 3-use this function AFTER creating clock process to initialize clock, and initialize MsgQueue
    initClk();
    

    
    //TODO:  Generation Main Loop
    //4-Creating a data structure for process  and  provide it with its parameters 
    //5-Send & Notify the information to  the scheduler at the appropriate time 
    //(only when a process arrives) so that it will be put it in its turn.
    readData();
    sort(v.begin(),v.end());



    //===================================
    //Preimplemented Functions examples

    
	while(v.size()>0)
	{       int x= getClk();
		
		int i=0;
		while(i<v.size())
		{  if(v[i].arrivalTime==x)
		   {    //kill(idsheduler,SIGCONT);
                       kill(idsheduler,SIGILL);
			 struct processData pD=v[i];
			Sendmsg(pD);
			v.erase (v.begin()+i,v.begin()+i+1);
                       // printf("sending data%d\n",pD.id) ;
                      sleep(1);
	           }
			else i++;
		
		}
          
		
	} 
    //no more processes, send end of transmission message
    // sleep(10);
    lastSend();
    kill(idsheduler,SIGILL);

    //////////To clear all resources
int pid,stat_loc;
  pid = wait(&stat_loc);
            if(!(stat_loc & 0x00FF))
                printf("\nA child with pid %d terminated with exit code %d\n", pid, stat_loc>>8);
            if(WIFEXITED(stat_loc))
                printf("\nChild terminated normally with status %d", WEXITSTATUS(stat_loc));
     ClearResources(0);
}
   
    //======================================



void ClearResources(int)
{
    msgctl(qid, IPC_RMID, (struct msqid_ds*)0);
    destroyClk(true); 
    exit(0);
}




