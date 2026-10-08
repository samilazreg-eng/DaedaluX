typedef features {
	bool B1
}

features f;

active proctype foo() {

	skip;

	if
	:: f.B1 -> 
		skip;
	:: else -> 
		skip;
	fi;
	
	skip;
}
