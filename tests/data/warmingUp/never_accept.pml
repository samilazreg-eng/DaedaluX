active proctype foo(){
	do
	::true;
	od;
}


never{
accept_init:
	if
	:: 1 -> goto accept_init
	:: 1 -> goto T0_S2
	fi;
	true;
	
T0_S2:
	if
	:: 1 -> goto T0_S3
	:: 1 -> goto accept_init
	fi;

	false;
	
T0_S3:
	if
	:: 1 -> goto T0_S2
	:: 1 -> goto accept_init
	:: 1 -> goto accept_all
	fi;
	
	true;
	
accept_all:
	skip;
	false;
}

/*
never{
accept_init:
	if
	:: !(p1.foo@Start) || !(p2.foo@Start) || !(p1.n==p2.n) || (p1.foo@Final && p2.foo@Final && p1.i>=p2.i) -> goto accept_init
	:: 1 -> goto T0_S2
	fi;
	
T0_S2:
	if
	:: 1 -> goto T0_S2
	:: (p1.foo@Final && p2.foo@Final && p1.i>=p2.i) -> goto accept_init
	fi;
}*/
/*
never{
T0_init:
	if
	::((!p1.foo@Final)||(!p2.foo@Final)||(!(p1.i>=p2.i)))&&((p1.foo@Start)&&(p2.foo@Start)&&(p1.n==p2.n))->goto accept_S2;
	::(1)->goto T0_init;
	fi;
accept_S2:
	if
	::(!p1.foo@Final)||(!p2.foo@Final)||(!(p1.i>=p2.i))->goto accept_S2;
	fi;
}*/
