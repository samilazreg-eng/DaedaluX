typedef features {
	bool B1;
	bool B2;
	bool B3;
	bool B4;
	bool B5
}

features f;

active proctype test (){
	byte flag;
	
	if
	:: f.B1 -> flag = 123;
		if 
		:: f.B2 ->
			if
			:: f.B3 -> flag = 3;
			:: else -> flag = 2;
			fi;
		:: else -> flag = 1;
		fi;
	:: else;
	fi;	
}

