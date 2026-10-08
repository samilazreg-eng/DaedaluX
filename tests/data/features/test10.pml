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
	::	f.B1;
		
		if
		::	flag = 1;
		
		:: 	flag = 2;
		
		::	else
		fi;
		
		flag = 0
			
	::	else
	fi;	
}
