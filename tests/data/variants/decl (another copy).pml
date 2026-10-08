variant p1;
variant p2;

typedef features {
	bool B1;
	bool B2
}

features f;

active proctype foo() {

	byte n, i;

	do 
	:: break;
	:: n++;
	od;
	
	i = n;
	
	p1.B1 = true;
	p2.B2 = true;
	
	if :: f.B1 -> i = i+2; :: else -> skip; fi;
	if :: f.B2 -> i = i+1; :: else -> skip; fi;
	
	assert(p1.i >= p2.i);
}

