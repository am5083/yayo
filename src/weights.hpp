#ifndef WEIGHTS_H_
#define WEIGHTS_H_
#include "util.hpp"

constexpr Score pawnScore = S(106, 159);
constexpr Score knightScore = S(384, 424);
constexpr Score bishopScore = S(395, 453);
constexpr Score rookScore = S(523, 702);
constexpr Score queenScore = S(1175, 1433);

constexpr Score taperedPawnPcSq[SQUARE_CT] = {
      S(0, 0),      S(0, 0),     S(0, 0),    S(0, 0),     S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),    S(-29, 192), S(98, 111),
      S(-37, 166),  S(36, 53),   S(-12, -8), S(-51, 20),  S(-104, 84),
      S(-223, 162), S(-37, 151), S(-3, 147), S(64, 15),   S(-12, 2),
      S(74, -5),    S(140, 45),  S(81, 77),  S(24, 75),   S(-33, 113),
      S(-51, 71),   S(-27, 42),  S(-2, 12),  S(12, -23),  S(24, 1),
      S(-34, 48),   S(-27, 61),  S(-64, 80), S(-39, 70),  S(-22, 19),
      S(3, 1),      S(-6, -1),   S(0, 14),   S(-20, 30),  S(-20, 30),
      S(-30, 56),   S(-26, 35),  S(-8, 10),  S(-10, 12),  S(7, 12),
      S(8, 14),     S(36, 17),   S(7, 23),   S(-39, 52),  S(-25, 54),
      S(-21, 23),   S(8, 10),    S(-10, 15), S(41, 15),   S(40, 15),
      S(0, -7),     S(0, 0),     S(0, 0),    S(0, 0),     S(0, 0),
      S(0, 0),      S(0, 0),     S(0, 0),    S(0, 0),
};
constexpr Score taperedKnightPcSq[SQUARE_CT] = {
      S(-193, -105), S(-102, -59), S(-62, -18), S(-79, -24), S(66, 18),
      S(-158, 17),   S(-110, -42), S(-53, -98), S(7, -9),    S(42, 36),
      S(-36, 2),     S(1, 36),     S(53, -56),  S(118, -74), S(87, 49),
      S(75, -59),    S(6, -28),    S(35, -33),  S(24, 73),   S(54, 72),
      S(145, -10),   S(111, 16),   S(68, 13),   S(67, -52),  S(11, 53),
      S(19, 11),     S(23, 56),    S(67, 77),   S(29, 86),   S(100, 43),
      S(25, 10),     S(120, -13),  S(-3, 56),   S(-19, 43),  S(22, 58),
      S(6, 76),      S(25, 51),    S(23, 29),   S(40, 45),   S(27, 57),
      S(-30, 56),    S(-26, -12),  S(-10, 24),  S(0, 34),    S(34, 9),
      S(13, 11),     S(23, 24),    S(30, -40),  S(-73, 5),   S(-85, -48),
      S(-31, -12),   S(3, 6),      S(0, -13),   S(7, 11),    S(15, -102),
      S(0, -79),     S(-64, 17),   S(-43, -21), S(-46, 29),  S(20, -60),
      S(37, -35),    S(28, -45),   S(-39, -9),  S(-43, -59),
};
constexpr Score taperedBishopPcSq[SQUARE_CT] = {
      S(-112, 80), S(-53, 7),   S(-20, 92),  S(-68, -40), S(-3, -16),
      S(-125, 25), S(43, 28),   S(-76, -77), S(8, -9),    S(48, -2),
      S(-41, 44),  S(-28, -12), S(72, -53),  S(-61, 37),  S(-16, -18),
      S(-44, -37), S(-37, 50),  S(26, 32),   S(29, 37),   S(22, -3),
      S(4, -29),   S(128, -3),  S(71, -60),  S(37, -37),  S(-30, 3),
      S(-17, 48),  S(6, 15),    S(78, 13),   S(53, 10),   S(4, 32),
      S(10, 15),   S(-11, 1),   S(44, 28),   S(-9, 20),   S(-25, 55),
      S(28, 51),   S(15, 40),   S(-8, 33),   S(-28, 36),  S(10, -30),
      S(-8, 25),   S(45, 12),   S(7, 18),    S(1, 27),    S(13, 53),
      S(1, 47),    S(34, 42),   S(14, 20),   S(15, -18),  S(19, -13),
      S(20, -28),  S(-7, 17),   S(-20, 14),  S(54, -11),  S(13, 20),
      S(45, -29),  S(29, -33),  S(9, -28),   S(-17, 58),  S(-1, 28),
      S(-8, 41),   S(-18, 11),  S(-39, -50), S(55, -40),
};
constexpr Score taperedRookPcSq[SQUARE_CT] = {
      S(-21, 72),  S(71, 65),  S(20, 59),   S(57, 35),  S(116, -8),
      S(94, 42),   S(40, 57),  S(58, 68),   S(-53, 69), S(-7, 44),
      S(25, 46),   S(38, 35),  S(80, -31),  S(77, 21),  S(-29, 20),
      S(78, 29),   S(55, 3),   S(62, 43),   S(21, 37),  S(41, 6),
      S(125, -25), S(77, 27),  S(177, -51), S(79, -37), S(2, 26),
      S(-4, -4),   S(4, 50),   S(3, 21),    S(-30, 3),  S(44, -28),
      S(96, -14),  S(32, -30), S(-72, 23),  S(-33, 13), S(8, -21),
      S(-54, 29),  S(3, -1),   S(-31, -25), S(-63, -7), S(-37, -15),
      S(-31, 43),  S(-31, -2), S(-5, 35),   S(1, 10),   S(-32, 24),
      S(26, 6),    S(77, -11), S(32, -23),  S(-22, 37), S(-34, -30),
      S(5, -2),    S(2, 2),    S(-47, 22),  S(-4, 23),  S(48, -47),
      S(-96, -27), S(-30, 34), S(1, 11),    S(0, 25),   S(29, -2),
      S(25, -1),   S(13, -4),  S(-26, -6),  S(-44, 12),
};
constexpr Score taperedQueenPcSq[SQUARE_CT] = {
      S(2, -15),  S(-106, -85), S(6, 58),    S(-12, 78),  S(87, 68),
      S(37, 3),   S(88, -5),    S(47, 38),   S(-13, 1),   S(-94, 44),
      S(-2, 87),  S(-21, 67),   S(-81, 150), S(49, 102),  S(-82, 3),
      S(44, -35), S(-18, 46),   S(-23, 66),  S(-40, 100), S(28, 94),
      S(-1, 49),  S(52, 34),    S(65, -20),  S(47, 51),   S(-50, -6),
      S(-19, 1),  S(18, 11),    S(-22, 58),  S(-17, 122), S(43, 84),
      S(47, 74),  S(0, 54),     S(8, -3),    S(-41, 43),  S(-36, 4),
      S(-4, 98),  S(-34, 85),   S(-20, 146), S(18, 33),   S(7, 50),
      S(26, -35), S(-19, 60),   S(-38, 106), S(18, 9),    S(-45, 133),
      S(21, 33),  S(21, 65),    S(10, -81),  S(52, -5),   S(-8, 13),
      S(2, 9),    S(16, -49),   S(12, 24),   S(90, -68),  S(34, -94),
      S(71, -39), S(-25, -6),   S(16, -1),   S(32, -81),  S(44, -64),
      S(31, -8),  S(-7, -77),   S(13, -10),  S(48, 28),
};
constexpr Score taperedKingPcSq[SQUARE_CT] = {
      S(-187, -225), S(4, -96),    S(48, -38),   S(-23, 28),   S(-5, 13),
      S(-78, -9),    S(76, -19),   S(-30, -160), S(-132, -41), S(-8, -60),
      S(-4, 6),      S(33, -17),   S(11, 41),    S(-5, 61),    S(7, 57),
      S(60, -5),     S(-158, -36), S(46, 30),    S(-20, 28),   S(-6, 28),
      S(-55, 27),    S(-8, 44),    S(72, 37),    S(-31, 66),   S(-42, 18),
      S(-93, 39),    S(-111, 69),  S(-233, 101), S(-146, 71),  S(-132, 85),
      S(-127, 54),   S(-104, 34),  S(-203, 19),  S(-46, 15),   S(-171, 86),
      S(-127, 67),   S(-258, 94),  S(-175, 68),  S(-147, 47),  S(-298, 44),
      S(-51, -33),   S(-74, 24),   S(-123, 68),  S(-135, 50),  S(-206, 58),
      S(-161, 53),   S(-110, 12),  S(-191, 22),  S(32, -82),   S(-104, 21),
      S(-100, 35),   S(-120, 47),  S(-117, 15),  S(-85, 36),   S(-2, -2),
      S(55, -98),    S(-42, -91),  S(57, -87),   S(34, -42),   S(-98, 5),
      S(-10, -59),   S(-38, -40),  S(89, -113),  S(79, -151),
};
constexpr Score passedPawnRankBonus[8] = {
      S(0, 0),    S(24, 20),   S(20, -3),  S(21, 46),
      S(-6, 112), S(-14, 184), S(-3, 226), S(0, 0),
};
constexpr Score doubledPawnRankBonus[8] = {
      S(-10, -10),  S(5, -68),     S(19, -52),  S(-14, -49),
      S(-26, -168), S(-135, -137), S(-10, -10), S(-10, -10),
};
constexpr Score isolatedPawnRankBonus[8] = {
      S(-6, -6),  S(-19, 9),  S(-34, 3),  S(-30, 10),
      S(16, -32), S(-7, -15), S(52, -60), S(-6, -6),
};
constexpr Score backwardPawnRankBonus[8] = {
      S(-15, -15), S(-19, -3),  S(-4, -10),  S(-8, -18),
      S(-12, 14),  S(-15, -15), S(-15, -15), S(-15, -15),
};
constexpr Score KnightMobilityScore[9] = {
      S(-47, -61), S(-24, 31), S(-19, 44), S(20, 41), S(32, 53),
      S(38, 70),   S(44, 70),  S(65, 56),  S(62, 29),
};
constexpr Score BishopMobilityScore[14] = {
      S(-9, -25), S(39, -36), S(46, 43),   S(52, 57),   S(78, 71),
      S(81, 78),  S(91, 96),  S(100, 101), S(100, 108), S(101, 107),
      S(108, 99), S(73, 85),  S(109, 129), S(149, 51),
};
constexpr Score RookMobilityScore[15] = {
      S(-68, 17), S(-25, 42), S(-20, 110), S(-8, 111), S(-13, 130),
      S(2, 132),  S(8, 141),  S(8, 157),   S(35, 148), S(37, 164),
      S(61, 158), S(58, 171), S(77, 170),  S(58, 166), S(91, 152),
};
constexpr Score QueenMobilityScore[28] = {
      S(-167, -181), S(-21, -6),  S(33, -8),   S(-6, 75),   S(-19, 93),
      S(-8, 87),     S(-2, 109),  S(10, 145),  S(17, 189),  S(35, 183),
      S(40, 206),    S(34, 220),  S(55, 176),  S(60, 158),  S(51, 175),
      S(64, 201),    S(56, 218),  S(41, 209),  S(57, 219),  S(105, 178),
      S(95, 203),    S(92, 126),  S(163, 182), S(151, 153), S(143, 122),
      S(146, 175),   S(172, 204), S(204, 217),
};

struct EvalWeights {
    const Score pawnScore = S(106, 159);
    const Score knightScore = S(384, 424);
    const Score bishopScore = S(395, 453);
    const Score rookScore = S(523, 702);
    const Score queenScore = S(1175, 1433);

    const Score taperedPawnPcSq[SQUARE_CT] = {
          S(0, 0),      S(0, 0),     S(0, 0),    S(0, 0),     S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),    S(-29, 192), S(98, 111),
          S(-37, 166),  S(36, 53),   S(-12, -8), S(-51, 20),  S(-104, 84),
          S(-223, 162), S(-37, 151), S(-3, 147), S(64, 15),   S(-12, 2),
          S(74, -5),    S(140, 45),  S(81, 77),  S(24, 75),   S(-33, 113),
          S(-51, 71),   S(-27, 42),  S(-2, 12),  S(12, -23),  S(24, 1),
          S(-34, 48),   S(-27, 61),  S(-64, 80), S(-39, 70),  S(-22, 19),
          S(3, 1),      S(-6, -1),   S(0, 14),   S(-20, 30),  S(-20, 30),
          S(-30, 56),   S(-26, 35),  S(-8, 10),  S(-10, 12),  S(7, 12),
          S(8, 14),     S(36, 17),   S(7, 23),   S(-39, 52),  S(-25, 54),
          S(-21, 23),   S(8, 10),    S(-10, 15), S(41, 15),   S(40, 15),
          S(0, -7),     S(0, 0),     S(0, 0),    S(0, 0),     S(0, 0),
          S(0, 0),      S(0, 0),     S(0, 0),    S(0, 0),
    };
    const Score taperedKnightPcSq[SQUARE_CT] = {
          S(-193, -105), S(-102, -59), S(-62, -18), S(-79, -24), S(66, 18),
          S(-158, 17),   S(-110, -42), S(-53, -98), S(7, -9),    S(42, 36),
          S(-36, 2),     S(1, 36),     S(53, -56),  S(118, -74), S(87, 49),
          S(75, -59),    S(6, -28),    S(35, -33),  S(24, 73),   S(54, 72),
          S(145, -10),   S(111, 16),   S(68, 13),   S(67, -52),  S(11, 53),
          S(19, 11),     S(23, 56),    S(67, 77),   S(29, 86),   S(100, 43),
          S(25, 10),     S(120, -13),  S(-3, 56),   S(-19, 43),  S(22, 58),
          S(6, 76),      S(25, 51),    S(23, 29),   S(40, 45),   S(27, 57),
          S(-30, 56),    S(-26, -12),  S(-10, 24),  S(0, 34),    S(34, 9),
          S(13, 11),     S(23, 24),    S(30, -40),  S(-73, 5),   S(-85, -48),
          S(-31, -12),   S(3, 6),      S(0, -13),   S(7, 11),    S(15, -102),
          S(0, -79),     S(-64, 17),   S(-43, -21), S(-46, 29),  S(20, -60),
          S(37, -35),    S(28, -45),   S(-39, -9),  S(-43, -59),
    };
    const Score taperedBishopPcSq[SQUARE_CT] = {
          S(-112, 80), S(-53, 7),   S(-20, 92),  S(-68, -40), S(-3, -16),
          S(-125, 25), S(43, 28),   S(-76, -77), S(8, -9),    S(48, -2),
          S(-41, 44),  S(-28, -12), S(72, -53),  S(-61, 37),  S(-16, -18),
          S(-44, -37), S(-37, 50),  S(26, 32),   S(29, 37),   S(22, -3),
          S(4, -29),   S(128, -3),  S(71, -60),  S(37, -37),  S(-30, 3),
          S(-17, 48),  S(6, 15),    S(78, 13),   S(53, 10),   S(4, 32),
          S(10, 15),   S(-11, 1),   S(44, 28),   S(-9, 20),   S(-25, 55),
          S(28, 51),   S(15, 40),   S(-8, 33),   S(-28, 36),  S(10, -30),
          S(-8, 25),   S(45, 12),   S(7, 18),    S(1, 27),    S(13, 53),
          S(1, 47),    S(34, 42),   S(14, 20),   S(15, -18),  S(19, -13),
          S(20, -28),  S(-7, 17),   S(-20, 14),  S(54, -11),  S(13, 20),
          S(45, -29),  S(29, -33),  S(9, -28),   S(-17, 58),  S(-1, 28),
          S(-8, 41),   S(-18, 11),  S(-39, -50), S(55, -40),
    };
    const Score taperedRookPcSq[SQUARE_CT] = {
          S(-21, 72),  S(71, 65),  S(20, 59),   S(57, 35),  S(116, -8),
          S(94, 42),   S(40, 57),  S(58, 68),   S(-53, 69), S(-7, 44),
          S(25, 46),   S(38, 35),  S(80, -31),  S(77, 21),  S(-29, 20),
          S(78, 29),   S(55, 3),   S(62, 43),   S(21, 37),  S(41, 6),
          S(125, -25), S(77, 27),  S(177, -51), S(79, -37), S(2, 26),
          S(-4, -4),   S(4, 50),   S(3, 21),    S(-30, 3),  S(44, -28),
          S(96, -14),  S(32, -30), S(-72, 23),  S(-33, 13), S(8, -21),
          S(-54, 29),  S(3, -1),   S(-31, -25), S(-63, -7), S(-37, -15),
          S(-31, 43),  S(-31, -2), S(-5, 35),   S(1, 10),   S(-32, 24),
          S(26, 6),    S(77, -11), S(32, -23),  S(-22, 37), S(-34, -30),
          S(5, -2),    S(2, 2),    S(-47, 22),  S(-4, 23),  S(48, -47),
          S(-96, -27), S(-30, 34), S(1, 11),    S(0, 25),   S(29, -2),
          S(25, -1),   S(13, -4),  S(-26, -6),  S(-44, 12),
    };
    const Score taperedQueenPcSq[SQUARE_CT] = {
          S(2, -15),  S(-106, -85), S(6, 58),    S(-12, 78),  S(87, 68),
          S(37, 3),   S(88, -5),    S(47, 38),   S(-13, 1),   S(-94, 44),
          S(-2, 87),  S(-21, 67),   S(-81, 150), S(49, 102),  S(-82, 3),
          S(44, -35), S(-18, 46),   S(-23, 66),  S(-40, 100), S(28, 94),
          S(-1, 49),  S(52, 34),    S(65, -20),  S(47, 51),   S(-50, -6),
          S(-19, 1),  S(18, 11),    S(-22, 58),  S(-17, 122), S(43, 84),
          S(47, 74),  S(0, 54),     S(8, -3),    S(-41, 43),  S(-36, 4),
          S(-4, 98),  S(-34, 85),   S(-20, 146), S(18, 33),   S(7, 50),
          S(26, -35), S(-19, 60),   S(-38, 106), S(18, 9),    S(-45, 133),
          S(21, 33),  S(21, 65),    S(10, -81),  S(52, -5),   S(-8, 13),
          S(2, 9),    S(16, -49),   S(12, 24),   S(90, -68),  S(34, -94),
          S(71, -39), S(-25, -6),   S(16, -1),   S(32, -81),  S(44, -64),
          S(31, -8),  S(-7, -77),   S(13, -10),  S(48, 28),
    };
    const Score taperedKingPcSq[SQUARE_CT] = {
          S(-187, -225), S(4, -96),    S(48, -38),   S(-23, 28),   S(-5, 13),
          S(-78, -9),    S(76, -19),   S(-30, -160), S(-132, -41), S(-8, -60),
          S(-4, 6),      S(33, -17),   S(11, 41),    S(-5, 61),    S(7, 57),
          S(60, -5),     S(-158, -36), S(46, 30),    S(-20, 28),   S(-6, 28),
          S(-55, 27),    S(-8, 44),    S(72, 37),    S(-31, 66),   S(-42, 18),
          S(-93, 39),    S(-111, 69),  S(-233, 101), S(-146, 71),  S(-132, 85),
          S(-127, 54),   S(-104, 34),  S(-203, 19),  S(-46, 15),   S(-171, 86),
          S(-127, 67),   S(-258, 94),  S(-175, 68),  S(-147, 47),  S(-298, 44),
          S(-51, -33),   S(-74, 24),   S(-123, 68),  S(-135, 50),  S(-206, 58),
          S(-161, 53),   S(-110, 12),  S(-191, 22),  S(32, -82),   S(-104, 21),
          S(-100, 35),   S(-120, 47),  S(-117, 15),  S(-85, 36),   S(-2, -2),
          S(55, -98),    S(-42, -91),  S(57, -87),   S(34, -42),   S(-98, 5),
          S(-10, -59),   S(-38, -40),  S(89, -113),  S(79, -151),
    };
    const Score passedPawnRankBonus[8] = {
          S(0, 0),    S(24, 20),   S(20, -3),  S(21, 46),
          S(-6, 112), S(-14, 184), S(-3, 226), S(0, 0),
    };
    const Score doubledPawnRankBonus[8] = {
          S(-10, -10),  S(5, -68),     S(19, -52),  S(-14, -49),
          S(-26, -168), S(-135, -137), S(-10, -10), S(-10, -10),
    };
    const Score isolatedPawnRankBonus[8] = {
          S(-6, -6),  S(-19, 9),  S(-34, 3),  S(-30, 10),
          S(16, -32), S(-7, -15), S(52, -60), S(-6, -6),
    };
    const Score backwardPawnRankBonus[8] = {
          S(-15, -15), S(-19, -3),  S(-4, -10),  S(-8, -18),
          S(-12, 14),  S(-15, -15), S(-15, -15), S(-15, -15),
    };
    const Score KnightMobilityScore[9] = {
          S(-47, -61), S(-24, 31), S(-19, 44), S(20, 41), S(32, 53),
          S(38, 70),   S(44, 70),  S(65, 56),  S(62, 29),
    };
    const Score BishopMobilityScore[14] = {
          S(-9, -25), S(39, -36), S(46, 43),   S(52, 57),   S(78, 71),
          S(81, 78),  S(91, 96),  S(100, 101), S(100, 108), S(101, 107),
          S(108, 99), S(73, 85),  S(109, 129), S(149, 51),
    };
    const Score RookMobilityScore[15] = {
          S(-68, 17), S(-25, 42), S(-20, 110), S(-8, 111), S(-13, 130),
          S(2, 132),  S(8, 141),  S(8, 157),   S(35, 148), S(37, 164),
          S(61, 158), S(58, 171), S(77, 170),  S(58, 166), S(91, 152),
    };
    const Score QueenMobilityScore[28] = {
          S(-167, -181), S(-21, -6),  S(33, -8),   S(-6, 75),   S(-19, 93),
          S(-8, 87),     S(-2, 109),  S(10, 145),  S(17, 189),  S(35, 183),
          S(40, 206),    S(34, 220),  S(55, 176),  S(60, 158),  S(51, 175),
          S(64, 201),    S(56, 218),  S(41, 209),  S(57, 219),  S(105, 178),
          S(95, 203),    S(92, 126),  S(163, 182), S(151, 153), S(143, 122),
          S(146, 175),   S(172, 204), S(204, 217),
    };
};

#endif // WEIGHTS_H_
