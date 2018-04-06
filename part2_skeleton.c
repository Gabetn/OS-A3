#include <stdio.h>  //for printf and scanf
#include <stdlib.h> //for malloc

#define LOW 0
#define HIGH 199
#define START 153

//compare function for qsort
//you might have to sort the request array
//use the qsort function 
// an argument to qsort function is a function that compares 2 quantities
//use this there.
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

//function to swap 2 integers
void swap(int *a, int *b)
{
    if (*a != *b)
    {
        *a = (*a ^ *b);
        *b = (*a ^ *b);
        *a = (*a ^ *b);
        return;
    }
}

int *output;

//Prints the sequence and the performance metric
void printSeqNPerformance(int *request, int numRequest) //given an array print the elements in order as well as number of tracks traversed.
{
    int i, last, acc = 0;
    last = START;
    printf("\n");
    printf("%d", START);
    for (i = 0; i < numRequest; i++)
    {
        printf(" -> %d", request[i]);
        acc += abs(last - request[i]);
        last = request[i];
    }
    printf("\nPerformance : %d\n", acc);
    return;
}

//access the disk location in FCFS
void accessFCFS(int *request, int numRequest)
{
    //simplest part of assignment
    printf("\n----------------\n");
    printf("FCFS :");
    printSeqNPerformance(request, numRequest);
    printf("----------------\n");
    return;
}

//access the disk location in SSTF
void accessSSTF(int *request, int numRequest)
{
    //write your logic here
    printf("\n----------------\n");
    printf("SSTF :");
    int last,next, diff, numPrev, notUsed;
    int prev[numRequest]; //previous is a temporary array for holding indicies
    int min;
    numPrev=0;
    last = START;
    for(int i=0; i<numRequest; i++){ //for each possible output value
        min = HIGH;
        next=-1;
        for(int j=0; j<numRequest; j++){  //loop through all input values
            notUsed=1; //true
            diff = abs(last - request[j]);  //dist from current out to all potential next values
            
            if(diff<=min){
                for(int k=0; k<numPrev; k++){ //check if index was previously used
                    if(j == prev[k]){
                        notUsed = 0; //false          
                        break;
                    }
                }
                if(notUsed){
                    min=diff;
                    next=j;
                }
            } //if

        } //for j
        if(next!=-1){
            output[i] = request[next];
            prev[numPrev] = next;
            numPrev++;
        }else{
            printf("Error: Failed to select next value\n");
        }        
        last = output[i];
    }
    printSeqNPerformance(output, numRequest);
    printf("----------------\n");
    return;
}

//access the disk location in SCAN
void accessSCAN(int *request, int numRequest)
{
    
	//write your logic here
    printf("\n----------------\n");
    printf("SCAN :");
    
    qsort(request, numRequest, sizeof(int), &cmpfunc);
    int loc, start;
    int dir=1; //dir will be direction in which scan travels 1=right
    if( START<((HIGH-LOW)/2) ){ //if start on lhs
        dir = -1;
    }
    for(int i=0; i<numRequest; i++){
        if(request[i] >= START){
            loc = i;
            if(request[i] > START && dir==-1)
                loc--;
            break;
        }
        else if(i==numRequest-1){ //if start greater than all requests
            loc=numRequest;
            //dir = -1;
        }    
    }
    start=loc;
    
    int inCtr = 0, outCtr = 0, newCnt=numRequest;
    
    while(inCtr < numRequest){ 
        
        if(loc == numRequest){ //if need to turn around;
            if(dir==1){ //jump to high. 
                output = (int *) realloc(output, sizeof(int)); //add extra space
                output[outCtr] = HIGH;
                outCtr++;
                newCnt++;
            }
            dir = -1; //flips dir
            loc = start-1;

        }
        else if(loc == -1){
            if(dir==-1){ //jump to low. 
                output = (int *) realloc(output, sizeof(int)); //add extra space
                output[outCtr] = LOW;
                outCtr++;
                newCnt++;
            }
            dir = 1;
            loc = start+1;
        }
        else{
            output[outCtr] = request[loc];
            inCtr++;
            loc+=dir;
            outCtr++;
        }
    }

    
    printSeqNPerformance(output, newCnt);
    printf("----------------\n");
    return;
}

//access the disk location in CSCAN
void accessCSCAN(int *request, int numRequest)
{
    //write your logic here
    printf("\n----------------\n");
    printf("CSCAN :");

    qsort(request, numRequest, sizeof(int), &cmpfunc);
    int loc, start;
    int dir=1; //dir will be direction in which scan travels 1=right
    if( START<((HIGH-LOW)/2) ){ //if start on lhs
        dir = -1;
    }
    for(int i=0; i<numRequest; i++){
        if(request[i] >= START){
            loc = i;
            if(request[i] > START && dir==-1)
                loc--;
            break;
        }
        else if(i==numRequest-1){ //if start greater than all requests
            loc=numRequest;
            if(dir == -1)
                loc--;
        }    
    }
    start=loc;
    int inCtr = 0, outCtr = 0, newCnt=numRequest;
    while(inCtr < numRequest){ //TODO: should this switch dir or always right
       
        if(loc == numRequest && dir==1){ //if need to loop to 0; 
            output = (int *) realloc(output, sizeof(int)*2); //add extra space
            output[outCtr] = HIGH;
            output[outCtr+1] = LOW;
            loc = 0;
            /*else{ //happens if first input is above all others
                // output[outCtr] = LOW; //outs 0
                // output[outCtr+1] = HIGH;
                loc = numRequest-1;
            }*/
            outCtr+=2;
            newCnt+=2;
        }
        else if(loc == -1 && dir==-1){
            output = (int *) realloc(output, sizeof(int)*2); //add extra space
            output[outCtr] = LOW;
            output[outCtr+1] = HIGH;
            loc = numRequest-1;
            outCtr+=2;
            newCnt+=2;
        }
        else{
            output[outCtr] = request[loc];
            inCtr++;
            loc+=dir;
            outCtr++;
        }
    }

    printSeqNPerformance(output, newCnt);
    printf("----------------\n");
    return;
}

//access the disk location in LOOK
void accessLOOK(int *request, int numRequest)
{
    //write your logic here
    printf("\n----------------\n");
    printf("LOOK :");
    
    qsort(request, numRequest, sizeof(int), &cmpfunc);
    int loc, start;
    int dir = 1; //dir will be direction in which scan travels 1=right
    if( START<((HIGH-LOW)/2) ){ //if start on lhs
        dir = -1;
    }
    for(int i=0; i<numRequest; i++){
        if(request[i] >= START){
            loc = i;
            if(request[i] > START && dir==-1)
                loc--;
            break;
        }
        else if(i==numRequest-1)
            loc=numRequest;
    }
    start=loc;

    int inCtr = 0, outCtr = 0;
    while(inCtr < numRequest){ //TODO: should this switch dir or always right
        if(loc<0)
            printf("Error invalid location\n");
        
        if(loc == numRequest){ //if need to turn around;
            dir = -1;
            loc = start-1;
            if(loc<0)
                printf("Error invalid location\n");
        }
        else if(loc == -1){
            dir = 1;
            loc = start+1;
        }
        else{
            output[outCtr] = request[loc];
            inCtr++;
            loc+=dir;
            outCtr++;
        }
    }
        
    printSeqNPerformance(output, numRequest);
    printf("----------------\n");
    return;
}

//access the disk location in CLOOK
void accessCLOOK(int *request, int numRequest) 
{
    //write your logic here
    printf("\n----------------\n");
    printf("CLOOK :");

    qsort(request, numRequest, sizeof(int), &cmpfunc);
    int loc, start;
    int dir=1; //dir will be direction in which scan travels 1=right
    if( START<((HIGH-LOW)/2) ){ //if start on lhs
        dir = -1;
    }
    for(int i=0; i<numRequest; i++){
        if(request[i] >= START){
            loc = i;
            if(request[i] > START && dir==-1)
                loc--;
            break;
        }
        else if(i==numRequest-1){ //if start greater than all requests
            loc=numRequest;
            if(dir == -1)
                loc--;
        }    
    }
    start=loc;
    int inCtr = 0, outCtr = 0, newCnt=numRequest;
    while(inCtr < numRequest){ //TODO: should this switch dir or always right
       
        if(loc == numRequest && dir==1){ //if need to loop to 0; 
            output = (int *) realloc(output, sizeof(int)); //add extra space
            output[outCtr] = LOW;
            loc = 0;
            /*else{ //happens if first input is above all others
                // output[outCtr] = LOW; //outs 0
                // output[outCtr+1] = HIGH;
                loc = numRequest-1;
            }*/
            outCtr++;
            newCnt++;
        }
        else if(loc == -1 && dir==-1){
            output = (int *) realloc(output, sizeof(int)); //add extra space
            output[outCtr] = HIGH;
            loc = numRequest-1;
            outCtr++;
            newCnt++;
        }
        else{
            output[outCtr] = request[loc];
            inCtr++;
            loc+=dir;
            outCtr++;
        }
    }
    printSeqNPerformance(output,newCnt);
    printf("----------------\n");
    return;
}

int main()
{
    int *request, numRequest, i,ans; //NOTE: request is a pointer to malloc array of ints storing the request positions in ints

    //allocate memory to store requests
    printf("Enter the number of disk access requests : ");
    scanf("%d", &numRequest);
    request = malloc(numRequest * sizeof(int)); //note: shouldn't this be cast into a int * or does the assignment cast automatically
    output = malloc(numRequest * sizeof(int));
    printf("Enter the requests ranging between %d and %d\n", LOW, HIGH);
    for (i = 0; i < numRequest; i++)
    {
        scanf("%d", &request[i]);
        if(request[i]>HIGH || request[i]<LOW){ //NOTE: input validation
            printf("INVALID INPUT: input must be between %d and %d (inclusive)\n", LOW, HIGH);
            free((void*)request);
            free((void*)output);
            return -1;
        }
    }

    printf("\nSelect the policy : \n");
    printf("----------------\n");
    printf("1\t FCFS\n");
    printf("2\t SSTF\n");
    printf("3\t SCAN\n");
    printf("4\t CSCAN\n");
    printf("5\t LOOK\n");
    printf("6\t CLOOK\n");
    printf("----------------\n");
    scanf("%d",&ans);

    switch (ans)
    {
    //access the disk location in FCFS
    case 1: accessFCFS(request, numRequest); 
        break;

    //access the disk location in SSTF
    case 2: accessSSTF(request, numRequest);
        break;

        //access the disk location in SCAN
     case 3: accessSCAN(request, numRequest);
        break;

        //access the disk location in CSCAN
    case 4: accessCSCAN(request,numRequest);
        break;

    //access the disk location in LOOK
    case 5: accessLOOK(request,numRequest);
        break;

    //access the disk location in CLOOK
    case 6: accessCLOOK(request,numRequest);
        break;

    default:
        break;
    }
    free((void*)request);
    free((void*)output);
    return 0;
}
