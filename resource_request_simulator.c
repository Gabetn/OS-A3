#include <stdio.h>  //for printf and scanf
#include <stdlib.h> //for malloc
#include <pthread.h>
#include <unistd.h>
//#include <limits.h>
#include <semaphore.h>


#ifdef DEBUG
    #define debug(x,y,z); prettyPrint(x,y,z);
#else
   #define debug(x,y,z); /* Nothing here indicates it's compiled out */
#endif

//FORWARD Declaration
void *processSimulator(int pid);
int requestSimulator(int pid, int* req);
int bankers(int *req, int pid);
int isSafe();
void free2DArr(int **arr);
void freedom(int stat);
void allocate2DArr(int ***arr, int rows, int cols, int type);
void prettyPrint(int **arr, int rows, int cols); //debugger helper

//GLOBAL vars. 
int numProcesses, numResourceType; //NOTE: NOT TOTAL NUM RESOURCES, BUT NUM RESOURCE TYPE?
int *avail, **max; //QUESTION: is maxClaimResPerProcess a global value
    //NOTE: avail array is the number of available resources per type
    /*NOTE: max is double array holding the max # of units of resource R that process P will request
            Thus max is #Processes wide x #ResourceType deep*/
int **need, **hold;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//sem_t full;
//sem_t empty;

int main()
{
    printf("Enter the number of processes : ");
    scanf("%d", &numProcesses);
    
    printf("Enter the number of distinct resource types : ");
    scanf("%d", &numResourceType);
    
    avail = malloc(numResourceType*sizeof(int));
    if(avail==NULL){
        printf("ERROR: failed to allocate memory for available vector\n");
        return -1;
    }

    int i, j;
    printf("Enter the amount of each resource in the system\n");
    for(i = 0; i<numResourceType; i++){ //NOTE: is there any kind of validation needed here?
        printf("\t Resource %d amount : ", i);
        scanf("%d", &avail[i]);
    }
   
    //Allocate max array
    allocate2DArr(&max,numProcesses,numResourceType,1);
    allocate2DArr(&need,numProcesses,numResourceType,1);
    allocate2DArr(&hold,numProcesses,numResourceType,0);

    //Initialize max array
    printf("For each process, enter the maximum amount claims allowed per resource type\n");
    for(i = 0; i<numProcesses; i++){
        printf("Process %d : \n",i);
        for(j=0; j<numResourceType; j++){
            printf("\t Max # requests for Resource %d : ",j);
            scanf("%d", &max[i][j]);
            need[i][j] = max[i][j];
        }
    }
    printf("MAX:\n")
    debug(max,numProcesses,numResourceType);
    printf("NEED:\n");
    debug(need,numProcesses,numResourceType);
    printf("HOLD:\n");
    debug(hold,numProcesses,numResourceType);
    
    pthread_t processes[numProcesses];
    /*sem_init(&full, 0, 0);
    sem_init(&empty, 0, 99);*/
    
    int status;
    for(i=0; i<numProcesses; i++){
        printf("Creating Process thread %d \n",i);
        status = pthread_create(&processes[i],NULL,processSimulator,i);
        if(status!=0){
            printf("Error creating process thread %d\n",i);
        }
    }
    for(i=0; i<numProcesses; i++){//wait for all processes to complete
        pthread_join(processes[i],NULL); //NOTE: if not NUll retval can get back the error codes
    }
    //while(1);
    

    freedom(0);
    return 0;
}



void *processSimulator(int pid){ //NOTE: all should be protected sinceneed can't even be 
    int *req;
    int result,j;
    req = (int*) malloc(sizeof(int)*numResourceType);
    if(req==NULL){ //TODO: ABORT, not end entire program. 
        printf("ERROR: failed to allocate memory for PID's: %d request vector\n",pid);
        return;
        //freedom(-1);
    }
    for(j=0; j<numResourceType; j++){ //NOTE: Should this be the case?
        req[j]= rand()%(need[pid][j]+1); //range from 0 to need inclusive
    } 
    while(1){
        pthread_mutex_lock(&mutex);
        /*for(j=0; j<numResourceType; j++){ //NOTE: Should this be the case?
            req[j]= rand()%(need[pid][j]+1); //range from 0 to need inclusive
        }*/

    //while(1){
        
        result = bankers(req, pid);
        switch(result){
            case -1:
                printf("Invalid request, try again\n");
                for(j=0; j<numResourceType; j++){ //NOTE: Should this be the case?
                    req[j]= rand()%(need[pid][j]+1); //range from 0 to need inclusive
                } 
            case -2:
                printf("Not enough resources available. Try again.\n");
                pthread_mutex_unlock(&mutex); //NOTE: REMOVE THIS
                usleep(1000); //1 milis. NOTE: is this necessary?
                continue;
            case 1:
                printf("System is safe : allocating\n");//note allocation in bankers
                result=0;
                for(j=0; j<numResourceType; j++){
                    if(need[pid][j]>0) //if still need a resource
                        result=1;
                }
                if(result==0){ //dont need any more resources
                    printf("PID: %d has completed allocation\n",pid);
                    for(j=0; j<numResourceType; j++){ //release each resource
                        avail[j] += hold[pid][j]; 
                        need[pid][j] = 0;
                        hold[pid][j] = 0;
                        max[pid][j] = 0;
                    }
                    pthread_mutex_unlock(&mutex);
                    return;
                }else if(result == 1){//still need resources
                    pthread_mutex_unlock(&mutex); //NOTE: is this necessary. 
                    sleep(3);
                    continue;
                }
            //sleep(3);
        }
        pthread_mutex_unlock(&mutex);
    }
    

    return;
}



int bankers(int *req, int pid){
    for(int i=0; i<numResourceType; i++){ //TODO: change to j
        if(req[i] > need[pid][i]){ //sanity check
            printf("Error: requested ammount: %d, of resource: %d, exceeds need: %d \n",req[i],i,need[pid][i]);
            return -1;
        }
        if(req[i] > avail[i]){
            printf("Not enough of resource %d available, waiting\n",i);
            return -2;
        }
    }
    
    //Provisional Allocation
    if( requestSimulator(pid,req) ){ //if safe. apply changes
        return 1;
    }else{//cancel allocation
        //NOTE: how does provisional allocation work. 
        bankers(req, pid);
    }

    /*while(1){
        //TODO: probs encapsulate in mutex
        if( req[j] > avail[j] ) //busy wait 
            continue;

        requestSimulator(pid,req); //NOTE: &req?

        result = bankers(req, pid); 
        if(result==0){ //step4.
            sleep(3);
        } else{ //step6 
            //process blocks until another process finishes and relinquishes its resources

        }
    }*/

    return(0);
}



int requestSimulator(int pid, int* req){ //NOTE: is this step 3? Provisional Alloc.
    for(int j=0; j<numResourceType; j++){
        avail[j]=avail[j]-req[j];
        hold[pid][j] = hold[pid][j]-req[j];
        need[pid][j] = need[pid][j]-req[j]; 
    }
    if(isSafe()){ //Then grant resources.

        return 1;
    } else{ // Go to step 1. 

    }
}




int isSafe(){
    int isSafe = 0;
    int work[numResourceType];
    int finish[numProcesses];
    int i,j;
    for(i=0; i<numProcesses; i++){
        finish[i] = 0;
    }
    for(j=0; j<numResourceType; j++){
        work[i] = avail[i];
    }
    for(j=0; j<numResourceType; j++){//loop through all resources
        for(i=0; i<numProcesses; i++){
            if(finish[i]==0 && need[i][j] <= work[j]){
                work[j] = work[j]+hold[i][j];
            }
        }
    }
  
    return isSafe;
}

void free2DArr(int **arr){ //HELPER
    if(arr!=NULL){
        for(int i=0; i<numProcesses; i++){
            if(arr[i]!=NULL)
                free(arr[i]);
        }
    }
}
void freedom(int stat){ //HELPER: called either when program fails or exits, used to automate freeing arrays. 
    free(avail);
    free2DArr(max);
    free(max);
    free2DArr(need);
    free(need);
    free2DArr(hold);
    free(hold);
    
    exit(stat); //0 = success, else fail
}
void allocate2DArr(int ***arr, int rows, int cols, int type) {//TODO: delete if not needed

 //allocate base ptr;
    *arr = (int **) malloc(sizeof(int*)*rows);
    if(*arr==NULL){
        printf("ERROR: failed to allocate memory for 2D vector\n");
        freedom(-1); 
    }
    //allocate second layer of pointers
    for(int i=0; i<rows; i++){
        if(type==0){ //calloc
            (*arr)[i] = (int *) calloc(cols,sizeof(int));    
        }else{
            (*arr)[i] = (int *) malloc(sizeof(int)*cols);
        }
        if((*arr)[i]==NULL){
            printf("ERROR: failed to allocate memory for max vector\n");
            freedom(-1);
        }
    }
}

void prettyPrint(int **arr, int rows, int cols){
    for(int i = 0; i<rows; i++){
        printf("Process %d : \n",i);
        for(int j=0; j<cols; j++){
            printf("\t Max # requests for Resource %d : %d",j,arr[i][j]);
        }
        printf("\n");
    }
}
