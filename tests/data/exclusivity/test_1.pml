
active proctype foo() {

	bool a;
	bool b;
	atomic {
		a = true;
		b = true;
	}
	
	//do :: true; od;
}

active proctype bar() {

	bool a;
	bool b;
	atomic {
		a = true;
		b = true;
	}
	
	//do :: true; od;
}

