#include "clkUtilities.h"
#include "queueUtilities.h"


int main(int argc, char* argv[]) {
    initQueue(false);
    initClk();
    
    //TODO: implement the scheduler :)


    //===================================
    //Preimplemented Functions examples

    /////Toget time use the following function
    

    //////To receive something from the generator, for example  id 2
    struct processData pD;
  int y=Recmsg(pD);
  while(y<=0)
  {  //printf("current received data %d\n",y); 
     y=Recmsg(pD);
     if(y==1||y==0){ 
	printf("current received data %d\n",pD.id);
	int x= getClk();
        printf("current time is %d\n",x);}
   // Recmsg(pD);
   //returns -1 on failure; 1 on the end of processes, 0 no processes available yet
   }
   //y=Recmsg(pD);
    printf("current received data %d\n",pD.id);

    return 0;
    
}
