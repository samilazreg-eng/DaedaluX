bool x;
bool y;
bool z;

active proctype test_z()
{
	do
	:: z = !z;
	od;
}

active proctype test_y()
{
	
	do
	:: y = !y;
	od;
}

active proctype test_x()
{
	do
	:: z && y -> x = !x;
	od;
}