active proctype test(){
	
	bool b = false;
	
	do
	:: true -> 
		b = true;
		break;
	od;
	
	assert(b && b == true);
}
