chan test = [0] of {bool}

active proctype sender(){
	bool b = true;
	test!b;
	atomic {
		if
		:: b -> b = false;
		:: else;
		fi;x
		assert(b == 1);
	}
	do
	:: true;
	od;
}

active proctype receiver(){
	byte c = false;
	test?c;
	atomic {
		if
		:: c -> c = false;
		:: else;
		fi;
		assert(c == 1);
	}
	do
	:: true;
	od;
}
