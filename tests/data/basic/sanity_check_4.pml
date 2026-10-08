int i = 0;

active proctype I (){
	if
	:: i++ -> skip;
	:: i-- -> skip;
	:: else -> skip;
	fi;
}

active proctype J (){
	if
	:: i++ -> skip;
	:: i-- -> skip;
	:: else -> skip;
	fi;
}
