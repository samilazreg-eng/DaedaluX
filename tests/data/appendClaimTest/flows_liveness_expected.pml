typedef structure {
	bool a;
	bool b;
	bool c;
	bool d
}

structure s;

active proctype I(){

	s.a = true;
	s.b = true;
	
	do
	::  s.c = false;
	::  s.d = false;
	::  !s.c && !s.d 
		-> break
	
	:: s.c = true;
	:: s.d = true;
	:: s.c && s.d 
		-> break
	od;
	
	assert((s.c && s.d) || (!s.c && !s.d));
}

active proctype J(){
	do
	:: s.a = !s.a
	:: s.b = !s.b		
	:: s.a || s.b 
		-> break
	od;
	
	assert(s.a || s.b);
}

# 25 "[]((!(x)) -> <>x)" 
never { /* !([]((!(x)) -> <>x)) */
T0_init:
	if
	:: (1) -> goto T0_init
	:: (!x) -> goto accept_S2
	fi;
accept_S2:
	if
	:: (!x) -> goto accept_S2
	fi;
}
