# OS-A3
##Questions

###Q2
1. **Default direction**
Scan has been updated to move in direction of which half of the disk the start sector is. E.g. if start is in Left (_i.e._ < Max-Min/2) move left. 
**However** cscan I'm by default moving to the right (up) regardless if all blocks are initially before me. _if_ i update it to choose initial direction should it still jump to opposite end. or always jump to 0?

2. **Possible INput values**
Do we need to deal with input validation (_e.g._ sector > max) _and_ do we also need to deal with inputs equal to start or max or min. I.e. do we need to worry about double counting?
    a. What to do with duplicate inputs. Since the disk is spinning I would assume that after the first is addressed it wouldn't be possile to revisit untill some other sector is read. 
