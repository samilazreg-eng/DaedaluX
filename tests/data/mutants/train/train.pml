
mtype = { appr, leave, go, stop, Empty, Notempty, add, rem, hd };

chan g    = [4] of { mtype, pid };
chan qg   = [0] of { mtype, pid };
chan q    = [0] of { mtype, pid };
chan t[4] = [0] of { mtype };

active [4] proctype train()
{
	assert(_pid >= 0 && _pid < 4);
Safe:	do
	:: g!appr(_pid);
Approaching:	if
		:: t[_pid]?go ->
			goto Start
		:: t[_pid]?stop
		fi;
Stopped:	t[_pid]?go;
Start:		skip; /* crossing */
Crossed:	g!leave(_pid)
	od
}

active proctype gate()
{	pid who;
Free:
	if
	:: qg?Empty(_) ->
		g?appr(who);
Add1:		q!add(who)
	:: qg?Notempty(who)
	fi;
	t[who]!go;
Occupied:
	do
	:: g?appr(who)  ->
		t[who]!stop;
Add2:		q!add(who)
	:: g?leave(who) ->
		q!rem(who);
		goto Free
	od
}

chan list = [4] of { pid };

active proctype queue()
{	pid who, x;
Start:
	if
	:: nempty(list) ->
		list?<who>;
		qg!Notempty(who)
	:: empty(list) ->
		qg!Empty(0)
	fi;
	do
	:: q?add(who) ->
		list!who
	:: q?rem(who) ->
Shiftdown:	list?x;
		assert(x == who);
		goto Start
	od
}