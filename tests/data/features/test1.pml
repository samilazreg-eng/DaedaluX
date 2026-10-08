typedef features {
	bool B1;
	bool B2	
}

features f;

active proctype test (){
	bool B1, B2;
	if :: f.B1 -> B1 = true; :: else; fi;
	if :: f.B2 -> B2 = true; :: else; fi;
}
