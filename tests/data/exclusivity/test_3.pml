chan test = [0] of {bool, bool}

active proctype sender() {

	bool a;
	bool b;
	
	atomic {
		a = true;
		test!a, b;
		b = true;
	}
}

active proctype receiver() {

	bool a;
	bool b;
	
	test?a, b;
	b = true;
}

