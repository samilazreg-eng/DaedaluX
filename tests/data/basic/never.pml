bool x;

active proctype n() { // (! x) -> X (!x) 
accept_init :    // init
	if
	:: (x) -> goto accept_all
	:: (1) -> goto accept_S1
	fi;
	
accept_S1 :    // 1 
	if
	:: (!x) -> goto accept_all
	fi;
accept_all :    // 2 
	skip
}
