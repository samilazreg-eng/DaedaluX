#define readCommand (readMsg == commandMsg)
#define readAlarm (readMsg == alarmMsg)
#define readLevel (readMsg == levelMsg)
#define userStart (uwants == start)
#define userStop (uwants == stop)
#define highWater (waterLevel == high)
#define mediumWater (waterLevel == medium)
#define lowWater (waterLevel == low)
#define stateReady (pstate == ready)
#define stateRunning (pstate == running)
#define stateStopped (pstate == stopped)
#define stateMethanestop (pstate == methanestop)
#define stateLowstop (pstate == lowstop)

mtype = {levelMsg, stop, methanestop, alarm, running, commandMsg, start, alarmMsg, high, low, stopped, medium, ready, lowstop}
chan cCmd = [0] of {mtype};
chan cAlarm = [0] of {mtype};
chan cMethane = [0] of {mtype};
chan cLevel = [0] of {mtype};
mtype pstate = stopped;
mtype readMsg = commandMsg;
bool pumpOn = false;
bool methane = false;
mtype waterLevel = medium;
mtype uwants = stop;
active proctype controller(){
	mtype pcommand = start;
	mtype level = medium;
	do
	::	atomic {
			cCmd?pcommand;
			readMsg = commandMsg;
		};
		
		if
		::	pcommand == stop;
			if
			::	atomic {
					pstate == running;
					pumpOn = false;
				};
			::	else;
			fi;
			pstate = stopped;
			
		::	pcommand == start;
			if
			::	atomic {
					pstate != running;
					pstate = ready;
				};
			::	else;
			fi;
		fi;
		cCmd!pstate; 
	
	::	atomic {
			cAlarm?_;
			readMsg = alarmMsg;
		};
		if
		::	atomic {
				pstate == running;
				pumpOn = false;
			};
		::	else;
		fi;
		pstate = methanestop;
		
	::	atomic {
			cLevel?level;
			readMsg = levelMsg;
		};
		if
		::	level == high;
			if
			::	pstate == ready || pstate == lowstop;
				atomic {
					cMethane!pstate;
					cMethane?pstate;
					if
					::	pstate == ready;
						pstate = running;
						pumpOn = true;
					::	else;
					fi;
				};
			::	else;
			fi;
			
		::	level == low;
			if
			::	atomic {
					pstate == running;
					pumpOn = false;
					pstate = lowstop;
				};
			::	else;
			fi;
		::	level == medium;
		fi;
	od;
}
active proctype user(){
	do
	::	if
		::	uwants = start;
		::	uwants = stop;
		fi;
		cCmd!uwants;
		cCmd?_;
	od;
}
active proctype methanealarm(){
	do
	::	methane = true;
		cAlarm!alarm;
	::	methane = false;
	od;
}
active proctype methanesensor(){
	do
	::	atomic {
			cMethane?_;
			if
			::	methane;
				cMethane!methanestop;
			::	!methane;
				cMethane!ready;
			fi;
		};
	od;
}
active proctype watersensor(){
	do
	::	atomic {
			if
			::	waterLevel == low;
				if
				::	waterLevel = low;
				::	waterLevel = medium;
				fi;
			::	waterLevel == medium;
				if
				::	waterLevel = low;
				::	waterLevel = medium;
				::	waterLevel = high;
				fi;
			::	waterLevel == high;
				if
				::	waterLevel = medium;
				::	waterLevel = high;
				fi;
			fi;
			cLevel!waterLevel;
		};
	od;
}

//never { /* G ((highWater && !methane) -> FpumpOn) */
//accept_init :    /* init */
//	if
//	:: (!(waterLevel == high)) || (methane) || (pumpOn) -> goto accept_init
//	:: (1) -> goto T0_S2
//	fi;
//T0_S2 :    /* 1 */
//	if
//	:: (1) -> goto T0_S2
//	:: (pumpOn) -> goto accept_init
//	fi;
//}
