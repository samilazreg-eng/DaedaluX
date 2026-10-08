bool x;
bool y;
bool z;

active proctype test_z()
{
	
	do
	:: z = !z;
	od;
}

active proctype test_y()
{
	
	do
	:: y = !y;
	od;
}

active proctype test_x()
{
	do
	:: z && y -> x = !x;
	od;
}

never { // !(G ((! x) -> F x)) 
T0_init :
	if
	:: (!x) -> goto accept_S2
	:: (1) -> goto T0_init

	fi;
accept_S2 :
	if
	:: (!x) -> goto accept_S2
	fi;
}

