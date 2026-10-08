/* runspinsafe: %spin -a        %gcc -o pan -DSAFETY pan.c %./pan -m100000000   % */
#define N 2
// declaration and initial values of global variables
bool entering[N] = false;
short number[N] = 0;
byte ncrit = 0;
active proctype client_1() {
    do
    ::  entering[0] = true;
        byte i = 0;
        short max = number[0];
        do
        :: i < 2 ->
            max = (number[i] > max -> number[i] : max);
            i = i + 1;
        :: i >= 2 ->
            i = 0;
            break;
        od;
        number[0] = 1 + max;
        entering[0] = false;
        do
        :: i < 2 ->
            if
            :: !entering[i] && (number[i] == 0 || number[i] > number[0] || (number[i] == number[0] && i >= 0)) -> skip;
            fi;
            i = i + 1;
        :: i >= 2 ->
            i = 0;
            break;
        od;
        ncrit = ncrit + 1;
        // critical section
        assert(ncrit <= 1);
        ncrit = ncrit - 1;
        number[0] = 0;
    od;
}

active proctype client_2() {
    do
    ::  entering[1] = true;
        byte i = 0;
        short max = number[1];
        do
        :: i < 2 ->
            max = (number[i] > max -> number[i] : max);
            i = i + 1;
        :: i >= 2 ->
            i = 0;
            break;
        od;
        number[1] = 1 + max;
        entering[1] = false;
        do
        :: i < 2 ->
            if
            :: !entering[i] && (number[i] == 0 || number[i] > number[1] || (number[i] == number[1] && i >= 1)) -> skip;
            fi;
            i = i + 1;
        :: i >= 2 ->
            i = 0;
            break;
        od;
        ncrit = ncrit + 1;
        // critical section
        assert(ncrit <= 1);
        ncrit = ncrit - 1;
        number[1] = 0;
    od;
}
