active proctype foo(){
	byte n;
	n = 0;
Start: 
	n++;
	goto Start;
}

active proctype test(){
	do
	:: foo@Start -> break;
	:: timeout -> 
		assert(false);
		break;
	od;
	
	assert(foo.n > 0);
}
