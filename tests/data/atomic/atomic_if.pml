typedef features {
	bool Start;
	bool Stop;
 	bool MethaneAlarm;
 	bool MethaneQuery;
	bool Low;
	bool Normal;
	bool High
}

features f;



active proctype atom (){
	bool pcommand;
	bool running;
	
	do	::	atomic {
			
			
			
			pcommand = !pcommand;
			
			if	::	pcommand;
					if	::	f.Stop;
							if	:: 	running == true;
									running = false;
									
								::	else
							fi;
							
							pcommand = false;
						
						::	else
						fi;
						
				::	!pcommand;
					if	::	f.Start;
							if	::	running != true;
									running = true;
									
								::	else
								fi;
						::	else
						fi;
				fi;
			
			};
			
	od;
}

never {
	do::skip; od;
}
