# OS-A3
## Questions

### Q2
1. **Default direction**
Scan has been updated to move in direction of which half of the disk the start sector is. E.g. if start is in Left (_i.e._ < Max-Min/2) move left. 
**However** cscan I'm by default moving to the right (up) regardless if all blocks are initially before me. _if_ i update it to choose initial direction should it still jump to opposite end. or always jump to 0?
Ans: by default go in direction of the half in
THE LECTURE NOTES CONTRADICT TUTORIAL NOTES. SLIDE 67. 
    the c-scan ought to initially move to left. 




2. **Possible INput values**
Do we need to deal with input validation (_e.g._ sector > max) _and_ do we also need to deal with inputs equal to start or max or min. I.e. do we need to worry about double counting?
    a. What to do with duplicate inputs. Since the disk is spinning I would assume that after the first is addressed it wouldn't be possile to revisit untill some other sector is read. 

### Q3
#### 3.1
1. **Max Resource Claim per process / resource**
    The maximum number of claims a process can make for that resource. 
    Does that mean then that the number of claims arent based on process-by-process basis, but rather a resource-by-resource basis. Thus a process can make varying 

2. **Allocation**
    Do we allocate as we go in the bankers for loop. e.g. if the first request is safe for process 1, do we allocate it immediately then check for resource 2? or do we only allocate all resources when we know all requests will be safe?   
        if _Yes_: Then how do we deal with thread synchronization? would all other threads have to wait until bankers algorithm is done on one process, at which point we shouldn't be using threads.  
    
    Global shared variable. 
    entire bankers algorithm protected by mutex. 

3. **Protection**
    How many semaphores and mutex do i need?

4. **General**
    if only reading from a global variable on multiple threads, is having it unprotected safe? Since i'm not changing it there shouldn't be any issue right?
        Note: if not pass _numResourceTypes_ locally via method calls. 
