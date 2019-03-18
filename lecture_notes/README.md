# Lecture notes
## Network Control... on the Cheap
+ Motivation: protect set of sys against ext attack
    1. the most network services (processes) machines run, the greater the risk
    due to increased attack surface

+ One Approach: Disable unnecessary network services
    1. But you have to know all services that are running (ps aux | grep _)
    2. Sometimes a few trsuted remote users still require access

+ Tricky to scale
    1. What happens with thousands of systems?
    2. may not all be identified (hard to track assets)
    3. doing machine by machine (manually... ugh)





