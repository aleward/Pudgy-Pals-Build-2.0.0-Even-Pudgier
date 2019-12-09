#include "stdafx.h"
#include "Cases.h"


Case::Case(std::vector<Triangle*> tris, bool canBeAmb, int ambN) :
	triangles(tris), canBeAmbiguous(canBeAmb), ambNum(ambN)
{}

Cases::Cases() :
	map(mapReturn())
{}


Cases::~Cases()
{}

std::array<std::pair<int, vec3>, 256> Cases::mapReturn() {
	std::array<std::pair<int, vec3>, 256> temp = std::array<std::pair<int, vec3>, 256>();
	temp.fill(std::make_pair(-1, vec3(0, 0, 0)));

	// CASE 0
	temp[0]   = std::make_pair(0, vec3(0, 0, 0));
	temp[255] = std::make_pair(0, vec3(0, 0, 0));

	// CASE 1 - 8 of em
	temp[128] = std::make_pair(1, vec3(0, 0, 0));
	temp[64]  = std::make_pair(1, vec3(0, 90, 0));
	temp[32]  = std::make_pair(1, vec3(0, 180, 0));
	temp[16]  = std::make_pair(1, vec3(0, 270, 0));
	temp[8]   = std::make_pair(1, vec3(90, 0, 0));
	temp[4]   = std::make_pair(1, vec3(180, 0, 0));
	temp[2]   = std::make_pair(1, vec3(0, 270, 180));
	temp[1]   = std::make_pair(1, vec3(0, 0, 180));

	// CASE 2 - 12 of em
	temp[192] = std::make_pair(2, vec3(0, 0, 0));
	temp[96]  = std::make_pair(2, vec3(0, 90, 0));
	temp[48]  = std::make_pair(2, vec3(0, 180, 0));
	temp[144] = std::make_pair(2, vec3(0, 270, 0));
	temp[136] = std::make_pair(2, vec3(90, 0, 0));
	temp[12]  = std::make_pair(2, vec3(180, 0, 0));
	temp[68]  = std::make_pair(2, vec3(270, 0, 0));
	temp[17]  = std::make_pair(2, vec3(90, 0, 90));
	temp[34]  = std::make_pair(2, vec3(270, 0, 90));
	temp[3]   = std::make_pair(2, vec3(0, 0, 180));
	temp[9]   = std::make_pair(2, vec3(0, 90, 180));
	temp[6]   = std::make_pair(2, vec3(0, 270, 180));/**/

	// CASE 3 - 12 of em
	temp[132] = std::make_pair(3, vec3(0, 0, 0));
	temp[66]  = std::make_pair(3, vec3(0, 90, 0));
	temp[33]  = std::make_pair(3, vec3(0, 180, 0));
	temp[24]  = std::make_pair(3, vec3(0, 270, 0));
	temp[72]  = std::make_pair(3, vec3(90, 0, 0));
	temp[80]  = std::make_pair(3, vec3(0, 0, 90));
	temp[129] = std::make_pair(3, vec3(90, 0, 90));
	temp[10]  = std::make_pair(3, vec3(0, 0, 270));
	temp[5]   = std::make_pair(3, vec3(0, 90, 270));
	temp[160] = std::make_pair(3, vec3(0, 90, 90));
	temp[18]  = std::make_pair(3, vec3(90, 0, 180));
	temp[36]  = std::make_pair(3, vec3(270, 0, 90));

	// CASE 4 - 4 of em
	temp[130] = std::make_pair(4, vec3(0, 0, 0));
	temp[40]  = std::make_pair(4, vec3(90, 0, 0));
	temp[20]  = std::make_pair(4, vec3(180, 0, 0));
	temp[65]  = std::make_pair(4, vec3(270, 0, 0));

	// CASE 5 - 24 of em
	temp[112] = std::make_pair(5, vec3(0, 0, 0));
	temp[200] = std::make_pair(5, vec3(90, 0, 270));
	temp[140] = std::make_pair(5, vec3(180, 0, 270));
	temp[76]  = std::make_pair(5, vec3(270, 0, 270));
	temp[196] = std::make_pair(5, vec3(0, 0, 270));
	temp[224] = std::make_pair(5, vec3(0, 270, 0));
	temp[176] = std::make_pair(5, vec3(0, 90, 0));
	temp[208] = std::make_pair(5, vec3(0, 180, 0));
	temp[70]  = std::make_pair(5, vec3(0, 270, 90));
	temp[38]  = std::make_pair(5, vec3(270, 0, 0));
	temp[100] = std::make_pair(5, vec3(270, 180, 0));
	temp[98]  = std::make_pair(5, vec3(270, 90, 0));
	temp[145] = std::make_pair(5, vec3(90, 0, 0));
	temp[25]  = std::make_pair(5, vec3(90, 90, 0));
	temp[137] = std::make_pair(5, vec3(90, 180, 0));
	temp[152] = std::make_pair(5, vec3(90, 270, 0));
	temp[50]  = std::make_pair(5, vec3(90, 0, 90));
	temp[35]  = std::make_pair(5, vec3(0, 0, 90));
	temp[49]  = std::make_pair(5, vec3(180, 0, 90));
	temp[19]  = std::make_pair(5, vec3(270, 0, 90));
	temp[7]   = std::make_pair(5, vec3(0, 90, 180));
	temp[14]  = std::make_pair(5, vec3(0, 0, 180));
	temp[11]  = std::make_pair(5, vec3(0, 180, 180));
	temp[13]  = std::make_pair(5, vec3(0, 270, 180));/**/

	// CASE 6 - 24 of em
	temp[194] = std::make_pair(6, vec3(0, 0, 0));
	temp[28]  = std::make_pair(6, vec3(180, 0, 0));
	temp[52]  = std::make_pair(6, vec3(0, 0, 90));
	temp[44]  = std::make_pair(6, vec3(0, 0, 270));
	temp[56]  = std::make_pair(6, vec3(0, 180, 0));
	temp[131] = std::make_pair(6, vec3(180, 0, 90));
	temp[193] = std::make_pair(6, vec3(0, 180, 90));
	temp[67]  = std::make_pair(6, vec3(0, 0, 180));
	temp[168] = std::make_pair(6, vec3(90, 0, 0));
	temp[162] = std::make_pair(6, vec3(270, 180, 0));
	temp[42]  = std::make_pair(6, vec3(90, 180, 270));
	temp[138] = std::make_pair(6, vec3(270, 0, 270));
	temp[81]  = std::make_pair(6, vec3(90, 0, 90));
	temp[21]  = std::make_pair(6, vec3(90, 180, 0));
	temp[69]  = std::make_pair(6, vec3(270, 0, 0));
	temp[84]  = std::make_pair(6, vec3(270, 180, 90));
	temp[97]  = std::make_pair(6, vec3(0, 90, 0));
	temp[73]  = std::make_pair(6, vec3(90, 270, 0));
	temp[104] = std::make_pair(6, vec3(270, 270, 0));
	temp[41]  = std::make_pair(6, vec3(0, 0, 180));
	temp[148] = std::make_pair(6, vec3(0, 270, 0));
	temp[146] = std::make_pair(6, vec3(90, 90, 0));
	temp[22]  = std::make_pair(6, vec3(0, 90, 270));
	temp[134] = std::make_pair(6, vec3(180, 270, 0));

	// CASE 7 - 8 of em
	temp[74]  = std::make_pair(8, vec3(0, 0, 0));
	temp[37]  = std::make_pair(8, vec3(0, 90, 0));
	temp[26]  = std::make_pair(8, vec3(0, 180, 0));
	temp[133] = std::make_pair(8, vec3(0, 270, 0));
	temp[82]  = std::make_pair(8, vec3(0, 0, 180));
	temp[161] = std::make_pair(8, vec3(0, 90, 180));
	temp[88]  = std::make_pair(8, vec3(0, 180, 180));
	temp[164] = std::make_pair(8, vec3(0, 270, 180));

	// CASE 8 - 6 of em
	temp[240] = std::make_pair(8, vec3(0, 0, 0));
	temp[204] = std::make_pair(8, vec3(0, 0, 270));
	temp[51]  = std::make_pair(8, vec3(0, 0, 90));
	temp[153] = std::make_pair(8, vec3(0, 90, 90));
	temp[102] = std::make_pair(8, vec3(0, 270, 90));
	temp[15]  = std::make_pair(8, vec3(0, 0, 180));

	// CASE 9 - 8 of em
	temp[177] = std::make_pair(9, vec3(0, 0, 0));
	temp[216] = std::make_pair(9, vec3(0, 90, 0));
	temp[228] = std::make_pair(9, vec3(0, 180, 0));
	temp[114] = std::make_pair(9, vec3(0, 270, 0));
	temp[141] = std::make_pair(9, vec3(0, 0, 180));
	temp[78]  = std::make_pair(9, vec3(0, 90, 180));
	temp[39]  = std::make_pair(9, vec3(0, 180, 180));
	temp[27]  = std::make_pair(9, vec3(0, 270, 180));/**/

	// CASE 10 - 8 of em
	temp[170] = std::make_pair(10, vec3(0, 0, 0));
	temp[85]  = std::make_pair(10, vec3(0, 90, 0));
	temp[60]  = std::make_pair(10, vec3(90, 0, 0));
	temp[195] = std::make_pair(10, vec3(270, 0, 0));

	// CASE 11 - 8 of em
	temp[178] = std::make_pair(11, vec3(0, 0, 0));
	temp[116] = std::make_pair(11, vec3(0, 270, 0));
	temp[209] = std::make_pair(11, vec3(0, 90, 0));
	temp[232] = std::make_pair(11, vec3(0, 180, 0));
	temp[77]  = std::make_pair(11, vec3(0, 0, 180));
	temp[46]  = std::make_pair(11, vec3(0, 90, 180));
	temp[23]  = std::make_pair(11, vec3(0, 180, 180));
	temp[139] = std::make_pair(11, vec3(0, 270, 180));
	temp[99]  = std::make_pair(11, vec3(90, 0, 90));
	temp[57]  = std::make_pair(11, vec3(270, 0, 90));
	temp[156] = std::make_pair(11, vec3(90, 0, 270));
	temp[198] = std::make_pair(11, vec3(270, 0, 270));/**/

	// CASE 12 - 24 of em
	temp[120] = std::make_pair(12, vec3(0, 0, 0));
	temp[202] = std::make_pair(12, vec3(90, 0, 270));
	temp[172] = std::make_pair(12, vec3(180, 0, 270));
	temp[92]  = std::make_pair(12, vec3(270, 0, 270));
	temp[197] = std::make_pair(12, vec3(0, 0, 270));
	temp[225] = std::make_pair(12, vec3(0, 270, 0));
	temp[180] = std::make_pair(12, vec3(0, 90, 0));
	temp[210] = std::make_pair(12, vec3(0, 180, 0));
	temp[86]  = std::make_pair(12, vec3(0, 270, 90));
	temp[166] = std::make_pair(12, vec3(270, 0, 0));
	temp[101] = std::make_pair(12, vec3(270, 180, 0));
	temp[106] = std::make_pair(12, vec3(270, 90, 0));
	temp[149] = std::make_pair(12, vec3(90, 0, 0));
	temp[89]  = std::make_pair(12, vec3(90, 90, 0));
	temp[169] = std::make_pair(12, vec3(90, 180, 0));
	temp[154] = std::make_pair(12, vec3(90, 270, 0));
	temp[58]  = std::make_pair(12, vec3(90, 0, 90));
	temp[163] = std::make_pair(12, vec3(0, 0, 90));
	temp[53]  = std::make_pair(12, vec3(180, 0, 90));
	temp[83]  = std::make_pair(12, vec3(270, 0, 90));
	temp[135] = std::make_pair(12, vec3(0, 90, 180));
	temp[30]  = std::make_pair(12, vec3(0, 0, 180));
	temp[75]  = std::make_pair(12, vec3(0, 180, 180));
	temp[45]  = std::make_pair(12, vec3(0, 270, 180));

	// CASE 13 - 2 of em
	temp[165] = std::make_pair(13, vec3(0, 0, 0));
	temp[90]  = std::make_pair(13, vec3(0, 90, 0));

	// CASE 14 - 8 of em
	temp[113] = std::make_pair(14, vec3(0, 0, 0));
	temp[226] = std::make_pair(14, vec3(0, 270, 0));
	temp[184] = std::make_pair(14, vec3(0, 90, 0));
	temp[212] = std::make_pair(14, vec3(0, 180, 0));
	temp[142] = std::make_pair(14, vec3(0, 0, 180));
	temp[71]  = std::make_pair(14, vec3(0, 90, 180));
	temp[43]  = std::make_pair(14, vec3(0, 180, 180));
	temp[29]  = std::make_pair(14, vec3(0, 270, 180));
	temp[54]  = std::make_pair(14, vec3(90, 0, 90));
	temp[147] = std::make_pair(14, vec3(270, 0, 90));
	temp[201] = std::make_pair(14, vec3(90, 0, 270));
	temp[108] = std::make_pair(14, vec3(270, 0, 270));

	return temp;
} 