#ifndef WEIGHTS_H_
#define WEIGHTS_H_
#include "util.hpp"
constexpr Score pawnScore = S(106, 139);
constexpr Score knightScore = S(385, 411);
constexpr Score bishopScore = S(380, 419);
constexpr Score rookScore = S(516, 680);
constexpr Score queenScore = S(1112, 1355);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),      S(0, 0),    S(0, 0),
      S(0, 0),    S(0, 0),    S(33, 134),  S(24, 110),   S(-4, 122), S(24, 61),
      S(-16, 63), S(17, 54),  S(-98, 123), S(-110, 154), S(-22, 89), S(8, 89),
      S(32, 65),  S(35, 16),  S(19, 3),    S(71, 27),    S(69, 51),  S(5, 94),
      S(-8, 50),  S(-32, 41), S(16, 32),   S(20, -26),   S(17, 10),  S(13, -12),
      S(-7, 38),  S(-6, 16),  S(-49, 50),  S(-12, 11),   S(-19, -9), S(14, 8),
      S(16, -20), S(26, 11),  S(11, -10),  S(-24, -6),   S(-13, 39), S(-11, 34),
      S(-13, -6), S(-11, 23), S(20, -6),   S(25, 3),     S(31, -11), S(-14, 17),
      S(-7, 37),  S(-15, 28), S(-20, 18),  S(-18, -3),   S(-7, 0),   S(40, 25),
      S(55, -15), S(-12, -5), S(0, 0),     S(0, 0),      S(0, 0),    S(0, 0),
      S(0, 0),    S(0, 0),    S(0, 0),     S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-141, -31), S(-94, -54), S(-58, -30), S(-2, -30),  S(27, -29),
      S(-88, -20),  S(-23, -20), S(-74, -81), S(-9, -11),  S(18, -16),
      S(65, -9),    S(36, 12),   S(55, -32),  S(76, -35),  S(34, 1),
      S(32, -20),   S(-35, -6),  S(53, -1),   S(40, 16),   S(62, 25),
      S(118, -12),  S(138, 3),   S(53, -40),  S(30, -47),  S(-14, -9),
      S(-2, 1),     S(20, 26),   S(52, 19),   S(32, 20),   S(52, 13),
      S(15, 24),    S(80, -16),  S(10, -1),   S(22, 23),   S(3, 34),
      S(13, 45),    S(19, 24),   S(20, 43),   S(36, -13),  S(9, -9),
      S(-34, -33),  S(-31, -17), S(16, 12),   S(-6, 20),   S(22, 39),
      S(23, 13),    S(26, -31),  S(-17, 12),  S(-59, -21), S(-39, 21),
      S(-28, -21),  S(19, 15),   S(20, 8),    S(-9, -20),  S(19, 1),
      S(-23, 7),    S(-103, -2), S(-29, -39), S(-17, -22), S(-26, 23),
      S(12, 7),     S(9, -27),   S(2, 8),     S(-50, -32),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(4, 22),    S(-58, -14), S(-82, -8),  S(-90, -11), S(-26, 11),
      S(-39, 19),  S(3, 18),    S(-36, 6),   S(6, -28),   S(24, 6),
      S(-29, 11),  S(-32, -15), S(-5, -1),   S(-3, 3),    S(54, 14),
      S(8, -29),   S(7, -2),    S(37, 14),   S(11, -10),  S(52, -1),
      S(34, 8),    S(41, -12),  S(37, 4),    S(17, 19),   S(-30, -17),
      S(23, 23),   S(-2, -1),   S(32, 2),    S(15, 19),   S(38, 26),
      S(-5, -3),   S(-20, 18),  S(-25, -19), S(-20, -1),  S(-8, 34),
      S(23, -1),   S(13, 8),    S(19, 0),    S(-20, 27),  S(11, -35),
      S(-13, -6),  S(23, 22),   S(-2, 0),    S(6, 13),    S(42, 3),
      S(5, 30),    S(35, 2),    S(42, -22),  S(-2, -6),   S(26, -21),
      S(41, -7),   S(18, -7),   S(-2, 22),   S(12, -17),  S(18, 14),
      S(4, -17),   S(-20, 5),   S(49, 3),    S(0, -1),    S(-18, -7),
      S(-17, -12), S(7, 32),    S(-1, -20),  S(27, 11),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(44, 4),   S(28, 23),   S(31, 24),  S(40, 14),  S(72, 5),   S(17, 10),
      S(63, 4),   S(52, 1),    S(-12, 38), S(-2, 16),  S(42, 21),  S(80, -2),
      S(36, 16),  S(88, -5),   S(24, 9),   S(55, -9),  S(15, 12),  S(16, 26),
      S(37, 2),   S(52, 5),    S(38, 25),  S(59, -21), S(121, 7),  S(49, 20),
      S(-31, 40), S(23, 30),   S(-13, 18), S(-4, 5),   S(37, 17),  S(40, -12),
      S(22, -3),  S(17, -17),  S(-19, -1), S(-13, 1),  S(-4, 8),   S(-34, 4),
      S(-25, -2), S(-9, 27),   S(-6, 21),  S(-23, -4), S(-51, 3),  S(-43, 3),
      S(-32, 1),  S(-2, -7),   S(5, -7),   S(-18, 2),  S(22, -32), S(11, 2),
      S(-28, -1), S(-14, 14),  S(-26, -7), S(-26, -8), S(13, -14), S(-10, -25),
      S(17, -27), S(-50, -16), S(-35, -2), S(-3, -6),  S(9, 17),   S(21, 3),
      S(-1, 12),  S(-13, -16), S(24, 10),  S(-20, 8),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(-57, 55), S(-6, 29),   S(3, 43),    S(34, 77),  S(50, 65),  S(59, 24),
      S(57, 28),  S(29, 14),   S(-2, 40),   S(-39, 56), S(-5, 81),  S(-22, 102),
      S(-16, 89), S(43, 22),   S(1, 19),    S(58, 19),  S(4, 42),   S(-44, 50),
      S(6, 70),   S(-33, 102), S(36, 91),   S(81, 69),  S(96, 30),  S(54, 38),
      S(-10, 18), S(-24, 49),  S(-5, 62),   S(-5, 83),  S(13, 97),  S(28, 94),
      S(35, 93),  S(6, 65),    S(-16, 25),  S(-13, 57), S(-44, 32), S(-1, 87),
      S(-31, 89), S(-1, 90),   S(-14, 36),  S(-2, 47),  S(3, -10),  S(5, 33),
      S(-26, 48), S(-29, 57),  S(-31, 77),  S(19, 45),  S(27, 1),   S(-5, 49),
      S(-28, 25), S(4, 1),     S(-11, -24), S(26, 6),   S(24, 20),  S(2, -36),
      S(1, -34),  S(47, -21),  S(13, -28),  S(7, -18),  S(-6, -43), S(6, -26),
      S(-8, -31), S(10, 9),    S(18, 2),    S(-7, -21),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-54, -87), S(-10, -83), S(30, -50),   S(-45, 8),   S(-81, -3),
      S(-44, 15),  S(-2, 27),   S(-12, -105), S(-37, -52), S(-32, -3),
      S(-54, 35),  S(-22, 38),  S(16, 41),    S(-13, 49),  S(44, 34),
      S(26, 18),   S(-54, 10),  S(22, 27),    S(-15, 56),  S(-6, 67),
      S(-35, 49),  S(17, 53),   S(32, 41),    S(11, 20),   S(-57, -2),
      S(-51, 14),  S(-44, 29),  S(-101, 56),  S(-88, 77),  S(-95, 47),
      S(-89, 59),  S(-59, -12), S(-70, -9),   S(-31, -1),  S(-96, 26),
      S(-153, 74), S(-173, 53), S(-123, 45),  S(-133, 42), S(-151, -11),
      S(-68, -50), S(-59, 12),  S(-114, 35),  S(-140, 28), S(-147, 58),
      S(-106, 18), S(-84, -3),  S(-113, 8),   S(21, -31),  S(-26, 0),
      S(-15, 11),  S(-80, 8),   S(-66, -1),   S(-70, 10),  S(-7, -7),
      S(29, -27),  S(54, -100), S(82, -90),   S(56, -35),  S(-74, -31),
      S(-6, -23),  S(-20, -49), S(60, -79),   S(41, -96),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),   S(9, 16),   S(-29, 29),  S(4, 61),
      S(21, 96), S(37, 160), S(107, 204), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10), S(-14, -25), S(6, -37),   S(-14, -18),
      S(21, -70),  S(-84, -70), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),   S(-24, 9),  S(-15, 0),  S(-29, 1),
      S(-20, -15), S(-9, -21), S(-22, -7), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-31, -25), S(-16, 4),   S(-12, -1),
      S(-33, -15), S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-42, -38), S(0, 31),  S(-5, 10), S(17, 17), S(6, 46),
      S(9, 63),    S(46, 57), S(29, 45), S(55, 15),
};
constexpr Score BishopMobilityScore[14] = {
      S(-28, -34), S(0, -7),  S(11, 16), S(40, 30),  S(51, 61),
      S(41, 55),   S(50, 83), S(61, 66), S(81, 80),  S(81, 89),
      S(87, 84),   S(42, 93), S(98, 74), S(87, 100),
};
constexpr Score RookMobilityScore[15] = {
      S(-42, 41), S(-19, 60), S(-23, 80), S(11, 86),  S(11, 93),
      S(-4, 111), S(28, 141), S(14, 130), S(34, 130), S(46, 158),
      S(55, 153), S(68, 150), S(50, 148), S(46, 175), S(54, 158),
};
constexpr Score QueenMobilityScore[28] = {
      S(-71, -93), S(-36, -50), S(-13, -27), S(-5, 23),   S(12, 53),
      S(36, 73),   S(21, 91),   S(39, 103),  S(34, 99),   S(53, 124),
      S(64, 151),  S(79, 147),  S(75, 142),  S(51, 181),  S(85, 185),
      S(83, 204),  S(86, 176),  S(91, 208),  S(102, 219), S(101, 197),
      S(137, 210), S(152, 188), S(135, 214), S(145, 187), S(107, 207),
      S(138, 179), S(139, 218), S(102, 204),
};

struct EvalWeights {
    const Score pawnScore = S(106, 139);
    const Score knightScore = S(385, 411);
    const Score bishopScore = S(380, 419);
    const Score rookScore = S(516, 680);
    const Score queenScore = S(1112, 1355);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(33, 134), S(24, 110),
          S(-4, 122),   S(24, 61),  S(-16, 63), S(17, 54),  S(-98, 123),
          S(-110, 154), S(-22, 89), S(8, 89),   S(32, 65),  S(35, 16),
          S(19, 3),     S(71, 27),  S(69, 51),  S(5, 94),   S(-8, 50),
          S(-32, 41),   S(16, 32),  S(20, -26), S(17, 10),  S(13, -12),
          S(-7, 38),    S(-6, 16),  S(-49, 50), S(-12, 11), S(-19, -9),
          S(14, 8),     S(16, -20), S(26, 11),  S(11, -10), S(-24, -6),
          S(-13, 39),   S(-11, 34), S(-13, -6), S(-11, 23), S(20, -6),
          S(25, 3),     S(31, -11), S(-14, 17), S(-7, 37),  S(-15, 28),
          S(-20, 18),   S(-18, -3), S(-7, 0),   S(40, 25),  S(55, -15),
          S(-12, -5),   S(0, 0),    S(0, 0),    S(0, 0),    S(0, 0),
          S(0, 0),      S(0, 0),    S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-141, -31), S(-94, -54), S(-58, -30), S(-2, -30),  S(27, -29),
          S(-88, -20),  S(-23, -20), S(-74, -81), S(-9, -11),  S(18, -16),
          S(65, -9),    S(36, 12),   S(55, -32),  S(76, -35),  S(34, 1),
          S(32, -20),   S(-35, -6),  S(53, -1),   S(40, 16),   S(62, 25),
          S(118, -12),  S(138, 3),   S(53, -40),  S(30, -47),  S(-14, -9),
          S(-2, 1),     S(20, 26),   S(52, 19),   S(32, 20),   S(52, 13),
          S(15, 24),    S(80, -16),  S(10, -1),   S(22, 23),   S(3, 34),
          S(13, 45),    S(19, 24),   S(20, 43),   S(36, -13),  S(9, -9),
          S(-34, -33),  S(-31, -17), S(16, 12),   S(-6, 20),   S(22, 39),
          S(23, 13),    S(26, -31),  S(-17, 12),  S(-59, -21), S(-39, 21),
          S(-28, -21),  S(19, 15),   S(20, 8),    S(-9, -20),  S(19, 1),
          S(-23, 7),    S(-103, -2), S(-29, -39), S(-17, -22), S(-26, 23),
          S(12, 7),     S(9, -27),   S(2, 8),     S(-50, -32),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(4, 22),    S(-58, -14), S(-82, -8),  S(-90, -11), S(-26, 11),
          S(-39, 19),  S(3, 18),    S(-36, 6),   S(6, -28),   S(24, 6),
          S(-29, 11),  S(-32, -15), S(-5, -1),   S(-3, 3),    S(54, 14),
          S(8, -29),   S(7, -2),    S(37, 14),   S(11, -10),  S(52, -1),
          S(34, 8),    S(41, -12),  S(37, 4),    S(17, 19),   S(-30, -17),
          S(23, 23),   S(-2, -1),   S(32, 2),    S(15, 19),   S(38, 26),
          S(-5, -3),   S(-20, 18),  S(-25, -19), S(-20, -1),  S(-8, 34),
          S(23, -1),   S(13, 8),    S(19, 0),    S(-20, 27),  S(11, -35),
          S(-13, -6),  S(23, 22),   S(-2, 0),    S(6, 13),    S(42, 3),
          S(5, 30),    S(35, 2),    S(42, -22),  S(-2, -6),   S(26, -21),
          S(41, -7),   S(18, -7),   S(-2, 22),   S(12, -17),  S(18, 14),
          S(4, -17),   S(-20, 5),   S(49, 3),    S(0, -1),    S(-18, -7),
          S(-17, -12), S(7, 32),    S(-1, -20),  S(27, 11),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(44, 4),    S(28, 23),   S(31, 24),  S(40, 14),   S(72, 5),
          S(17, 10),   S(63, 4),    S(52, 1),   S(-12, 38),  S(-2, 16),
          S(42, 21),   S(80, -2),   S(36, 16),  S(88, -5),   S(24, 9),
          S(55, -9),   S(15, 12),   S(16, 26),  S(37, 2),    S(52, 5),
          S(38, 25),   S(59, -21),  S(121, 7),  S(49, 20),   S(-31, 40),
          S(23, 30),   S(-13, 18),  S(-4, 5),   S(37, 17),   S(40, -12),
          S(22, -3),   S(17, -17),  S(-19, -1), S(-13, 1),   S(-4, 8),
          S(-34, 4),   S(-25, -2),  S(-9, 27),  S(-6, 21),   S(-23, -4),
          S(-51, 3),   S(-43, 3),   S(-32, 1),  S(-2, -7),   S(5, -7),
          S(-18, 2),   S(22, -32),  S(11, 2),   S(-28, -1),  S(-14, 14),
          S(-26, -7),  S(-26, -8),  S(13, -14), S(-10, -25), S(17, -27),
          S(-50, -16), S(-35, -2),  S(-3, -6),  S(9, 17),    S(21, 3),
          S(-1, 12),   S(-13, -16), S(24, 10),  S(-20, 8),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(-57, 55),  S(-6, 29),   S(3, 43),   S(34, 77),  S(50, 65),
          S(59, 24),   S(57, 28),   S(29, 14),  S(-2, 40),  S(-39, 56),
          S(-5, 81),   S(-22, 102), S(-16, 89), S(43, 22),  S(1, 19),
          S(58, 19),   S(4, 42),    S(-44, 50), S(6, 70),   S(-33, 102),
          S(36, 91),   S(81, 69),   S(96, 30),  S(54, 38),  S(-10, 18),
          S(-24, 49),  S(-5, 62),   S(-5, 83),  S(13, 97),  S(28, 94),
          S(35, 93),   S(6, 65),    S(-16, 25), S(-13, 57), S(-44, 32),
          S(-1, 87),   S(-31, 89),  S(-1, 90),  S(-14, 36), S(-2, 47),
          S(3, -10),   S(5, 33),    S(-26, 48), S(-29, 57), S(-31, 77),
          S(19, 45),   S(27, 1),    S(-5, 49),  S(-28, 25), S(4, 1),
          S(-11, -24), S(26, 6),    S(24, 20),  S(2, -36),  S(1, -34),
          S(47, -21),  S(13, -28),  S(7, -18),  S(-6, -43), S(6, -26),
          S(-8, -31),  S(10, 9),    S(18, 2),   S(-7, -21),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-54, -87), S(-10, -83), S(30, -50),   S(-45, 8),   S(-81, -3),
          S(-44, 15),  S(-2, 27),   S(-12, -105), S(-37, -52), S(-32, -3),
          S(-54, 35),  S(-22, 38),  S(16, 41),    S(-13, 49),  S(44, 34),
          S(26, 18),   S(-54, 10),  S(22, 27),    S(-15, 56),  S(-6, 67),
          S(-35, 49),  S(17, 53),   S(32, 41),    S(11, 20),   S(-57, -2),
          S(-51, 14),  S(-44, 29),  S(-101, 56),  S(-88, 77),  S(-95, 47),
          S(-89, 59),  S(-59, -12), S(-70, -9),   S(-31, -1),  S(-96, 26),
          S(-153, 74), S(-173, 53), S(-123, 45),  S(-133, 42), S(-151, -11),
          S(-68, -50), S(-59, 12),  S(-114, 35),  S(-140, 28), S(-147, 58),
          S(-106, 18), S(-84, -3),  S(-113, 8),   S(21, -31),  S(-26, 0),
          S(-15, 11),  S(-80, 8),   S(-66, -1),   S(-70, 10),  S(-7, -7),
          S(29, -27),  S(54, -100), S(82, -90),   S(56, -35),  S(-74, -31),
          S(-6, -23),  S(-20, -49), S(60, -79),   S(41, -96),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),   S(9, 16),   S(-29, 29),  S(4, 61),
          S(21, 96), S(37, 160), S(107, 204), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10), S(-14, -25), S(6, -37),   S(-14, -18),
          S(21, -70),  S(-84, -70), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),   S(-24, 9),  S(-15, 0),  S(-29, 1),
          S(-20, -15), S(-9, -21), S(-22, -7), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-31, -25), S(-16, 4),   S(-12, -1),
          S(-33, -15), S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-42, -38), S(0, 31),  S(-5, 10), S(17, 17), S(6, 46),
          S(9, 63),    S(46, 57), S(29, 45), S(55, 15),
    };
    const Score BishopMobilityScore[14] = {
          S(-28, -34), S(0, -7),  S(11, 16), S(40, 30),  S(51, 61),
          S(41, 55),   S(50, 83), S(61, 66), S(81, 80),  S(81, 89),
          S(87, 84),   S(42, 93), S(98, 74), S(87, 100),
    };
    const Score RookMobilityScore[15] = {
          S(-42, 41), S(-19, 60), S(-23, 80), S(11, 86),  S(11, 93),
          S(-4, 111), S(28, 141), S(14, 130), S(34, 130), S(46, 158),
          S(55, 153), S(68, 150), S(50, 148), S(46, 175), S(54, 158),
    };
    const Score QueenMobilityScore[28] = {
          S(-71, -93), S(-36, -50), S(-13, -27), S(-5, 23),   S(12, 53),
          S(36, 73),   S(21, 91),   S(39, 103),  S(34, 99),   S(53, 124),
          S(64, 151),  S(79, 147),  S(75, 142),  S(51, 181),  S(85, 185),
          S(83, 204),  S(86, 176),  S(91, 208),  S(102, 219), S(101, 197),
          S(137, 210), S(152, 188), S(135, 214), S(145, 187), S(107, 207),
          S(138, 179), S(139, 218), S(102, 204),
    };
};
#endif // WEIGHTS_H_
