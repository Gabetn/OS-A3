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
    
    /*pthread_t processes[numProcesses];
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 99);
    
    int status;
    for(i=0; i<numProcesses; i++){
        printf("Creating Process thread %d \n",i);
        status = pthread_create(&processes[i],NULL,processSimulator,i);
        if(status>0){
            printf("Error creating process thread %d\n",i);
        }
    }

    while(1);
    */
    freedom(0);
    return 0;
}

void *processSimulator(int pid){
    int *req;
    int result;
    while(1){
        req = (int*) malloc(sizeof(int)*numResourceType);
        if(req==NULL){
            printf("ERROR: fiald to allocate memory for request vector\n");
            freedom(-1);
        }
        //TODO: probs encapsulate in mutex
        for(int i=0; i<numResourceType; i++){
            req[i]= rand()%(need[pid][i]+1); //range from 0 to need inclusive
        }

        result = bankers(req, pid);
        if(result==0){ //step4.
            sleep(3);
        } else{ //step6 
            //process blocks until another process finishes and relinquishes its resources

        }
    }


}

int bankers(int *req, int pid){
    for(int i=0; i<numResourceType; i++){ //TODO: change to j
        if(req[i] > need[pid][i]){ //sanity check
            printf("Error: requested ammount: %d, of resource: %d, exceeds need: %d \n",req[i],i,need[pid][i]);
            return -1;
        }
        if(req[i] > avail[i]){
            printf("Not enough of resource %d available, waiting\n",i);
            //TODO: waiting code. 
            bankers(req,pid); //NOTE: this creates a busy waiting loop.
        }
        //Provisional Allocation
        if(isSafe()==0){

        }else{//cancel allocation
            //NOTE: how does provisional allocation work. 
            bankers(req, pid);
        }

    }
    return(0);
}

int isSafe(){

    return -1;
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
