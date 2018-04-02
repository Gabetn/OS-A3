#include <stdio.h>  //for printf and scanf
#include <stdlib.h> //for malloc

//FORWARD Declaration
void free2DArr(int **arr);
void freedom(int stat);
void allocate2DArr(int **arr, int rows, int cols);

//GLOBAL vars. 
int numProcesses, numResourceType; //NOTE: NOT TOTAL NUM RESOURCES, BUT NUM RESOURCE TYPE?
int *avail, **max; //QUESTION: is maxClaimResPerProcess a global value
    //NOTE: avail array is the number of available resources per type
    /*NOTE: max is double array holding the max # of units of resource R that process P will request
            Thus max is #Processes wide x #ResourceType deep*/
int **need, **hold;

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
   
    //initialize max array
    allocate2DArr(max,numProcesses,numResourceType);

    freedom(0);
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
void allocate2DArr(int **arr, int rows, int cols) {//TODO: delete if not needed

 //allocate base ptr;
    arr = (int **) malloc(sizeof(int*)*rows);
    if(arr==NULL){
        printf("ERROR: failed to allocate memory for 2D vector\n");
        freedom(-1); 
        return -1;
    }
    //allocate second layer of pointers
    for(int i=0; i<rows; i++){
        arr[i] = (int *) malloc(sizeof(int)*cols);
        if(arr[i]==NULL){
            printf("ERROR: failed to allocate memory for max vector\n");
            freedom(-1);
            return -1;
        }
    }
}
