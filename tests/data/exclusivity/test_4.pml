chan test = [0] of {bool, bool}

active proctype sender() {

	bool a;
	bool b;
	
	atomic {
		a = true;
		b = true
		test!a, b;
		b = false;
		a = false;
	}
}

active proctype receiver() {

	bool a;
	bool b;
	
	atomic {
		test?a, b;
		b = true;
		a = false;
	}
}

