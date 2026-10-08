mtype = {right, left, up, down}

mtype test = left;

active proctype foo() {
	test = up;
	assert(test == up);
}
