
active proctype foo() {

	bool a;
	bool b;
	if
	:: atomic {
		a = true;
		b = true;
	}
	:: a = true;
	:: b = true;
	fi;
}

active proctype bar() {

	bool a;
	bool b;
	if
	:: atomic {
		a = true;
		b = true;
	}
	:: a = true;
	:: b = true;
	fi;
}

