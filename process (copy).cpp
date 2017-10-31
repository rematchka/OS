
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

/* Modify this file as needed*/
int remainingtime =0;

void sleepplease(int x)
{
  int clll=getClk();
   int time=clll+x;
   
   while(time>getClk()){}
   printf("clcok in child%d\n",getClk());
}

int main(int agrc, char* argv[]) {

    //if you need to use the emulated clock uncomment the following line
    initClk();

    //TODO: it needs to get the remaining time from somewhere
    //remainingtime = ??;
 remainingtime=atoi(argv[1]);
 // printf("Hello im new child..... with remaining time%d\n",remainingtime);
  
    while(remainingtime>0) {
     
       
       
     
   //  printf("clock in child %d\n",getClk());
    sleepplease(1);
remainingtime--;
 printf(" child..... with remaining time%d\n",remainingtime);
    
    }
    //printf("parent id%d\n",getppid());
   // kill(getppid(),SIGCONT);
  //  printf("continueing parent\n");
     
    //if you need to use the emulated clock uncomment the following line
    destroyClk(false);
   exit(0);
    return 0;
}
