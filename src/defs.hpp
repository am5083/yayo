/*
**    Yayo is a UCI chess engine written by am5083 (am@kvasm.us)
**    Copyright (C) 2022 Ahmed Mohamed (am@kvasm.us)
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEFS_H_
#define DEFS_H_
#include "bitboard.hpp"
#include "util.hpp"
#include <array>

namespace Yayo {
constexpr auto g_rookMagics = std::array<std::uint64_t, 64>({
      0x800080221a4000ULL,   0x2040002000401000ULL, 0xa900090010422000ULL,
      0x200041140200a00ULL,  0x1001008040200810ULL, 0x200100200080401ULL,
      0x280108002000100ULL,  0x200041500802242ULL,  0x200a002080420100ULL,
      0x400c808040002000ULL, 0x216801001200080ULL,  0x8201001000082100ULL,
      0x2c40800800800400ULL, 0x60808002000400ULL,   0x9021800100800200ULL,
      0x6601000040810002ULL, 0x2080004020004001ULL, 0x1010024000402009ULL,
      0x2000808020001000ULL, 0x26020040201208ULL,   0x5004008004080081ULL,
      0x100808004000200ULL,  0x40002100801ULL,      0x4800020010440389ULL,
      0x6200400080008020ULL, 0xa0008280400220ULL,   0x40120200208040ULL,
      0x1288002101001000ULL, 0x800080080040080ULL,  0x32802008010410c0ULL,
      0x4202020400100801ULL, 0x80c00042000c00a1ULL, 0x4080004081002100ULL,
      0x2800804000802001ULL, 0x40100080802000ULL,   0x201000821001002ULL,
      0x8001009000500ULL,    0x400a001492006810ULL, 0x12508804000142ULL,
      0x8021000045000882ULL, 0x2000204000908002ULL, 0x240100028006000ULL,
      0x2880110020010040ULL, 0x1021002210008ULL,    0x8020004004040ULL,
      0x9000804010002ULL,    0x14020001008080ULL,   0x9004304401820005ULL,
      0x220100408200ULL,     0x50401008200040ULL,   0x201801000200480ULL,
      0x6485022008100100ULL, 0x1008440008008280ULL, 0x9000400080300ULL,
      0x20080110020400ULL,   0x8440802100004080ULL, 0x210046128001ULL,
      0x10010240002a1081ULL, 0xc81114008200501ULL,  0x4100281000a015ULL,
      0x32002008041002ULL,   0x3206000130082422ULL, 0xc101801122084ULL,
      0x8084030840102ULL,
});

constexpr auto g_bishopMagics = std::array<Bitboard, 64>{
      0xffedf9fd7cfcffff,    0x222040804a90040ULL,  0x1010042048400060ULL,
      0x44040088828020ULL,   0xa008484140300880ULL, 0x2226020020080ULL,
      0x21011003a00484ULL,   0x100442084202001ULL,  0x8046425848009880ULL,
      0x143021001120098ULL,  0x220204082000ULL,     0x2a80944400808880ULL,
      0x2811040400308ULL,    0x40011048040108ULL,   0x300410802110438ULL,
      0x200820504024200ULL,  0x840881022080120ULL,  0x20c7104080481ULL,
      0x4400020802010cULL,   0x8018009028401000ULL, 0x486000400a2000cULL,
      0x2000200d00884002ULL, 0x4000212020280ULL,    0x8000808030880801ULL,
      0x5810880850200161ULL, 0x30c80350210108ULL,   0x206a40088254400ULL,
      0xb284004004010102ULL, 0x1006840008802000ULL, 0x1c30010008825102ULL,
      0x84840001015800ULL,   0x200410004440200ULL,  0x210101308240c21ULL,
      0x40823000206402ULL,   0x2084040400020020ULL, 0x6801040401080120ULL,
      0x1240010100111040ULL, 0x4800880040020111ULL, 0x808020440048800ULL,
      0x1404200048200ULL,    0x2804100410040400ULL, 0x1308820000400ULL,
      0x90041042280d1002ULL, 0x1020820214010200ULL, 0x5600204410100102ULL,
      0x40008a04100080ULL,   0x8021010208840208ULL, 0x8230011040800101ULL,
      0x40248c3008050200ULL, 0x2001010090042821ULL, 0x80210808080cULL,
      0x201820b041108000ULL, 0x204c141082020004ULL, 0x880850810244000ULL,
      0x4084c408861340ULL,   0x811010104008000ULL,  0x2081120811041000ULL,
      0x808c020200840504ULL, 0x85210021080805ULL,   0x90000200840402ULL,
      0x800040104128ULL,     0xa08450220201ULL,     0x41680804a292ULL,
      0x2022812108200ULL,
};

constexpr auto bishopBitCounts = std::array{
      6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
      5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7,
      7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6,
};

constexpr auto rookBitCounts = std::array{
      12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
      11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12,
};

} // namespace Yayo
#endif // DEFS_H_
