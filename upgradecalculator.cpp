/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* Author: Paul Grathwohl */

#include "upgradecalculator.h"
#include "log.h"
#include <sstream>

//--------------------------------------------------
cUpgradeCalculator& cUpgradeCalculator::instance()
{
  static cUpgradeCalculator _instance;
  if (!_instance.setupDone) // do the setup on the first call, not on startup of the program
	  _instance.setupLookupTables();
  return _instance;
}

//--------------------------------------------------
cUpgradeCalculator::cUpgradeCalculator()
: setupDone (false)
{}

//--------------------------------------------------
void cUpgradeCalculator::setupLookupTables()
{
	// ------------------------------ HITPOINTS and ARMOR and AMMO
	hitpointsArmorAmmo_2 [2] = 39;
	hitpointsArmorAmmo_2 [3] = 321;

	hitpointsArmorAmmo_4 [4] = 8;
	hitpointsArmorAmmo_4 [5] = 31;
	hitpointsArmorAmmo_4 [6] = 91;
	hitpointsArmorAmmo_4 [7] = 230;
	hitpointsArmorAmmo_4 [8] = 513;

	hitpointsArmorAmmo_6 [6]  = 4;
	hitpointsArmorAmmo_6 [7]  = 11;
	hitpointsArmorAmmo_6 [8]  = 24;
	hitpointsArmorAmmo_6 [9]  = 51;
	hitpointsArmorAmmo_6 [10] = 96;
	hitpointsArmorAmmo_6 [11] = 174;
	hitpointsArmorAmmo_6 [12] = 297;
	hitpointsArmorAmmo_6 [13] = 491;
	hitpointsArmorAmmo_6 [14] = 780;

	hitpointsArmorAmmo_8  [8]  = 2;
	hitpointsArmorAmmo_8  [9]  = 6;
	hitpointsArmorAmmo_8  [10] = 11;
	hitpointsArmorAmmo_8  [11] = 20;
	hitpointsArmorAmmo_8  [12] = 35;
	hitpointsArmorAmmo_8  [13] = 56;
	hitpointsArmorAmmo_8  [14] = 91;
	hitpointsArmorAmmo_8  [15] = 139;
	hitpointsArmorAmmo_8  [16] = 208;
	hitpointsArmorAmmo_8  [17] = 305;
	hitpointsArmorAmmo_8  [18] = 438;
	hitpointsArmorAmmo_8  [19] = 617;

	hitpointsArmorAmmo_9  [9]  = 2;
	hitpointsArmorAmmo_9  [10] = 5;
	hitpointsArmorAmmo_9  [11] = 8;
	hitpointsArmorAmmo_9  [12] = 14;
	hitpointsArmorAmmo_9  [13] = 23;
	hitpointsArmorAmmo_9  [14] = 38;
	hitpointsArmorAmmo_9  [15] = 57;
	hitpointsArmorAmmo_9  [16] = 86;
	hitpointsArmorAmmo_9  [17] = 127;
	hitpointsArmorAmmo_9  [18] = 181;
	hitpointsArmorAmmo_9  [19] = 254;
	hitpointsArmorAmmo_9  [20] = 353;
	hitpointsArmorAmmo_9  [21] = 480;
	hitpointsArmorAmmo_9  [22] = 645;

	hitpointsArmorAmmo_10 [10] = 5;
	hitpointsArmorAmmo_10 [12] = 17;
	hitpointsArmorAmmo_10 [14] = 43;
	hitpointsArmorAmmo_10 [16] = 97;
	hitpointsArmorAmmo_10 [18] = 198;
	hitpointsArmorAmmo_10 [20] = 377;
	hitpointsArmorAmmo_10 [22] = 682;

	hitpointsArmorAmmo_12 [12] = 4;
	hitpointsArmorAmmo_12 [14] = 11;
	hitpointsArmorAmmo_12 [16] = 24;
	hitpointsArmorAmmo_12 [18] = 51;
	hitpointsArmorAmmo_12 [20] = 96;
	hitpointsArmorAmmo_12 [22] = 174;
	hitpointsArmorAmmo_12 [24] = 297;
	hitpointsArmorAmmo_12 [26] = 491;
	hitpointsArmorAmmo_12 [28] = 780;

	hitpointsArmorAmmo_14 [14] = 3;
	hitpointsArmorAmmo_14 [16] = 8;
	hitpointsArmorAmmo_14 [18] = 16;
	hitpointsArmorAmmo_14 [20] = 30;
	hitpointsArmorAmmo_14 [22] = 54;
	hitpointsArmorAmmo_14 [24] = 94;
	hitpointsArmorAmmo_14 [26] = 155;
	hitpointsArmorAmmo_14 [28] = 245;
	hitpointsArmorAmmo_14 [30] = 378;
	hitpointsArmorAmmo_14 [32] = 567;

	hitpointsArmorAmmo_16 [16] = 2;
	hitpointsArmorAmmo_16 [18] = 6;
	hitpointsArmorAmmo_16 [20] = 11;
	hitpointsArmorAmmo_16 [22] = 20;
	hitpointsArmorAmmo_16 [24] = 35;
	hitpointsArmorAmmo_16 [26] = 56;
	hitpointsArmorAmmo_16 [28] = 91;
	hitpointsArmorAmmo_16 [30] = 139;
	hitpointsArmorAmmo_16 [32] = 208;
	hitpointsArmorAmmo_16 [34] = 305;
	hitpointsArmorAmmo_16 [36] = 438;
	hitpointsArmorAmmo_16 [38] = 617;

	hitpointsArmorAmmo_18 [18] = 2;
	hitpointsArmorAmmo_18 [20] = 5;
	hitpointsArmorAmmo_18 [22] = 8;
	hitpointsArmorAmmo_18 [24] = 14;
	hitpointsArmorAmmo_18 [26] = 23;
	hitpointsArmorAmmo_18 [28] = 38;
	hitpointsArmorAmmo_18 [30] = 57;
	hitpointsArmorAmmo_18 [32] = 86;
	hitpointsArmorAmmo_18 [34] = 127;
	hitpointsArmorAmmo_18 [36] = 181;
	hitpointsArmorAmmo_18 [38] = 254;
	hitpointsArmorAmmo_18 [40] = 353;
	hitpointsArmorAmmo_18 [42] = 480;
	hitpointsArmorAmmo_18 [44] = 645;

	hitpointsArmorAmmo_20 [20] = 2;
	hitpointsArmorAmmo_20 [22] = 3;
	hitpointsArmorAmmo_20 [24] = 7;
	hitpointsArmorAmmo_20 [26] = 10;
	hitpointsArmorAmmo_20 [28] = 17;
	hitpointsArmorAmmo_20 [30] = 26;
	hitpointsArmorAmmo_20 [32] = 40;
	hitpointsArmorAmmo_20 [34] = 57;
	hitpointsArmorAmmo_20 [36] = 82;
	hitpointsArmorAmmo_20 [38] = 116;
	hitpointsArmorAmmo_20 [40] = 160;
	hitpointsArmorAmmo_20 [42] = 217;
	hitpointsArmorAmmo_20 [44] = 293;
	hitpointsArmorAmmo_20 [46] = 389;
	hitpointsArmorAmmo_20 [48] = 509;
	hitpointsArmorAmmo_20 [50] = 660;

	hitpointsArmorAmmo_24 [24] = 1;
	hitpointsArmorAmmo_24 [26] = 3;
	hitpointsArmorAmmo_24 [28] = 4;
	hitpointsArmorAmmo_24 [30] = 7;
	hitpointsArmorAmmo_24 [32] = 10;
	hitpointsArmorAmmo_24 [34] = 14;
	hitpointsArmorAmmo_24 [36] = 21;
	hitpointsArmorAmmo_24 [38] = 30;
	hitpointsArmorAmmo_24 [40] = 40;
	hitpointsArmorAmmo_24 [42] = 56;
	hitpointsArmorAmmo_24 [44] = 75;
	hitpointsArmorAmmo_24 [46] = 99;
	hitpointsArmorAmmo_24 [48] = 129;
	hitpointsArmorAmmo_24 [50] = 168;
	hitpointsArmorAmmo_24 [52] = 216;
	hitpointsArmorAmmo_24 [54] = 275;
	hitpointsArmorAmmo_24 [56] = 346;
	hitpointsArmorAmmo_24 [58] = 434;
	hitpointsArmorAmmo_24 [60] = 538;
	hitpointsArmorAmmo_24 [62] = 663;

	hitpointsArmorAmmo_26 [26] = 5;
	hitpointsArmorAmmo_26 [31] = 15;
	hitpointsArmorAmmo_26 [36] = 38;
	hitpointsArmorAmmo_26 [41] = 84;
	hitpointsArmorAmmo_26 [46] = 168;
	hitpointsArmorAmmo_26 [51] = 319;
	hitpointsArmorAmmo_26 [56] = 567;

	hitpointsArmorAmmo_28 [28] = 4;
	hitpointsArmorAmmo_28 [33] = 13;
	hitpointsArmorAmmo_28 [38] = 30;
	hitpointsArmorAmmo_28 [43] = 64;
	hitpointsArmorAmmo_28 [48] = 126;
	hitpointsArmorAmmo_28 [53] = 232;
	hitpointsArmorAmmo_28 [58] = 404;
	hitpointsArmorAmmo_28 [63] = 677;

	hitpointsArmorAmmo_32 [32] = 3;
	hitpointsArmorAmmo_32 [37] = 10;
	hitpointsArmorAmmo_32 [42] = 20;
	hitpointsArmorAmmo_32 [47] = 41;
	hitpointsArmorAmmo_32 [52] = 75;
	hitpointsArmorAmmo_32 [57] = 134;
	hitpointsArmorAmmo_32 [62] = 225;
	hitpointsArmorAmmo_32 [67] = 365;
	hitpointsArmorAmmo_32 [72] = 574;

	hitpointsArmorAmmo_36 [36] = 3;
	hitpointsArmorAmmo_36 [41] = 7;
	hitpointsArmorAmmo_36 [46] = 15;
	hitpointsArmorAmmo_36 [51] = 27;
	hitpointsArmorAmmo_36 [56] = 50;
	hitpointsArmorAmmo_36 [61] = 84;
	hitpointsArmorAmmo_36 [66] = 137;
	hitpointsArmorAmmo_36 [71] = 218;
	hitpointsArmorAmmo_36 [76] = 332;
	hitpointsArmorAmmo_36 [81] = 497;
	hitpointsArmorAmmo_36 [86] = 724;

	hitpointsArmorAmmo_40 [40] = 2;
	hitpointsArmorAmmo_40 [45] = 6;
	hitpointsArmorAmmo_40 [50] = 11;
	hitpointsArmorAmmo_40 [55] = 20;
	hitpointsArmorAmmo_40 [60] = 35;
	hitpointsArmorAmmo_40 [65] = 56;
	hitpointsArmorAmmo_40 [70] = 91;
	hitpointsArmorAmmo_40 [75] = 139;
	hitpointsArmorAmmo_40 [80] = 208;
	hitpointsArmorAmmo_40 [85] = 305;
	hitpointsArmorAmmo_40 [90] = 438;
	hitpointsArmorAmmo_40 [95] = 617;

	hitpointsArmorAmmo_56 [56]  = 4;
	hitpointsArmorAmmo_56 [66]  = 13;
	hitpointsArmorAmmo_56 [76]  = 30;
	hitpointsArmorAmmo_56 [86]  = 64;
	hitpointsArmorAmmo_56 [96]  = 126;
	hitpointsArmorAmmo_56 [106] = 232;
	hitpointsArmorAmmo_56 [116] = 404;
	hitpointsArmorAmmo_56 [126] = 677;


	// ------------------------------ ATTACK and SPEED
	attackSpeed_5  [5]  = 11;
	attackSpeed_5  [6]  = 34;
	attackSpeed_5  [7]  = 86;
	attackSpeed_5  [8]  = 193;
	attackSpeed_5  [9]  = 396;
	attackSpeed_5  [10] = 755;

	attackSpeed_6  [6]  = 8;
	attackSpeed_6  [7]  = 22;
	attackSpeed_6  [8]  = 49;
	attackSpeed_6  [9]  = 101;
	attackSpeed_6  [10] = 193;
	attackSpeed_6  [11] = 347;
	attackSpeed_6  [12] = 595;

	attackSpeed_7  [7]  = 6;
	attackSpeed_7  [8]  = 16;
	attackSpeed_7  [9]  = 32;
	attackSpeed_7  [10] = 60;
	attackSpeed_7  [11] = 109;
	attackSpeed_7  [12] = 188;
	attackSpeed_7  [13] = 309;
	attackSpeed_7  [14] = 490;
	attackSpeed_7  [15] = 757;

	attackSpeed_8  [8]  = 5;
	attackSpeed_8  [9]  = 12;
	attackSpeed_8  [10] = 22;
	attackSpeed_8  [11] = 40;
	attackSpeed_8  [12] = 69;
	attackSpeed_8  [13] = 113;
	attackSpeed_8  [14] = 181;
	attackSpeed_8  [15] = 278;
	attackSpeed_8  [16] = 416;
	attackSpeed_8  [17] = 611;

	attackSpeed_9  [9]  = 4;
	attackSpeed_9  [10] = 10;
	attackSpeed_9  [11] = 16;
	attackSpeed_9  [12] = 29;
	attackSpeed_9  [13] = 46;
	attackSpeed_9  [14] = 75;
	attackSpeed_9  [15] = 115;
	attackSpeed_9  [16] = 172;
	attackSpeed_9  [17] = 253;
	attackSpeed_9  [18] = 362;
	attackSpeed_9  [19] = 509;
	attackSpeed_9  [20] = 705;

	attackSpeed_10 [10] = 11;
	attackSpeed_10 [12] = 34;
	attackSpeed_10 [14] = 86;
	attackSpeed_10 [16] = 193;
	attackSpeed_10 [18] = 396;
	attackSpeed_10 [20] = 755;

	attackSpeed_11 [11] = 10;
	attackSpeed_11 [13] = 26;
	attackSpeed_11 [15] = 64;
	attackSpeed_11 [17] = 137;
	attackSpeed_11 [19] = 269;
	attackSpeed_11 [21] = 500;
	
	attackSpeed_12 [12] = 8;
	attackSpeed_12 [14] = 22;
	attackSpeed_12 [16] = 49;
	attackSpeed_12 [18] = 101;
	attackSpeed_12 [20] = 193;
	attackSpeed_12 [22] = 347;
	attackSpeed_12 [24] = 595;

	attackSpeed_14 [14] = 6;
	attackSpeed_14 [16] = 16;
	attackSpeed_14 [18] = 32;
	attackSpeed_14 [20] = 60;
	attackSpeed_14 [22] = 109;
	attackSpeed_14 [24] = 188;
	attackSpeed_14 [26] = 309;
	attackSpeed_14 [28] = 490;
	attackSpeed_14 [30] = 757;

	attackSpeed_15 [15] = 6;
	attackSpeed_15 [17] = 13;
	attackSpeed_15 [19] = 26;
	attackSpeed_15 [21] = 49;
	attackSpeed_15 [23] = 86;
	attackSpeed_15 [25] = 144;
	attackSpeed_15 [27] = 233;
	attackSpeed_15 [29] = 364;
	attackSpeed_15 [31] = 554;

	attackSpeed_16 [16] = 5;
	attackSpeed_16 [18] = 12;
	attackSpeed_16 [20] = 22;
	attackSpeed_16 [22] = 40;
	attackSpeed_16 [24] = 69;
	attackSpeed_16 [26] = 113;
	attackSpeed_16 [28] = 181;
	attackSpeed_16 [30] = 278;
	attackSpeed_16 [32] = 416;
	attackSpeed_16 [34] = 611;

	attackSpeed_18 [18] = 4;
	attackSpeed_18 [20] = 10;
	attackSpeed_18 [22] = 16;
	attackSpeed_18 [24] = 29;
	attackSpeed_18 [26] = 46;
	attackSpeed_18 [28] = 75;
	attackSpeed_18 [30] = 115;
	attackSpeed_18 [32] = 172;
	attackSpeed_18 [34] = 253;
	attackSpeed_18 [36] = 362;
	attackSpeed_18 [38] = 509;
	attackSpeed_18 [40] = 705;

	attackSpeed_20 [20] = 4;
	attackSpeed_20 [22] = 7;
	attackSpeed_20 [24] = 13;
	attackSpeed_20 [26] = 21;
	attackSpeed_20 [28] = 34;
	attackSpeed_20 [30] = 52;
	attackSpeed_20 [32] = 79;
	attackSpeed_20 [34] = 114;
	attackSpeed_20 [36] = 164;
	attackSpeed_20 [38] = 232;
	attackSpeed_20 [40] = 320;
	attackSpeed_20 [42] = 435;
	attackSpeed_20 [44] = 586;
	attackSpeed_20 [46] = 777;

	attackSpeed_22 [22] = 3;
	attackSpeed_22 [24] = 7;
	attackSpeed_22 [26] = 10;
	attackSpeed_22 [28] = 16;
	attackSpeed_22 [30] = 26;
	attackSpeed_22 [32] = 38;
	attackSpeed_22 [34] = 56;
	attackSpeed_22 [36] = 81;
	attackSpeed_22 [38] = 113;
	attackSpeed_22 [40] = 156;
	attackSpeed_22 [42] = 214;
	attackSpeed_22 [44] = 286;
	attackSpeed_22 [46] = 380;
	attackSpeed_22 [48] = 498;
	attackSpeed_22 [50] = 646;

	attackSpeed_24 [24] = 3;
	attackSpeed_24 [26] = 5;
	attackSpeed_24 [28] = 9;
	attackSpeed_24 [30] = 13;
	attackSpeed_24 [32] = 20;
	attackSpeed_24 [34] = 29;
	attackSpeed_24 [36] = 42;
	attackSpeed_24 [38] = 59;
	attackSpeed_24 [40] = 81;
	attackSpeed_24 [42] = 112;
	attackSpeed_24 [44] = 149;
	attackSpeed_24 [46] = 198;
	attackSpeed_24 [48] = 259;
	attackSpeed_24 [50] = 336;
	attackSpeed_24 [52] = 432;
	attackSpeed_24 [54] = 549;
	attackSpeed_24 [56] = 693;

	attackSpeed_28 [28] = 9;
	attackSpeed_28 [33] = 26;
	attackSpeed_28 [38] = 60;
	attackSpeed_28 [43] = 128;
	attackSpeed_28 [48] = 252;
	attackSpeed_28 [53] = 463;

	attackSpeed_30 [30] = 8;
	attackSpeed_30 [35] = 22;
	attackSpeed_30 [40] = 49;
	attackSpeed_30 [45] = 101;
	attackSpeed_30 [50] = 193;
	attackSpeed_30 [55] = 347;
	attackSpeed_30 [60] = 595;

	attackSpeed_36 [36] = 6;
	attackSpeed_36 [41] = 15;
	attackSpeed_36 [46] = 29;
	attackSpeed_36 [51] = 55;
	attackSpeed_36 [56] = 99;
	attackSpeed_36 [61] = 169;
	attackSpeed_36 [66] = 274;
	attackSpeed_36 [71] = 435;
	attackSpeed_36 [76] = 665;


	// ------------------------------ RANGE and SCAN
	rangeScan_3  [3] = 61;
	rangeScan_3  [4] = 299;

	rangeScan_4  [4] = 34;
	rangeScan_4  [5] = 125;
	rangeScan_4  [6] = 364;

	rangeScan_5  [5] = 23;
	rangeScan_5  [6] = 68;
	rangeScan_5  [7] = 172;
	rangeScan_5  [8] = 386;
	rangeScan_5  [9] = 791;

	rangeScan_6  [6]  = 17;
	rangeScan_6  [7]  = 44;
	rangeScan_6  [8]  = 98;
	rangeScan_6  [9]  = 201;
	rangeScan_6  [10] = 386;
	rangeScan_6  [11] = 694;

	rangeScan_7  [7]  = 13;
	rangeScan_7  [8]  = 31;
	rangeScan_7  [9]  = 64;
	rangeScan_7  [10] = 121;
	rangeScan_7  [11] = 218;
	rangeScan_7  [12] = 375;
	rangeScan_7  [13] = 618;

	rangeScan_8  [8]  = 11;
	rangeScan_8  [9]  = 23;
	rangeScan_8  [10] = 45;
	rangeScan_8  [11] = 80;
	rangeScan_8  [12] = 138;
	rangeScan_8  [13] = 226;
	rangeScan_8  [14] = 361;
	rangeScan_8  [15] = 556;

	rangeScan_9  [9]  = 9;
	rangeScan_9  [10] = 19;
	rangeScan_9  [11] = 33;
	rangeScan_9  [12] = 57;
	rangeScan_9  [13] = 93;
	rangeScan_9  [14] = 149;
	rangeScan_9  [15] = 230;
	rangeScan_9  [16] = 345;
	rangeScan_9  [17] = 505;
	rangeScan_9  [18] = 724;

	rangeScan_10 [10] = 23;
	rangeScan_10 [12] = 68;
	rangeScan_10 [14] = 172;
	rangeScan_10 [16] = 386;
	rangeScan_10 [18] = 791;

	rangeScan_11 [11] = 20;
	rangeScan_11 [13] = 53;
	rangeScan_11 [15] = 128;
	rangeScan_11 [17] = 273;
	rangeScan_11 [19] = 539;

	rangeScan_12 [12] = 17;
	rangeScan_12 [14] = 44;
	rangeScan_12 [16] = 98;
	rangeScan_12 [18] = 201;
	rangeScan_12 [20] = 386;
	rangeScan_12 [22] = 694;

	rangeScan_14 [14] = 13;
	rangeScan_14 [16] = 31;
	rangeScan_14 [18] = 64;
	rangeScan_14 [20] = 121;
	rangeScan_14 [22] = 218;
	rangeScan_14 [24] = 375;
	rangeScan_14 [26] = 618;

	rangeScan_16 [16] = 11;
	rangeScan_16 [18] = 23;
	rangeScan_16 [20] = 45;
	rangeScan_16 [22] = 80;
	rangeScan_16 [24] = 138;
	rangeScan_16 [26] = 226;
	rangeScan_16 [28] = 361;
	rangeScan_16 [30] = 556;

	rangeScan_18 [18] = 9;
	rangeScan_18 [20] = 19;
	rangeScan_18 [22] = 33;
	rangeScan_18 [24] = 57;
	rangeScan_18 [26] = 93;
	rangeScan_18 [28] = 149;
	rangeScan_18 [30] = 230;
	rangeScan_18 [32] = 345;
	rangeScan_18 [34] = 505;
	rangeScan_18 [36] = 724;

	rangeScan_20 [20] = 8;
	rangeScan_20 [22] = 15;
	rangeScan_20 [24] = 26;
	rangeScan_20 [26] = 42;
	rangeScan_20 [28] = 68;
	rangeScan_20 [30] = 104;
	rangeScan_20 [32] = 157;
	rangeScan_20 [34] = 229;
	rangeScan_20 [36] = 328;
	rangeScan_20 [38] = 463;
	rangeScan_20 [40] = 640;

	rangeScan_24 [24] = 6;
	rangeScan_24 [26] = 11;
	rangeScan_24 [28] = 17;
	rangeScan_24 [30] = 27;
	rangeScan_24 [32] = 40;
	rangeScan_24 [34] = 58;
	rangeScan_24 [36] = 84;
	rangeScan_24 [38] = 117;
	rangeScan_24 [40] = 163;
	rangeScan_24 [42] = 223;
	rangeScan_24 [44] = 298;
	rangeScan_24 [46] = 396;
	rangeScan_24 [48] = 518;
	rangeScan_24 [50] = 673;

	// ------------------------------ SHOTS
	shots_1 [1] = 720;

	shots_2 [2] = 79;
	shots_2 [3] = 641;

	// finished....
	setupDone = true;
}

//--------------------------------------------------
int cUpgradeCalculator::lookupPrice(const PriceMap& prices, int value) const
{
	PriceMap::const_iterator it = prices.find(value);
	if (it != prices.end())
		return it->second; // the price
	return kNoPriceAvailable;
}

//--------------------------------------------------
int cUpgradeCalculator::calcPrice(int curValue, int orgValue, int upgradeType, cResearch& researchLevel) const
{
	int bonusByResearch = calcChangeByResearch(orgValue, researchLevel.getCurResearchLevel (researchLevel.getResearchArea (upgradeType)));
	curValue -= bonusByResearch;
	
	int price = kNoPriceAvailable;
	switch (upgradeType) {

		case kHitpoints:
		case kArmor:
		case kAmmo:
		{
			switch (orgValue) {
				case 2:  price = lookupPrice (hitpointsArmorAmmo_2,  curValue); break;
				case 4:  price = lookupPrice (hitpointsArmorAmmo_4,  curValue); break;
				case 6:  price = lookupPrice (hitpointsArmorAmmo_6,  curValue); break;
				case 8:  price = lookupPrice (hitpointsArmorAmmo_8,  curValue); break;
				case 9:  price = lookupPrice (hitpointsArmorAmmo_9,  curValue); break;
				case 10: price = lookupPrice (hitpointsArmorAmmo_10, curValue); break;
				case 12: price = lookupPrice (hitpointsArmorAmmo_12, curValue); break;
				case 14: price = lookupPrice (hitpointsArmorAmmo_14, curValue); break;
				case 16: price = lookupPrice (hitpointsArmorAmmo_16, curValue); break;
				case 18: price = lookupPrice (hitpointsArmorAmmo_18, curValue); break;
				case 20: price = lookupPrice (hitpointsArmorAmmo_20, curValue); break;
				case 24: price = lookupPrice (hitpointsArmorAmmo_24, curValue); break;
				case 26: price = lookupPrice (hitpointsArmorAmmo_26, curValue); break;
				case 28: price = lookupPrice (hitpointsArmorAmmo_28, curValue); break;
				case 32: price = lookupPrice (hitpointsArmorAmmo_32, curValue); break;
				case 36: price = lookupPrice (hitpointsArmorAmmo_36, curValue); break;
				case 40: price = lookupPrice (hitpointsArmorAmmo_40, curValue); break;
				case 56: price = lookupPrice (hitpointsArmorAmmo_56, curValue); break;
				default: break;
			}
			break;
		}

		case kAttack:
		case kSpeed:
		{
			switch (orgValue) {
				case 5:  price = lookupPrice (attackSpeed_5,  curValue); break;
				case 6:  price = lookupPrice (attackSpeed_6,  curValue); break;
				case 7:  price = lookupPrice (attackSpeed_7,  curValue); break;
				case 8:  price = lookupPrice (attackSpeed_8,  curValue); break;
				case 9:  price = lookupPrice (attackSpeed_9,  curValue); break;
				case 10: price = lookupPrice (attackSpeed_10, curValue); break;
				case 11: price = lookupPrice (attackSpeed_11, curValue); break;
				case 12: price = lookupPrice (attackSpeed_12, curValue); break;
				case 14: price = lookupPrice (attackSpeed_14, curValue); break;
				case 15: price = lookupPrice (attackSpeed_15, curValue); break;
				case 16: price = lookupPrice (attackSpeed_16, curValue); break;
				case 18: price = lookupPrice (attackSpeed_18, curValue); break;
				case 20: price = lookupPrice (attackSpeed_20, curValue); break;
				case 22: price = lookupPrice (attackSpeed_22, curValue); break;
				case 24: price = lookupPrice (attackSpeed_24, curValue); break;
				case 28: price = lookupPrice (attackSpeed_28, curValue); break;
				case 30: price = lookupPrice (attackSpeed_30, curValue); break;
				case 36: price = lookupPrice (attackSpeed_36, curValue); break;
				default: break;
			}
			break;
		}

		case kRange:
		case kScan:
		{
			switch (orgValue) {
				case 3:  price = lookupPrice (rangeScan_3,  curValue); break;
				case 4:  price = lookupPrice (rangeScan_4,  curValue); break;
				case 5:  price = lookupPrice (rangeScan_5,  curValue); break;
				case 6:  price = lookupPrice (rangeScan_6,  curValue); break;
				case 7:  price = lookupPrice (rangeScan_7,  curValue); break;
				case 8:  price = lookupPrice (rangeScan_8,  curValue); break;
				case 9:  price = lookupPrice (rangeScan_9,  curValue); break;
				case 10: price = lookupPrice (rangeScan_10, curValue); break;
				case 11: price = lookupPrice (rangeScan_11, curValue); break;
				case 12: price = lookupPrice (rangeScan_12, curValue); break;
				case 14: price = lookupPrice (rangeScan_14, curValue); break;
				case 16: price = lookupPrice (rangeScan_16, curValue); break;
				case 18: price = lookupPrice (rangeScan_18, curValue); break;
				case 20: price = lookupPrice (rangeScan_20, curValue); break;
				case 24: price = lookupPrice (rangeScan_24, curValue); break;
				default: break;
			}
			break;
		}

		case kShots:
		{
			switch (orgValue) {
				case 1:  price = lookupPrice (shots_1,  curValue); break;
				case 2:  price = lookupPrice (shots_2,  curValue); break;
				default: break;
			}
			break;
		}

		default: break;
	}
	return price;
}

//--------------------------------------------------
int cUpgradeCalculator::getCostForUpgrade(int orgValue, int curValue, int newValue, int upgradeType, cResearch& researchLevel) const
{
	int cost = 0;
	if (orgValue <= curValue && curValue < newValue)
	{
		int upgradedValue = curValue;
		while (upgradedValue < newValue)
		{
			int costsForThis = calcPrice (upgradedValue, orgValue, upgradeType, researchLevel);
			if (costsForThis != kNoPriceAvailable)
			{
				cost += costsForThis;
				upgradedValue += calcIncreaseByUpgrade (orgValue);
				if (upgradedValue > newValue)
					return kNoPriceAvailable; // it is not possible to reach the newValue with upgrading
			}
			else
				return kNoPriceAvailable;
		}
	}
	return cost;
}

//--------------------------------------------------
int cUpgradeCalculator::calcResearchTurns(int curResearchLevel, int upgradeType) const
{
	int neededTurns = kNoResearchAvailable;

	switch (upgradeType)
	{
		case kHitpoints:
		case kArmor:
		{
			if (curResearchLevel < 10) neededTurns = 8;
			else if (curResearchLevel < 20)  neededTurns = 15;
			else if (curResearchLevel < 30)  neededTurns = 25;
			else if (curResearchLevel < 40)  neededTurns = 42;
			else if (curResearchLevel < 50)  neededTurns = 67;
			else if (curResearchLevel < 60)  neededTurns = 104;
			else if (curResearchLevel < 70)  neededTurns = 156;
			else if (curResearchLevel < 80)  neededTurns = 229;
			else if (curResearchLevel < 90)  neededTurns = 328;
			else if (curResearchLevel < 100) neededTurns = 462;
			else if (curResearchLevel < 110) neededTurns = 639;
			else if (curResearchLevel < 120) neededTurns = 871;
			else if (curResearchLevel < 130) neededTurns = 1171;
			else if (curResearchLevel < 140) neededTurns = 1553;
			else if (curResearchLevel < 150) neededTurns = 2036;
			else if (curResearchLevel < 160) neededTurns = 2640;
			else if (curResearchLevel < 170) neededTurns = 3389;
			else if (curResearchLevel < 180) neededTurns = 4311;
			else if (curResearchLevel < 190) neededTurns = 5437;
			else if (curResearchLevel < 200) neededTurns = 6803;
			else if (curResearchLevel < 210) neededTurns = 8448;
			break;
		}

		case kAttack:
		case kSpeed:
		case kShots:
		{
			if (curResearchLevel < 10) neededTurns = 16;
			else if (curResearchLevel < 20)  neededTurns = 30;
			else if (curResearchLevel < 30)  neededTurns = 51;
			else if (curResearchLevel < 40)  neededTurns = 85;
			else if (curResearchLevel < 50)  neededTurns = 135;
			else if (curResearchLevel < 60)  neededTurns = 208;
			else if (curResearchLevel < 70)  neededTurns = 312;
			else if (curResearchLevel < 80)  neededTurns = 458;
			else if (curResearchLevel < 90)  neededTurns = 657;
			else if (curResearchLevel < 100) neededTurns = 924;
			else if (curResearchLevel < 110) neededTurns = 1278;
			else if (curResearchLevel < 120) neededTurns = 1742;
			else if (curResearchLevel < 130) neededTurns = 2342;
			else if (curResearchLevel < 140) neededTurns = 3106;
			else if (curResearchLevel < 150) neededTurns = 4072;
			else if (curResearchLevel < 160) neededTurns = 5280;
			else if (curResearchLevel < 170) neededTurns = 6778;
			else if (curResearchLevel < 180) neededTurns = 8622;
			else if (curResearchLevel < 190) neededTurns = 10874;
			else if (curResearchLevel < 200) neededTurns = 13606;
			else if (curResearchLevel < 210) neededTurns = 16896;
			break;
		}

		case kRange:
		case kScan:
		case kCost:
		{
			if (curResearchLevel < 10) neededTurns = 33;
			else if (curResearchLevel < 20)  neededTurns = 60;
			else if (curResearchLevel < 30)  neededTurns = 103;
			else if (curResearchLevel < 40)  neededTurns = 170;
			else if (curResearchLevel < 50)  neededTurns = 270;
			else if (curResearchLevel < 60)  neededTurns = 416;
			else if (curResearchLevel < 70)  neededTurns = 625;
			else if (curResearchLevel < 80)  neededTurns = 916;
			else if (curResearchLevel < 90)  neededTurns = 1314;
			else if (curResearchLevel < 100) neededTurns = 1849;
			else if (curResearchLevel < 110) neededTurns = 2559;
			else if (curResearchLevel < 120) neededTurns = 3487;
			else if (curResearchLevel < 130) neededTurns = 4684;
			else if (curResearchLevel < 140) neededTurns = 6213;
			else if (curResearchLevel < 150) neededTurns = 8144;
			else if (curResearchLevel < 160) neededTurns = 10560;
			else if (curResearchLevel < 170) neededTurns = 13556;
			else if (curResearchLevel < 180) neededTurns = 17244;
			else if (curResearchLevel < 190) neededTurns = 21748;
			else if (curResearchLevel < 200) neededTurns = 27212;
			else if (curResearchLevel < 210) neededTurns = 33792;
			break;
		}

		// case kAmmo: <- There's no research possibility for Ammo in M.A.X.!
		default: break;
	}

	return neededTurns;
}

//--------------------------------------------------
int cUpgradeCalculator::calcIncreaseByUpgrade(int startValue) const
{
	if (startValue < 10)
		return 1;
	if (startValue < 26)
		return 2;
	if (startValue < 56)
		return 5;
	return 10;
}

//--------------------------------------------------
int cUpgradeCalculator::calcChangeByResearch(int startValue, int curResearchLevel, 
											 int upgradeType, int unitType) const
{
	if (curResearchLevel <= 0) // no research done yet...
		return 0;

	if (upgradeType == -1 || upgradeType != kCost) // standard research areas - all handled the same way
	{
		// a simple integer division does the job
		int newValue = (startValue * (100 + curResearchLevel)) / 100;
		return newValue - startValue;
	}
	else if (upgradeType == kCost)
	{
		// cost makes a decrease based on the formula 1/x (where x is the research level)
		double realCost = startValue / ((100.0f + curResearchLevel) / 100.0f);

		// now the real cost is rounded to the next possible cost value 
		// (Unit factories: steps of 3, Building construction: steps of 2, Infantry training: steps of 1)
		int costRounded = startValue;
		if (unitType == kBuilding)
			costRounded = getNearestPossibleCost(realCost, 2);
		else if (unitType == kInfantry)
			costRounded = getNearestPossibleCost(realCost, 1);
		else if (unitType == kStandardUnit)
			costRounded = getNearestPossibleCost(realCost, 3);

		return costRounded - startValue;
	}
	else
		return 0;
}

//--------------------------------------------------
int cUpgradeCalculator::getMaterialCostForUpgrading(int unitCost) const
{
	if (unitCost < 4)
		return 0;
	return unitCost / 4;
}

//--------------------------------------------------
int cUpgradeCalculator::getNearestPossibleCost(double realCost, int costDifference) const
{
	if (costDifference <= 0)
		return (int) realCost;

	int intCost = (int) realCost;
	int nearestLowerCost = intCost - (intCost % costDifference); 
	if (realCost - nearestLowerCost < (costDifference / 2.0f))
		return nearestLowerCost;
	else
		return nearestLowerCost + costDifference;
}

//--------------------------------------------------
void cUpgradeCalculator::printAllToLog() const
{
	printToLog("CALC CHANGE BY RESEARCH TEST ---- CALC CHANGE BY RESEARCH TEST");

	printToLog("--------------- Cost-Research for Buildings ----------------");
	printToLog("Building-Cost: Start 40, Level   0 => Change: ", calcChangeByResearch(40,   0, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  10 => Change: ", calcChangeByResearch(40,  10, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  20 => Change: ", calcChangeByResearch(40,  20, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  30 => Change: ", calcChangeByResearch(40,  30, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  40 => Change: ", calcChangeByResearch(40,  40, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  50 => Change: ", calcChangeByResearch(40,  50, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  60 => Change: ", calcChangeByResearch(40,  60, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  70 => Change: ", calcChangeByResearch(40,  70, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  80 => Change: ", calcChangeByResearch(40,  80, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level  90 => Change: ", calcChangeByResearch(40,  90, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 100 => Change: ", calcChangeByResearch(40, 100, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 110 => Change: ", calcChangeByResearch(40, 110, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 120 => Change: ", calcChangeByResearch(40, 120, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 130 => Change: ", calcChangeByResearch(40, 130, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 140 => Change: ", calcChangeByResearch(40, 140, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 150 => Change: ", calcChangeByResearch(40, 150, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 160 => Change: ", calcChangeByResearch(40, 160, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 170 => Change: ", calcChangeByResearch(40, 170, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 180 => Change: ", calcChangeByResearch(40, 180, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 190 => Change: ", calcChangeByResearch(40, 190, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 200 => Change: ", calcChangeByResearch(40, 200, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 210 => Change: ", calcChangeByResearch(40, 210, kCost, kBuilding));
	printToLog("Building-Cost: Start 40, Level 220 => Change: ", calcChangeByResearch(40, 220, kCost, kBuilding));

	printToLog("--------------- Cost-Research for Standard Units ----------------");
	printToLog("Unit-Cost: Start 24, Level   0 => Change: ", calcChangeByResearch(24,   0, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  10 => Change: ", calcChangeByResearch(24,  10, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  20 => Change: ", calcChangeByResearch(24,  20, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  30 => Change: ", calcChangeByResearch(24,  30, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  40 => Change: ", calcChangeByResearch(24,  40, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  50 => Change: ", calcChangeByResearch(24,  50, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  60 => Change: ", calcChangeByResearch(24,  60, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  70 => Change: ", calcChangeByResearch(24,  70, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  80 => Change: ", calcChangeByResearch(24,  80, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level  90 => Change: ", calcChangeByResearch(24,  90, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 100 => Change: ", calcChangeByResearch(24, 100, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 110 => Change: ", calcChangeByResearch(24, 110, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 120 => Change: ", calcChangeByResearch(24, 120, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 130 => Change: ", calcChangeByResearch(24, 130, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 140 => Change: ", calcChangeByResearch(24, 140, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 150 => Change: ", calcChangeByResearch(24, 150, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 160 => Change: ", calcChangeByResearch(24, 160, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 170 => Change: ", calcChangeByResearch(24, 170, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 180 => Change: ", calcChangeByResearch(24, 180, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 190 => Change: ", calcChangeByResearch(24, 190, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 200 => Change: ", calcChangeByResearch(24, 200, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 210 => Change: ", calcChangeByResearch(24, 210, kCost, kStandardUnit));
	printToLog("Unit-Cost: Start 24, Level 220 => Change: ", calcChangeByResearch(24, 220, kCost, kStandardUnit));

	printToLog("--------------- Cost-Research for Infantry ----------------");
	printToLog("Infantry-Cost: Start 9, Level   0 => Change: ", calcChangeByResearch(9,   0, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  10 => Change: ", calcChangeByResearch(9,  10, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  20 => Change: ", calcChangeByResearch(9,  20, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  30 => Change: ", calcChangeByResearch(9,  30, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  40 => Change: ", calcChangeByResearch(9,  40, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  50 => Change: ", calcChangeByResearch(9,  50, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  60 => Change: ", calcChangeByResearch(9,  60, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  70 => Change: ", calcChangeByResearch(9,  70, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  80 => Change: ", calcChangeByResearch(9,  80, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level  90 => Change: ", calcChangeByResearch(9,  90, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 100 => Change: ", calcChangeByResearch(9, 100, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 110 => Change: ", calcChangeByResearch(9, 110, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 120 => Change: ", calcChangeByResearch(9, 120, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 130 => Change: ", calcChangeByResearch(9, 130, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 140 => Change: ", calcChangeByResearch(9, 140, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 150 => Change: ", calcChangeByResearch(9, 150, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 160 => Change: ", calcChangeByResearch(9, 160, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 170 => Change: ", calcChangeByResearch(9, 170, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 180 => Change: ", calcChangeByResearch(9, 180, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 190 => Change: ", calcChangeByResearch(9, 190, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 200 => Change: ", calcChangeByResearch(9, 200, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 210 => Change: ", calcChangeByResearch(9, 210, kCost, kInfantry));
	printToLog("Infantry-Cost: Start 9, Level 220 => Change: ", calcChangeByResearch(9, 220, kCost, kInfantry));

	printToLog("--------------- Upgrade-Research (e.g. Armor) ----------------");
	printToLog("Normal-Research: Start 12, Level   0 => Change: ", calcChangeByResearch(12,   0));
	printToLog("Normal-Research: Start 12, Level  10 => Change: ", calcChangeByResearch(12,  10));
	printToLog("Normal-Research: Start 12, Level  20 => Change: ", calcChangeByResearch(12,  20));
	printToLog("Normal-Research: Start 12, Level  30 => Change: ", calcChangeByResearch(12,  30));
	printToLog("Normal-Research: Start 12, Level  40 => Change: ", calcChangeByResearch(12,  40));
	printToLog("Normal-Research: Start 12, Level  50 => Change: ", calcChangeByResearch(12,  50));
	printToLog("Normal-Research: Start 12, Level  60 => Change: ", calcChangeByResearch(12,  60));
	printToLog("Normal-Research: Start 12, Level  70 => Change: ", calcChangeByResearch(12,  70));
	printToLog("Normal-Research: Start 12, Level  80 => Change: ", calcChangeByResearch(12,  80));
	printToLog("Normal-Research: Start 12, Level  90 => Change: ", calcChangeByResearch(12,  90));
	printToLog("Normal-Research: Start 12, Level 100 => Change: ", calcChangeByResearch(12, 100));
	printToLog("Normal-Research: Start 12, Level 110 => Change: ", calcChangeByResearch(12, 110));
	printToLog("Normal-Research: Start 12, Level 120 => Change: ", calcChangeByResearch(12, 120));
	printToLog("Normal-Research: Start 12, Level 130 => Change: ", calcChangeByResearch(12, 130));
	printToLog("Normal-Research: Start 12, Level 140 => Change: ", calcChangeByResearch(12, 140));
	printToLog("Normal-Research: Start 12, Level 150 => Change: ", calcChangeByResearch(12, 150));
	printToLog("Normal-Research: Start 12, Level 160 => Change: ", calcChangeByResearch(12, 160));
	printToLog("Normal-Research: Start 12, Level 170 => Change: ", calcChangeByResearch(12, 170));
	printToLog("Normal-Research: Start 12, Level 180 => Change: ", calcChangeByResearch(12, 180));
	printToLog("Normal-Research: Start 12, Level 190 => Change: ", calcChangeByResearch(12, 190));
	printToLog("Normal-Research: Start 12, Level 200 => Change: ", calcChangeByResearch(12, 200));
	printToLog("Normal-Research: Start 12, Level 210 => Change: ", calcChangeByResearch(12, 210));
	printToLog("Normal-Research: Start 12, Level 220 => Change: ", calcChangeByResearch(12, 220));
}

//--------------------------------------------------
void cUpgradeCalculator::printToLog(const char* str, int value) const
{
	if (value != -1000)
	{
		std::stringstream ss;
		std::string printStr;
		ss << value;
		ss >> printStr;
		printStr.insert(0, str);
		Log.write(printStr, cLog::eLOG_TYPE_INFO);
	}
	else
		Log.write(str, cLog::eLOG_TYPE_INFO);
}




/*
// This is the old code (from JCK?) for calculating the upgrade cost. 
// Interesting is the formula used for calculating (although it doesn't produce
// the exact same results - but nearly).
// Maybe the formula could be used for interpolating costs for "non M.A.X." upgrades,
// which are introduced by mods of M.A.X. Reloaded.

	int tmp;
	double a, b, c;
	switch (upgradeKind)
	{
			// Treffer, Panzerung, Munition & Angriff
		case 0:
			switch ( org )
			{
				case 2:
					if ( value==2 ) return 39;
					else return 321;
					break;
				case 4:
					a=0.0016091639;
					b=-0.073815318;
					c=6.0672869;
					break;
				case 6:
					a=0.000034548596;
					b=-0.27217472;
					c=6.3695123;
					break;
				case 8:
					a=0.00037219059;
					b=2.5148748;
					c=5.0938608;
					break;
				case 9:
					a=0.000059941694;
					b=1.3962889;
					c=4.6045196;
					break;
				case 10:
					a=0.000033736018;
					b=1.4674423;
					c=5.5606209;
					break;
				case 12:
					a=0.0000011574058;
					b=0.23439586;
					c=6.113616;
					break;
				case 14:
					a=0.0000012483447;
					b=1.4562373;
					c=5.8250952;
					break;
				case 15:
					a=0.00000018548742;
					b=-0.33519669;
					c=6.3333527;
					break;
				case 16:
					a=0.000010898263;
					b=5.0297434;
					c=5.0938627;
					break;
				case 18:
					a=0.00000017182818;
					b=2.0009536;
					c=5.8937153;
					break;
				case 20:
					a=0.00000004065782;
					b=1.6533066;
					c=6.0601538;
					break;
				case 22:
					a=0.0000000076942857;
					b=-0.45461813;
					c=6.4148588;
					break;
				case 24:
					a=0.00000076484313;
					b=8.0505377;
					c=5.1465019;
					break;
				case 28:
					a=0.00000015199858;
					b=5.1528048;
					c=5.4700225;
					break;
				case 32:
					a=0.00000030797077;
					b=8.8830596;
					c=5.1409486;
					break;
				case 56:
					a=0.000000004477053;
					b=11.454622;
					c=5.4335099;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Geschwindgigkeit
		case 1:
			org=org/4;
			value=value/4;
			switch ( org )
			{
				case 5:
					a=0.00040716128;
					b=-0.16662054;
					c=6.2234362;
					break;
				case 6:
					a=0.00038548127;
					b=0.48236948;
					c=5.827724;
					break;
				case 7:
					a=0.000019798772;
					b=-0.31204765;
					c=6.3982628;
					break;
				case 9:
					a=0.0000030681294;
					b=-0.25372812;
					c=6.3995668;
					break;
				case 10:
					a=0.0000062019158;
					b=-0.23774407;
					c=6.1901333;
					break;
				case 12:
					a=0.0000064901101;
					b=0.93320705;
					c=5.8395847;
					break;
				case 14:
					a=0.0000062601892;
					b=2.1588132;
					c=5.5866699;
					break;
				case 15:
					a=0.00000027748628;
					b=-0.0031671959;
					c=6.2349744;
					break;
				case 16:
					a=0.0000011401659;
					b=1.8660343;
					c=5.7884287;
					break;
				case 18:
					a=0.00000093928003;
					b=2.9224069;
					c=5.6503159;
					break;
				case 20:
					a=0.00000003478867;
					b=0.44735558;
					c=6.2388156;
					break;
				case 24:
					a=0.0000000038623391;
					b=-0.4486039;
					c=6.4245686;
					break;
				case 28:
					a=0.000000039660207;
					b=1.6425505;
					c=5.8842817;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Shots
		case 2:
			switch ( org )
			{
				case 1:
					return 720;
					break;
				case 2:
					if ( value==2 ) return 79;
					else return 641;
					break;
				default:
					return 0;
					break;
			}
			break;
			// Reichweite, Scan
		case 3:
			switch ( org )
			{
				case 3:
					if ( value==3 ) return 61;
					else return 299;
					break;
				case 4:
					a=0.010226741;
					b=-0.001141961;
					c=5.8477272;
					break;
				case 5:
					a=0.00074684696;
					b=-0.24064936;
					c=6.2377712;
					break;
				case 6:
					a=0.0000004205569;
					b=-2.5074874;
					c=8.1868728;
					break;
				case 7:
					a=0.00018753949;
					b=0.42735532;
					c=5.9259322;
					break;
				case 8:
					a=0.000026278484;
					b=0.0026600724;
					c=6.2281618;
					break;
				case 9:
					a=0.000017724816;
					b=0.35087138;
					c=6.1028354;
					break;
				case 10:
					a=0.000011074461;
					b=-0.41358078;
					c=6.2067919;
					break;
				case 11:
					a=0.0000022011968;
					b=-0.97456761;
					c=6.4502985;
					break;
				case 12:
					a=0.0000000034515189;
					b=-4.4597674;
					c=7.9715326;
					break;
				case 14:
					a=0.0000028257552;
					b=0.78730358;
					c=5.9483863;
					break;
				case 18:
					a=0.00000024289322;
					b=0.64536566;
					c=6.11706;
					break;
				default:
					return 0;
					break;
			}
			break;
		default:
			return 0;
	}

	tmp = (int) Round((a * pow((value-b), c) ), 0);
	return tmp;
*/



//---------------------------------------------------------------------
// R E S E A R C H   C L A S S ----------------------------------------
//---------------------------------------------------------------------

//---------------------------------------------------------------------
cResearch::cResearch ()
{
	init ();
}

//---------------------------------------------------------------------
void cResearch::init ()
{
	for (int i = 0; i < kNrResearchAreas; i++)
	{
		curResearchLevel[i] = 0;
		curResearchPoints[i] = 0;
		neededResearchPoints[i] = cUpgradeCalculator::instance ().calcResearchTurns (0, getUpgradeCalculatorUpgradeType (i));
	}
}

//---------------------------------------------------------------------
int cResearch::getCurResearchLevel (int researchArea) const
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas)
		return curResearchLevel[researchArea];
	return 0;
}

//---------------------------------------------------------------------
int cResearch::getCurResearchPoints (int researchArea) const
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas)
		return curResearchPoints[researchArea];
	return 0;
}

//---------------------------------------------------------------------
int cResearch::getNeededResearchPoints (int researchArea) const
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas)
		return neededResearchPoints[researchArea];
	return cUpgradeCalculator::kNoResearchAvailable;
}

//---------------------------------------------------------------------
int cResearch::getRemainingTurns (int researchArea, int centersWorkingOn) const
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas && centersWorkingOn > 0)
	{
		int remainingPoints = getRemainingResearchPoints(researchArea);
		if (remainingPoints % centersWorkingOn == 0)
			return remainingPoints / centersWorkingOn;
		else
			return (remainingPoints / centersWorkingOn) + 1;
	}
	return 0;
}

//---------------------------------------------------------------------
void cResearch::setCurResearchLevel (int researchLevel, int researchArea)
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas && researchLevel >= 0 && researchLevel % 10 == 0)
	{
		curResearchLevel[researchArea] = researchLevel;
		neededResearchPoints[researchArea] = cUpgradeCalculator::instance ().calcResearchTurns (researchLevel, getUpgradeCalculatorUpgradeType (researchArea));
		if (curResearchPoints[researchArea] >= neededResearchPoints[researchArea])
			curResearchPoints[researchArea] = 0;
	}
}

//---------------------------------------------------------------------
void cResearch::setCurResearchPoints (int researchPoints, int researchArea)
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas && researchPoints >= 0 && researchPoints < neededResearchPoints[researchArea])
		curResearchPoints[researchArea] = researchPoints;
}

//---------------------------------------------------------------------
bool cResearch::doResearch (int researchPoints, int researchArea)
{
	if (0 <= researchArea && researchArea <= kNrResearchAreas && researchPoints > 0)
	{
		curResearchPoints[researchArea] += researchPoints;
		if (curResearchPoints[researchArea] >= neededResearchPoints [researchArea])
		{
			curResearchPoints[researchArea] = 0;
			curResearchLevel[researchArea] += 10;
			neededResearchPoints[researchArea] = cUpgradeCalculator::instance ().calcResearchTurns (curResearchLevel[researchArea], 
																									getUpgradeCalculatorUpgradeType (researchArea));
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------
int cResearch::getUpgradeCalculatorUpgradeType (int researchArea) const
{
	switch (researchArea)
	{
		case kHitpointsResearch: return cUpgradeCalculator::kHitpoints;
		case kArmorResearch: return cUpgradeCalculator::kArmor;
		case kAttackResearch: return cUpgradeCalculator::kAttack;
		case kSpeedResearch: return cUpgradeCalculator::kSpeed;
		case kShotsResearch: return cUpgradeCalculator::kShots;
		case kRangeResearch: return cUpgradeCalculator::kRange;
		case kScanResearch: return cUpgradeCalculator::kScan;
		case kCostResearch: return cUpgradeCalculator::kCost;
	}
	return 0;
}

//---------------------------------------------------------------------
int cResearch::getResearchArea (int upgradeCalculatorType) const
{
	switch (upgradeCalculatorType)
	{
		case cUpgradeCalculator::kHitpoints: return kHitpointsResearch;
		case cUpgradeCalculator::kArmor: return kArmorResearch;
		case cUpgradeCalculator::kAmmo: return -1;
		case cUpgradeCalculator::kAttack: return kAttackResearch;
		case cUpgradeCalculator::kSpeed: return kSpeedResearch;
		case cUpgradeCalculator::kShots: return kShotsResearch;
		case cUpgradeCalculator::kRange: return kRangeResearch;
		case cUpgradeCalculator::kScan: return kScanResearch;
		case cUpgradeCalculator::kCost: return kCostResearch;
	}
	return 0;	
}

