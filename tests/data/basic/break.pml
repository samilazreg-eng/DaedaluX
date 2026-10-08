active proctype test(){
	
	do
	:: true -> break;
	:: true -> skip;
	od;
	
	assert(true);
}
