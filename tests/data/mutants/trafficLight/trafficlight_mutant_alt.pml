mtype = {green, yellow, red}

mtype state = red;

active proctype foo() {
	
	do
	:: state == red -> state = green;
	:: state == yellow -> state = red;
	od;
}
