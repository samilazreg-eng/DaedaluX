byte i = 0;
byte a = 0;
byte j = 0;
byte b = 0;
	
active proctype I (){
	
	do
	:: atomic {
		if
		:: i < 128 -> i = i + 1; 
		:: a < 128 -> a = a + 1;
		:: else;
		fi;
		}
	od;	
}

active proctype J (){
	
	do
	:: atomic { 
		if
		:: j > 0 -> j = j - 1; 
		:: b > 0 -> b = b - 1;
		:: else;
		fi;
		}
	od;		
}

active proctype test(){
	do
	::	assert(i == a && j == b);
	od;
}

