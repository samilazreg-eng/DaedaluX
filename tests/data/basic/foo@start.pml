typedef features {
	bool B1;
	bool B2;
}

byte n, i;

features p1;
p1.B1 = true;
p1.B1 = true;

features p2;
p1.B1 = true;
p1.B1 = true;

features p3;
p1.B1 = true;
p1.B1 = true;

features p4;
p1.B1 = true;
p1.B1 = true;

active proctype foo() {
	do 
	:: break;
	:: n++;
	od;
	
Start:
	i = n;
	
	if :: f.B1 => i = i+2; else -> skip; fi;
	if :: f.B2 => i = i+1; else -> skip; fi;
	
Final:
	assert();
}
