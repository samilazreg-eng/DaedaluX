typedef features {
	bool B1;
	bool B2	
}

features f;

active proctype test (){
	byte flag;
	bool B1, B2;
	
	if
	:: f.B1 -> B1 = true;
	:: else -> flag = 1;
	fi;	
}
