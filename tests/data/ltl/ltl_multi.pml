system p1;
system p2;

bool x;

active proctype test()
{
	do
	:: x = false;
	:: x = true;
	od;
}

never { // !(G ((! x) -> F x)) 
T0_init :
	if
	:: p1.x != p2.x -> goto accept_S2
	:: (1) -> goto T0_init

	fi;
accept_S2 :
	if
	:: p1.x != p2.x -> goto accept_S2
	fi;
}
