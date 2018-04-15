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
    
    pthread_t processes[numProcesses+2];
    /*sem_init(&full, 0, 0);
    sem_init(&empty, 0, 99);*/
    
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
        pthread_join(processes[i],NULL); //NOTE: if not NUll retval can get back the error codes
    }
    //while(1);
    

    freedom(0);
    return 0;
}

void *faultSimulator(){
    //int pid = *(int *)pid; //NOTE: not necessary
    while(1){ //HOW to deal with the infinite loop? how to check when processes finish
        int rando = rand()%numResourceType+1;
        
        pthread_mutex_lock(&mutex);
        avail[rando] -= 1;
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
            if(flag==0)
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

void requestor(int pid, int *req){
    for(int j=0; j<numResourceType; j++){ //NOTE: Should this be the case?
        if(need[pid][j]>0){
            req[j]= rand()%(need[pid][j]+1); //range from 0 to need inclusive
        }else{
            req[j]=0; //NOTE: x%0 is undefined
        }
    } 
}

void *processSimulator(void *pidl){ //NOTE: all should be protected sinceneed can't even be 
    int pid = *(int*)pidl;
    int *req;
    int result,j;
    req = (int*) malloc(sizeof(int)*numResourceType);
    if(req==NULL){  
        printf("ERROR: failed to allocate memory for PID's: %d request vector\n",pid);
        return NULL;
        //freedom(-1);
    }
    
    requestor(pid, req);
    
    while(1){
        pthread_mutex_lock(&mutex);
        printf("\n \t Request for PID: %d is:",pid);
        for(j=0; j<numResourceType; j++){ //NOTE: Should this be the case?
            printf(" %d ",req[j]);
        }
        printf("\n");
        
    //while(1){
        
        result = bankers(req, pid);
        switch(result){
            case -1:
                printf("Invalid request, try again\n");
                requestor(pid, req);
                break; 
            case -2: //if not safe 
                pthread_mutex_unlock(&mutex); //NOTE: REMOVE THIS
                //usleep(1000); //1 milis. NOTE: is this necessary?
                sleep(1);
                continue;
            case 1:
                printf("PID: %d System is safe : allocated request\n",pid);//note allocation in bankers
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
                        max[pid][j] = 0; //NOTE: is this necessary?
                    }
                    free(req);
                    pthread_mutex_unlock(&mutex);
                    return NULL;
                }else if(result == 1){//still need resources
                    requestor(pid, req);//generate new request vals, but service after sleep
                    pthread_mutex_unlock(&mutex); //NOTE: is this necessary. 
                    sleep(3);
                    continue;
                }
            //sleep(3);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    free(req);
    return NULL;
}



int bankers(int *req, int pid){
    for(int i=0; i<numResourceType; i++){ //TODO: change to j
        if(req[i] > need[pid][i]){ //sanity check
            printf("Error: requested ammount: %d, of resource: %d, exceeds need: %d \n",req[i],i,need[pid][i]);
            return -1;
        }
        if(req[i] > avail[i]){
            printf("PID: %d Not enough of resource %d available, waiting\n",pid,i);
            return -2;
        }
    }
    int retval = requestSimulator(pid,req);
    //Provisional Allocation
    if( retval == 1 ){ //if valid request. 
        return 1;
    }else{//cancel allocation
        //NOTE: how does provisional allocation work. 
        //bankers(req, pid);
        return retval; //retval == -2
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
            //flag=0;
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
                for(j=0; j<numResourceType; j++){
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
