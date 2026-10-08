typedef features {
	bool B1;
	bool B2
}

features f;

byte n, i;

active proctype foo() {
	
	/*do
	:: break;
	:: n++;
	od;*/
	
Start:
	skip;
	if
	:: f.B1 -> i = i + 1;
	:: else -> skip;
	fi;
	
	/*if
	:: f.B2 -> i = i + 2;
	:: else -> skip;
	fi;*/
	
Final:
	skip;
}
