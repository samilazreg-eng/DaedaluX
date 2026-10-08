typedef features {
	bool B1;
	bool B2
}

features f;

system p1 = f.B1 && !f.B2;
//system p2 = f.B2 && !f.B1;

bool x, y, z;

active proctype foo() {
	
	do
	::	if 
		:: f.B1 -> x = !x; 
		:: else;
		fi;
	od;
}

active proctype bar() {
	
	do
	::	if 
		:: f.B2 -> y = !y; 
		:: else
		fi;
	od;
}
