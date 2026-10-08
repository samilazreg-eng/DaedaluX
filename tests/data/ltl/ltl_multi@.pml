system p1;
system p2;

bool x;

active proctype test()
{
Start:	do
	:: x = false;
	:: x = true;
	:: break;
	od;

Final:	skip;

	do
	:: true;
	od;
}

never { // !(G ((! x) -> F x)) 
T0_init :
	if
	:: (p1.test@Final && p2.test@Final) && (p1.x != p2.x) -> goto accept_S2
	:: (1) -> goto T0_init
	fi;
	
accept_S2 :
	if
	:: (p1.test@Final && p2.test@Final) && (p1.x != p2.x) -> goto accept_S2
	fi;
}
