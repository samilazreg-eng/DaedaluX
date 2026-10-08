typedef features {
	bool B1;
	bool B2
}

features f;

system p1 = f.B1 && !f.B2;
system p2 = f.B2 && !f.B1;

bool x;

active proctype test()
{
Start:
	do
	:: f.B1 -> x = false;
	:: f.B2 -> x = true;
	:: break;
	od;
Final:
	skip;
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
