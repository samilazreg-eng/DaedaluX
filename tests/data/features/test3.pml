typedef features {
	bool B1;
	bool B2	
}

features f;

active proctype test (){
	byte flag;
	bool b1, b2;
	
	if
	:: f.B1 -> b1 = true;
		if
		:: f.B2 -> b2 = true;
		:: else -> flag = 2; 
		fi;
	:: else -> flag = 1
	fi;	
}

//!B1 -> flag == 1
//B1 && !B2 -> b1 == true && flag == 2
//B1 && B2 -> b1 == true && b2 == true
 
