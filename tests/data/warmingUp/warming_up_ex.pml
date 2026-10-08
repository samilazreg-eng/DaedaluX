typedef features {
	bool B1;
	bool B2
}

features f;

system p1 = f.B1;
system p2 = f.B2;



active proctype foo() {

	do 
	:: break;
	:: n++;
	od;
	
	
Start:  i = n;
	
	if :: f.B1 -> i = i + 2; :: else ; fi;
	
	if :: f.B2 -> i = i + 1; :: else ; fi;
	
Final:  skip;

	do
	:: true;
	od;
}


