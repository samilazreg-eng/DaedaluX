mtype { p, v };

chan sema = [0] of { mtype };
byte count = 1;

active proctype Dijkstra()
{	
	do
	:: (count == 1) ->
		sema!p; count = 0
	:: (count == 0) ->
		sema?v; count = 1
	od	
}

active proctype userA()
{	do
	:: sema?p;	   /* enter */
          skip;	   /* leave */
	   sema!v;
	od
}

active proctype userB()
{	do
	:: sema?p;	   /* enter */
          skip;	   /* leave */
	   sema!v;
	od
}

active proctype userC()
{	do
	:: sema?p;	   /* enter */
          skip;	   /* leave */
	   sema!v;
	od
}
