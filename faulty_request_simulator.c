/*          ECSE 427 Assignment 3 Question 3 part 2
    Name: Gabriel Negash
    ID: 260679520
*/
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
void *faultSimulator();
void* deadlockChecker();
void *processSimulator(void *pid);
void requestor(int pid, int *req);
int requestSimulator(int pid, int* req);
int bankers(int *req, int pid);
int isSafe(int pid);
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
    
    pthread_t processes[numProcesses+2]; //+2 for faulty & deadlock checker
    
    int status;
    for(i=0; i<numProcesses; i++){
        printf("Creating Process thread %d \n",i);
        status = pthread_create(&processes[i],NULL,processSimulator,(void *)&i);
        if(status!=0){
            printf("Error creating process thread %d\n",i);
        }
    }
    int tmp = numProcesses;
    status = pthread_create(&processes[numProcesses],NULL,faultSimulator,NULL);
    if(status!=0){
        printf("Error creating fault_thread\n");
    }
    status = pthread_create(&processes[numProcesses+1],NULL,deadlockChecker,NULL);
    if(status!=0){
        printf("Error creating fault_thread\n");
    }
    for(i=0; i<numProcesses; i++){//wait for all processes to complete
        pthread_join(processes[i],NULL);  //No need to wait for the fault/deadlock checker thread
    }

    freedom(0);
    return 0;
}

void *faultSimulator(){
    //int pid = *(int *)pid; //NOTE: not necessary
    while(1){ //Infinite loop exits when either deadlock occurs or all processes serviced. 
        int fault = rand()%numResourceType; //from 0 to numRT exclusive since array indexing
        
        pthread_mutex_lock(&mutex);
        if(avail[fault]>0)
            avail[fault] -= 1;
        pthread_mutex_unlock(&mutex);
        
        sleep(10);
    }
    return NULL;
}

void *deadlock_checker(){
    int flag;
    while(1){
        flag=0;
        pthread_mutex_lock(&mutex);
        for(int i=0; i<numProcesses; i++){
            for(int j=0; j<numResourceType; j++){ 
                if(need[i][j] > avail[j]){ //if can't get all of its needed.
                    flag = 1;
                    break;
                }
            }
            if(flag==0) //process with all of resources needs <= avail
                break;
            if(i=numProcesses-1){ //if made it to the end and no process had all needs less than avail
                printf("Deadlock will occur as processes request more resources, exiting...\n");
                freedom(-1);
            }
        }
        pthread_mutex_unlock(&mutex);

        sleep(10);
    }
    return NULL;
}

/* HELPER FUNCTION: to initialize request vectors. */
//NOTE: is only called from within locked mutex ranges, thus no race conditions. 
void requestor(int pid, int *req){
    for(int j=0; j<numResourceType; j++){
        if(need[pid][j]>0){
            req[j]= rand()%(need[pid][j]+1); //range from 0 to need inclusive (+1)
        }else{
            req[j]=0; //NOTE: x%0 is undefined
        }
    } 
}

void *processSimulator(void *pidl){
    int pid = *(int*)pidl;
    int *req;
    int result,j;
    
    //Generate request vector
    req = (int*) malloc(sizeof(int)*numResourceType);
    if(req==NULL){  
        printf("ERROR: failed to allocate memory for PID's: %d request vector\n",pid);
        return NULL;
    }
    //Initialize request vector
    requestor(pid, req);
    
    while(1){
        pthread_mutex_lock(&mutex);
        printf("\n \t Request for PID: %d is:",pid);
        for(j=0; j<numResourceType; j++){ 
            printf(" %d ",req[j]);
        }
        printf("\n");
        
        result = bankers(req, pid);

        switch(result){
            case -1: //req > need
                printf("Invalid request, try again\n");
                requestor(pid, req); //REINITIALIZE REQUEST VECTOR for next iteration. 
                break; 
            case -2: //if not safe 
                pthread_mutex_unlock(&mutex); 
                //usleep(1000); //1 milis. 
                sleep(1); //used to minimize busy waiting 
                continue;
            case 1:
                printf("PID: %d System is safe : allocated request\n",pid);//Note: allocation happened in bankers
                result=0;
                //debug(hold,numProcesses,numResourceType);
                for(j=0; j<numResourceType; j++){
                    if(need[pid][j]>0) //if still need a resource
                        result=1;
                }
                if(result==0){ //TERMINATE: dont need any more resources
                    printf("PID: %d has completed allocation\n",pid);
                    for(j=0; j<numResourceType; j++){ //release each resource
                        avail[j] += hold[pid][j]; 
                        need[pid][j] = 0;
                        hold[pid][j] = 0;
                        max[pid][j] = 0; 
                    }
                    free(req);
                    pthread_mutex_unlock(&mutex);
                    return NULL;
                }else if(result == 1){//still need resources
                    requestor(pid, req);//generate new request vals, but service after sleep
                    pthread_mutex_unlock(&mutex);
                    sleep(3);
                    continue;
                }

        }
        pthread_mutex_unlock(&mutex);
    }
    
    free(req);
    return NULL;
}



int bankers(int *req, int pid){
    for(int j=0; j<numResourceType; j++){ 
        /* STEP 1: Verify needs*/
        if(req[j] > need[pid][j]){ //sanity check
            printf("Error: requested ammount: %d, of resource: %d, exceeds need: %d \n",req[j],j,need[pid][j]);
            return -1;
        }
        /* STEP 2: Verify availability */
        if(req[j] > avail[j]){
            printf("PID: %d Not enough of resource %d available, waiting\n",pid,j);
            return -2;
        }
    }
    int retval = requestSimulator(pid,req);

    if( retval == 1 ){ //if valid request. 
        return 1;
    }else{//cancel allocation
        return retval; //retval == -2
    }

    return(0);
}


/*BANKERS: Step 3 - Provisional Allocation */
int requestSimulator(int pid, int* req){ 
    for(int j=0; j<numResourceType; j++){
        avail[j]=avail[j]-req[j];
        hold[pid][j] = hold[pid][j]+req[j];
        need[pid][j] = need[pid][j]-req[j]; 
    }
    if(isSafe(pid)==0){ //Then grant resources.
        return 1; //don't undo provisional allocation, thus granting resources
    } else{ // not safe: Go to step 1. 
        //undo provisional allocation
        for(int j=0; j<numResourceType; j++){ 
            avail[j]=avail[j]+req[j];
            hold[pid][j] = hold[pid][j]-req[j];
            need[pid][j] = need[pid][j]+req[j]; 
        }
        return -2;
    }
}




int isSafe(int pid){
    int work[numResourceType]; //TEMP avail vector
    int finish[numProcesses];   //TEMP
    int i,j;

    /* Step 1: INITIALIZE */
    for(j=0; j<numResourceType; j++){
        work[j] = avail[j];
    }
    for(i=0; i<numProcesses; i++){
        finish[i] = 0;
    }

    /* Step 2: */
    int flag=0;
    i=0;
    while(i<numProcesses){ //each process
        if(finish[i]==0){ //if not done
        /*check if given prov. alloc. can ALL resources required
            be allocated to curr pros given the new availability (work)? */
            for(j=0; j<numResourceType; j++){ 
                if(need[i][j] > work[j]){ //if can't get all of its needed.
                    flag = 1;
                    break;
                }
                else if(j == numResourceType-1)
                    flag = 0;
            }

            /*STEP 3: Release Resource*/
            if(flag ==0){ //the process needs less than the new available resource for all resources
                finish[i]=1;
                for(j=0; j<numResourceType; j++){ //release what process was holding
                    work[j] = work[j] + hold[i][j];
                }
                i=0; //reset counter
                continue;
            }
        }

        i++;
    }
    
    for(i=0; i<numProcesses; i++){
        if(finish[i]==0){
            printf("Allocation is not safe. Try again\n");
            return -1; //not safe
        }
    }
    return 0;
}

void free2DArr(int **arr){ //HELPER of freedom(): frees nested arrays 
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
void allocate2DArr(int ***arr, int rows, int cols, int type) {

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

void prettyPrint(int **arr, int rows, int cols){ //HELPER: called when -DDEBUG flag is raised. used for printing 2d array.
    for(int i = 0; i<rows; i++){
        printf("Process %d : \n",i);
        for(int j=0; j<cols; j++){
            printf("\t Max/Need/HOLD requests for Resource %d : %d",j,arr[i][j]);
        }
        printf("\n");
    }
}
