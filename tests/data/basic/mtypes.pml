mtype = {low, high}

mtype waterLevel = low;

active proctype test() {
	waterLevel = high;
	assert(waterLevel == high)
	assert(waterLevel != low)
}
