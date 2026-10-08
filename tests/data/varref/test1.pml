mtype = {green, yellow, red}

mtype state = red;

active proctype foo() {
	
	do
	:: state == red -> state = green;
	:: state == green -> state = yellow;
	:: state == yellow -> state = red;
	:: skip;
	od;
}

never { /* !(GF state == green) */
T0_init :
	if
	:: (1) -> goto T0_init
	:: !(state == green) -> goto accept_S2
	fi;
accept_S2 :
	if
	:: !(state == green) -> goto accept_S2
	fi;
}
