typedef struct {
	bool B1;
	bool B2;
	bool B3;
	bool B4;
	bool B5;
	bool B6
}
struct f;

system p1;
system p2;

byte n;
short i;

active proctype foo(){
	do
	::break;
	::n++;
	od;

Start: i = n;
	if::true -> f.B1 = true ->i=i+6::true -> f.B1 = false ;fi;
	if::true -> f.B2 = true ->i=i+5::true -> f.B1 = false ;fi;
	if::true -> f.B3 = true ->i=i+4::true -> f.B1 = false ;fi;
	if::true -> f.B4 = true ->i=i+3::true -> f.B1 = false ;fi;
	if::true -> f.B5 = true ->i=i+2::true -> f.B1 = false ;fi;
	if::true -> f.B6 = true ->i=i+1::true -> f.B1 = false ;fi;

Final: skip;
	do
	::true;
	od;
}

/*// phi_1
never{
T0_init:
	if
	::((!p1.foo@Final)||(!p2.foo@Final)||(!(p1.i>=p2.i)))&&((p1.foo@Start)&&(p2.foo@Start)&&(p1.n==p2.n))->goto accept_S2;
	::(1)->goto T0_init;
	fi;
accept_S2:
	if
	::(!p1.foo@Final)||(!p2.foo@Final)||(!(p1.i>=p2.i))->goto accept_S2;
	fi;
}*/

// \phi_2
never{
accept_init:
	if
	:: !(p1.foo@Start) || !(p2.foo@Start) || !(p1.n==p2.n) || (p1.foo@Final && p2.foo@Final && p1.i>=p2.i) -> goto accept_init
	:: 1 -> goto T0_S2
	fi;
	
T0_S2:
	if
	:: (p1.foo@Final && p2.foo@Final && p1.i>=p2.i) -> goto accept_init
	:: 1 -> goto T0_S2
	fi;
}
