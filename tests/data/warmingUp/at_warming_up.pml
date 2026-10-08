system p1;
system p2;

active proctype foo() {

Start:
	skip;

Final:
	skip;
}

never { /* x -> F y */
T0_init :    /* init */
	if
	:: (1) -> goto T0_S1
	:: (p1.foo@Final && p2.foo@Final) || (!(p1.foo@Start && p2.foo@Start)) -> goto accept_all
	fi;
T0_S1 :    /* 1 */
	if
	:: (1) -> goto T0_S1
	:: (p1.foo@Final && p2.foo@Final) -> goto accept_all
	fi;
accept_all :    /* 2 */
	skip
}
