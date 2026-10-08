typedef features {
	bool B1;
	bool B2;
	bool B3;
	bool B4;
	bool B5;
	bool B6
}
features f;

system p1 = f.B1;
system p2 = f.B2;

byte n;
short i;

active proctype foo(){
	do
	::break;
	::n++;
	od;

Start: i = n;
	if::f.B1->i=i+6::else;fi;
	if::f.B2->i=i+5::else;fi;
	if::f.B3->i=i+4::else;fi;
	if::f.B4->i=i+3::else;fi;
	if::f.B5->i=i+2::else;fi;
	if::f.B6->i=i+1::else;fi;

Final: skip;
	do
	::true;
	od;
}

#define p1_foo_at_start p1.foo@Start
#define p2_foo_at_start p2.foo@Start
#define p1_foo_at_final p1.foo@Final
#define p2_foo_at_final p2.foo@Final
#define p1_n_eq_p2_n (p1.n == p2.n)
#define p1_i_geq_p2_i (p1.i >= p1.i)

never { // (G((p1_foo_at_start && p2_foo_at_start && p1_n_eq_p2_n) && G!(p1_foo_at_final && p2_foo_at_final && p1_i_geq_p2_i )))
accept_init:
	if
	:: (p1_foo_at_start && p2_foo_at_start && p1_n_eq_p2_n && !p1_foo_at_final) || (p1_foo_at_start && p2_foo_at_start && p1_n_eq_p2_n && !p2_foo_at_final) || (p1_foo_at_start && p2_foo_at_start && p1_n_eq_p2_n && !p1_i_geq_p2_i) -> goto accept_init
	fi;
}

/*never { // !(G((p1_foo_at_start && p2_foo_at_start && p1_n_eq_p2_n) && G!(p1_foo_at_final && p2_foo_at_final && p1_i_geq_p2_i ))) 
T1_init :    
	if
	:: (!p1_foo_at_start) || (!p2_foo_at_start) || (p1_foo_at_final && p1_i_geq_p2_i && p2_foo_at_final) || (!p1_n_eq_p2_n) -> goto accept_all
	:: (1) -> goto T1_init
	:: (1) -> goto T0_S2
	fi;
T0_S2 :   
	if
	:: (1) -> goto T0_S2
	:: (p1_foo_at_final && p1_i_geq_p2_i && p2_foo_at_final) -> goto accept_all
	fi;
accept_all :
	skip
}*/

//!([]((p1.foo@Start && p2.foo@Start && p1.n == p2.n) -> <>(p1.foo@Final && p2.foo@Final && p1.i >= p2.i )))
//(<>((p1.foo@Start && p2.foo@Start && p1.n == p2.n) && [](NOT p1.foo@Final || NOT p2.foo@Final || NOT p1.i >= p2.i )))

