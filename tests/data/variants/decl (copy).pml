variant p1;
variant p2;

typedef features {
	bool B1;
	bool B2
}

features f;

active proctype foo() {

	byte i;

	do 
	:: break;
	:: n++;
	od;
	
	p1.B1 = true;
	p2.B2 = true;
	
Start:
	i = n;
	
	if :: f.B1 -> i = i+2; :: else -> skip; fi;
	if :: f.B2 -> i = i+1; :: else -> skip; fi;
	
Final:
	assert(i == n + 3);
}

active proctype test() {
	do
	:: p1.foo@Start && p2.foo@Start;
		
		do
		:: foo.i == foo.n -> break;
		:: p1.foo@Final || p2.foo@Final -> assert(false);
		od;
		
	:: foo@Final || p2.foo@Final -> assert(false);
	 
	:: true;
	od;
}
