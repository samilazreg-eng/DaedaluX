mtype = {green, yellow, red}

active proctype foo() {
	
	mtype state = red;
	do
	:: state == red -> state = green;
	:: state == green -> state = yellow;
	:: state == yellow -> state = red;
	:: skip;
	od;
}


active proctype bar() {
	
	mtype state = red;
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
	:: !(foo.state == green) -> goto accept_S2
	fi;
accept_S2 :
	if
	:: !(foo.state == green) -> goto accept_S2
	fi;
}
