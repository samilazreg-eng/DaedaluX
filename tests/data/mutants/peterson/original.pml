bool turn, flag[2];
byte cnt;
active proctype P1(){
	pid i = 0;
	pid j = 1;
	do
	:: true;
		flag[i] = true;
		turn = i;
		!(flag[j] && turn == i);
		cnt++;
		assert(cnt == 1);
		cnt--;
		flag[i] = false;
	od;
}
active proctype P2(){
	pid i = 1;
	pid j = 0;
	do
	:: true;
		flag[i] = true;
		turn = i;
		!(flag[j] && turn == i);
		cnt++;
		assert(cnt == 1);
		cnt--;
		flag[i] = false;
	od;
}
