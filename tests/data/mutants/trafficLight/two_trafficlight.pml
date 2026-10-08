mtype = {green, yellow, red}

mtype state1 = red;
mtype state2 = green;

active proctype light1() {
	do
	:: state1 == red  && state2 == yellow -> state1 = green;
	:: state1 == green && state2 == red -> state1 = yellow;
	:: state1 == yellow && state2 == green -> state1 = red;
	od;
}

active proctype light2() {
	do
	:: state2 == red && state1 == yellow -> state2 = green;
	:: state2 == green && state1 == red -> state2 = yellow;
	:: state2 == yellow && state1 == green -> state2 = red;
	od;
}
