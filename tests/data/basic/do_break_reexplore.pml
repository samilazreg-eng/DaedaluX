typedef features {
	bool B1;
	bool B2
}

features f;

active proctype foo() {

	bool x = false;
	bool y = false;

	if
	:: f.B1 -> x = !x;
	:: else -> skip;
	fi;
	
	if
	:: f.B2 -> y = !y;
	:: else -> skip;
	fi;
	
	do
	:: true;
	od;	
	
}

never { // !(G ((! x) -> F x)) 
T0_init :
	if
	:: (!foo.x) -> goto accept_S2
	:: (1) -> goto T0_init

	fi;
accept_S2 :
	if
	:: (!foo.x) -> goto accept_S2
	fi;
}
