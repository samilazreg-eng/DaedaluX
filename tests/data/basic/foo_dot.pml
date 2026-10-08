active proctype foo() {
	byte n, i;
	do 
	:: break;
	:: n++;
	od;
}

active proctype test() {
	assert(foo.n >= foo.i);
}
