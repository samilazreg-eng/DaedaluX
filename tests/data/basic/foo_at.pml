typedef structure {
	bool B1;
	bool B2
}

active proctype foo() {

	byte n, i;

	structure f;
	f.B1 = true;
	f.B2 = true;

	if 
	:: n++;
	:: else -> skip;
	fi;
	
	i = n;
	
	skip;
	
	if :: f.B1 -> i = i+2; :: else -> skip; fi;
	if :: f.B2 -> i = i+1; :: else -> skip; fi;

	skip;
Start:
	skip;	
Final:
	assert(i == n + 3);
}

active proctype test() {
	do
	:: foo@Start;
		do
		:: foo.i == foo.n -> break;
		:: foo@Final -> assert(false);
		od;
		
	:: foo@Final -> assert(false);
	 
	:: true;
	od;
}
