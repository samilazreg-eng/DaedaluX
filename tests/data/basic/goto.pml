active proctype test () {
	int i;
	i = 0;
begin:
	skip;
	if
	:: i == 0 -> i++;
	:: true -> goto begin;
	fi;
	
	assert(i == 1);
}
