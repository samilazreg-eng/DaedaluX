typedef features {
	bool B1;
	bool B2
}

features f;

system p1 = f.B1;
system p2 = f.B2;

bool n, i;

active proctype foo() {

	do 
	:: n = true;
	:: break;
	od;
	
Start:
	
	if :: f.B1 -> i = n; :: else -> skip; fi;
	if :: f.B2 -> i = !n; :: else -> skip; fi;
	
Final:
	skip;
}

never { /* (p1.foo@Start && p2.foo@Start && p1.n == p2.n) -> F (p1.foo@Final && p2.foo@Final && p1.i == p2.i) */
T0_init :    /* init */
	if
	:: !(p1.n == p2.n) || (p1.foo@Final && p2.foo@Final && p1.i != p2.i) || !(p1.foo@Start && p2.foo@Start) -> goto accept_all
	:: (1) -> goto T0_S2
	fi;
T0_S2 :    /* 1 */
	if
	:: (1) -> goto T0_S2
	:: (p1.foo@Final && p2.foo@Final && p1.i != p2.i) -> goto accept_all
	fi;
accept_all :    /* 2 */
	skip
}
