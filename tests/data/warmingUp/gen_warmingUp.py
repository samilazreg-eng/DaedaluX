import sys

if __name__ == "__main__":
	
	F = int(sys.argv[1])
	Q = int(sys.argv[2])
	
	tvl = "root WarmingUp {\n\tgroup allOf {\n"
	for i in range (F-1):
		tvl += "\t\topt B" + str(i+1) + ",\n"
	tvl += "\t\topt B" + str(F) + "\n\t}\n}\n"
	
	features = "typedef features {\n"
	
	for i in range(F-1):
		features += "\tbool B"+ str(i+1) +";\n"
	features += "\tbool B"+ str(F) +"\n}\n"
	features += "features f;\n"
	
	systems = "\n"
	for i in range(Q):
		systems += "system p"+ str(i+1) +" = f.B"+ str(i+1) +";\n"
	
	start = "\nbyte n;\nshort i;\n\n"
	start += "active proctype foo(){\n"
	start += "\tdo\n\t::break;\n\t::n++;\n\tod;\n\n"
	start += "Start: i = n;\n"
	
	f_expr = ""
	
	for i in range(F):
		f_expr += "\tif::f.B" + str(i+1) + "->i=i+"+ str(F-i) + "::else;fi;\n"
		
	final = "\nFinal: skip;\n"
	final += "\tdo\n\t::true;\n\tod;\n}\n"
	
	never = "never{\n"
	never += "T0_init :\n"
	never += "\tif\n\t::"
	
	p_at_start = []
	p_at_final = []
	
	for i in range(Q):
		p_at_start.append("p"+str(i+1)+".foo@Start")
		p_at_final.append("p"+str(i+1)+".foo@Final")
		
	n_eq = []
	i_geq = []
	
	for i in range(Q-1):
		n_eq.append("p"+str(i+1)+".n==p"+str(i+2)+".n")
		i_geq.append("p"+str(i+1)+".i>=p"+str(i+2)+".i")
		
	accept_final = ""
	for i in range(Q):
		accept_final += "(!"+p_at_final[i]+")||"
	for i in range(len(i_geq)-1):
		accept_final += "(!("+i_geq[i]+"))||"
	accept_final += "(!("+i_geq[len(i_geq)-1]+"))"
	
	accept_start = ""
	for i in range(Q):
		accept_start += "("+p_at_start[i]+")&&"
	for i in range(len(n_eq)-1):
		accept_start += "("+n_eq[i]+")&&"
	accept_start += "("+n_eq[Q-2]+")"
	
	never += "("+ accept_final + ")&&(" + accept_start + ")->goto accept_S2;\n\t"
	never += "::(1)->goto T0_init;\n\tfi;\n"
	never += "accept_S2:\n\t"
	never += "if\n\t::" + accept_final + "->goto accept_S2;\n\tfi;\n}"
	
	tvl_file = open("warmingUp_"+ str(F) + "_" + str(Q) +".tvl", "w")
	tvl_file.write(tvl)
	tvl_file.close()
	
	promela_file = open("warmingUp_"+ str(F) +"_" + str(Q) +".pml", "w")
	promela_file.write(features + systems + start + f_expr + final + never)
	promela_file.close()
