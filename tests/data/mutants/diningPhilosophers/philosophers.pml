#define philosopher_0_eats (fork[0] == 0 && fork[1] == 0)
#define philosopher_1_eats (fork[1] == 1 && fork[2] == 1)
#define philosopher_2_eats (fork[2] == 2 && fork[3] == 2)
#define philosopher_3_eats (fork[3] == 3 && fork[4] == 3)

ltl no_starvation_0 { []<> philosopher_0_eats }
ltl no_starvation_1 { []<> philosopher_1_eats }
ltl no_starvation_2 { []<> philosopher_2_eats }
ltl no_starvation_3 { []<> philosopher_3_eats }

#define N 4
#define FREE 255

int fork[N] = {FREE, FREE, FREE, FREE};
chan waiter = [1] of {int};

active proctype philosopher_0() {
    do
    :: 
       waiter!0; // request permission to eat
       waiter?0; // wait for permission to eat
       atomic {
           // get left and right forks
           fork[0] = 0;
           fork[1] = 0;
       };
       // eat
       // put back forks
       atomic {
           fork[0] = FREE;
           fork[1] = FREE;
       };
       waiter!0; // signal done eating
    od;
}

active proctype philosopher_1() {
    do
    :: 
       waiter!1; // request permission to eat
       waiter?1; // wait for permission to eat
       atomic {
           // get left and right forks
           fork[1] = 1;
           fork[2] = 1;
       };
       // eat
       // put back forks
       atomic {
           fork[1] = FREE;
           fork[2] = FREE;
       };
       waiter!1; // signal done eating
    od;
}

active proctype philosopher_2() {
    do
    :: 
       waiter!2; // request permission to eat
       waiter?2; // wait for permission to eat
       atomic {
           // get left and right forks
           fork[2] = 2;
           fork[3] = 2;
       };
       // eat
       // put back forks
       atomic {
           fork[2] = FREE;
           fork[3] = FREE;
       };
       waiter!2; // signal done eating
    od;
}

active proctype philosopher_3() {
    do
    :: 
       waiter!3; // request permission to eat
       waiter?3; // wait for permission to eat
       atomic {
           // get left and right forks
           fork[3] = 3;
           fork[0] = 3;
       };
       // eat
       // put back forks
       atomic {
           fork[3] = FREE;
           fork[0] = FREE;
       };
       waiter!3; // signal done eating
    od;
}

active proctype waiter_process() {
    int req;
    do
    :: waiter?req ->
        if
        :: req == 0 && fork[0] == FREE && fork[1] == FREE -> waiter!0;
        :: req == 1 && fork[1] == FREE && fork[2] == FREE -> waiter!1;
        :: req == 2 && fork[2] == FREE && fork[3] == FREE -> waiter!2;
        :: req == 3 && fork[3] == FREE && fork[0] == FREE -> waiter!3;
        :: else -> skip;
        fi;
    od;
}
