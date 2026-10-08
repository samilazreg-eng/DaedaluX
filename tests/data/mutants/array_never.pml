int array[4];
int i = 0;

active proctype test (){
	do
	:: i < 4 ->
		array[i] = i;
		i++;
	:: else ->
		skip;
	od;
	assert(array[0] == 0 && array[1] == 1 && array[2] == 2 && array[3] == 3);
}

#define a (test.i < 5)

never {    /* [](IUPPERBOUND) */
accept_init :    /* init */
T0_init :    /* init */
	if
	:: (! (a)) -> goto T0_init
	fi;
}