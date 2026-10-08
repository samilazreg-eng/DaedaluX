int i;
int j;

active proctype test () {
	

begin:
	skip;
	if
	:: true -> 
		i++; 
		j--; 
		goto begin;
		
	:: i != 0 && j != 0 -> break;
	fi;
	
	assert(i == 1);
}

active proctype atom () {
	do
	:: atomic { i++; j--; }
	:: i != 0 && j != 0 -> break;
	od;
	
	assert(i + j == 0); 
}
