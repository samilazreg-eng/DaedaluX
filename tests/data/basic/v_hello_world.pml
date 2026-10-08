typedef features {
	bool B1
}

features f;

system s1;
system s2;

active proctype foo() {

	bool b1;
	
	if 
	:: f.B1 -> b1 = true;
	:: else -> skip; 
	fi;
}
