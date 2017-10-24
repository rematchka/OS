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
       printf("child is dead %d\n",p);}
     else { printf("child is waiting /resumed %d\n",p);}
       break;
      

    }
}

/* It's better to use sigaction() over signal().  You won't run into the
 * issue where BSD signal() acts one way and Linux or SysV acts another. */



int main(int argc, char* argv[]) {
    initQueue(false);
    initClk();
    printf("my PID %d\n",getpid());
   

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_sigchld_handler;

    sigaction(SIGCHLD, &sa, NULL);

    
   
    

    //////To receive something from the generator, for example  id 2
  
  int y=Recmsg(pD);
  while(y<=0)
  {     int x= getClk();
     y=Recmsg(pD);
     if(y==0)
	{ 
	printf("current received data %d priority %d\n",pD.id,pD.priority);
       // printf("current received data %d\n",pD.runTime);
	
	if(!running_process)
	{
	           pq.push(pD); 		  
                   current_running=pq.top();
                   pq.pop(); 
          
                  int id=fork();
    if(id==0)
         { 
                 //std::string s = to_string(pD.runTime);
			stringstream strs;
			  strs << current_running.runTime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
                     perror("execl() failure!\n\n");    
        } 

         else 
		{ 
                    

		  running_process=true;
                  
                    current_running.pid=id;
                    current_running.state=4;
                    current_running.startOfExecution=x;
                    printf("running process child id %d\n",pD.id);
                 
                     
		}	
 	 	
	}
	else  {
                   if(pD.priority<current_running.priority)
                    {
                         //do some computation 
                            printf("Killed process %d\n",current_running.pid); 
                           kill(current_running.pid,SIGSTOP);
                           
                         
                         current_running.state=2;
                       
                         current_running.stopTime=x;
                        printf("here\n");
			 	
                         pq.push(current_running); 
			 current_running=pD;
			 running_process=true;
			 int id=fork();
                           if(id==0)
                           {      stringstream strs;
			  strs << current_running.runTime;
			  string temp_str = strs.str();
	        	char const *pchar = temp_str.c_str();
                    execl("./process.out","./process.out",pchar);
				     perror("execl() failure!\n\n");    }
			else 
				{   
                                  


					current_running.pid=id;
				    current_running.state=4;
                                    current_running.startOfExecution=x; 
				    printf("running child process with id %d\n",pD.id);
				}
                    }
                  else {pq.push(pD);printf("waiting in queue\n");}
		}
     }

    if(!running_process&&!pq.empty())
      {
          current_running=pq.top();
          pq.pop(); 
          running_process=true;
          if(current_running.state==2)
             {  current_running.state=1;
                current_running.startOfExecution=x;	
                 printf("continue process %d\n",current_running.id); 
                kill(current_running.pid,SIGCONT);
		
             }
           else { current_running.state=4;
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
				    current_running.state=4;
                                     current_running.startOfExecution=x;
                                     printf("running child process with id  %d\n",current_running.id);
				}
		}
      }
   sleep(1);
   printf(" clck%d\n",x); 
   }
   //y=Recmsg(pD);
    //printf("current received data %d\n",pD.id);

  while(!pq.empty())
    {  int x= getClk();
        printf("im in loop with clck%d\n",x);  
        
       
	if(!running_process)
            {
                 current_running=pq.top();
               pq.pop(); 
              running_process=true;
          if(current_running.state==2)
             {  current_running.state=1;
                current_running.startOfExecution=x;
                kill(current_running.pid,SIGCONT);
                 printf("running child process with id  %d\n",current_running.id);
                
             }
           else { current_running.state=4;
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
				    current_running.state=4;
                                    current_running.startOfExecution=x;
                                    printf("running child process with id  %d\n",current_running.id);
				}
		}
            }
     sleep(1);
    } 

while(running_process) sleep(1);
    return 0;
    
}
