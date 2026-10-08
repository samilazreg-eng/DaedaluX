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
	bool b1;
	bool test;
	
	if
	:: !b1 ->
		if 
		:: f.B1 ->
			atomic {
				flag = 1;
				b1 = true;
			}
			test = true;	
		:: else;
		fi;
	:: else;
	fi;	
}
