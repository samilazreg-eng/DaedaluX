int i = 0;

active proctype test (){
	if
	:: i == 0 -> skip;
	:: true -> skip;
	:: else -> skip;
	fi;
}
