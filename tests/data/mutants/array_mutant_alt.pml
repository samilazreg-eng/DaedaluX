int array[4];
int i = 0;

active proctype test (){
	do
	:: i < 3 ->
		array[i] = i + 2;
		i++;
	:: else ->
		skip;
	od;
}