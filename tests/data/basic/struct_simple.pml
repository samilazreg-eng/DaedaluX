typedef vector {
	int x;
	int y;
	int z
}

vector v;

active proctype test (){
	v.x = -1;
	v.y =  0;
	v.z =  1;

	assert(v.x == -1);
	assert(v.y ==  0);
	assert(v.z ==  1);
	
	//assert((v.x * v.y * v.z) == 0);
}
