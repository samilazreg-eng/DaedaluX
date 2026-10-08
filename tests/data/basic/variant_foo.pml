typedef features {
	bool B1;
	bool B2
}

features f;

system p1;
system p2;

active proctype foo() {

	byte n;
	short i;

	if 
	:: n++;
	:: skip;
	fi;
	
Start:
	i = n;
	
	if :: f.B1 -> i = i+2; :: else -> skip; fi;
	if :: f.B2 -> i = i+1; :: else -> skip; fi;
	
Final:
	assert(i == n + 3);
}

/*active proctype test() {
	do
	:: foo@Start;
		do
		:: foo.i == foo.n -> break;
		:: foo@Final -> assert(false);
		od;
		
	:: foo@Final -> assert(false);
	 
	:: true;
	od;
}*/
