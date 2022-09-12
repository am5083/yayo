#include "eval.hpp"
#include "board.hpp"
#include "movegen.hpp"
#include "util.hpp"

namespace Yayo {

Trace trace;

// clang-format off
template <> int eval<TRACE>(Board &board, moveList &mList) {
    const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
    const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
    const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
    const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
    const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

    const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
    const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
    const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
    const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
    const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

    const int wMaterial =
        (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) + (ROOK_VAL * whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

    const int bMaterial =
        (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) + (ROOK_VAL * blackRookScore) + (QUEEN_VAL * blackQueenScore);

    trace.pawnScore[WHITE] = whitePawnScore;
    trace.pawnScore[BLACK] = blackPawnScore;

    trace.knightScore[WHITE] = whiteKnightScore;
    trace.knightScore[BLACK] = blackKnightScore;

    trace.bishopScore[WHITE] = whiteBishopScore;
    trace.bishopScore[BLACK] = blackBishopScore;

    trace.rookScore[WHITE] = whiteRookScore;
    trace.rookScore[BLACK] = blackRookScore;

    trace.queenScore[WHITE] = whiteQueenScore;
    trace.queenScore[BLACK] = blackQueenScore;

    const auto color = (board.turn == WHITE) ? 1 : -1;

    int eval = TEMPO;
    const auto pcSqEval = pieceSquare<TRACE, WHITE>(board) - pieceSquare<TRACE, BLACK>(board);
    const auto passed = passedPawnScore<TRACE, WHITE>(board) - passedPawnScore<TRACE, BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<TRACE, WHITE>(board) - doubledPawnPenalty<TRACE, BLACK>(board);
    const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
    const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
    const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
    const auto backwardPenalty = backwardPawnScore<TRACE, WHITE>(board) - backwardPawnScore<TRACE, BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
    const auto mobility = mobilityScore<TRACE, WHITE>(board) - mobilityScore<TRACE, BLACK>(board);

    trace.isolatedPawns[WHITE] = whiteIsolatedPawnCount;
    trace.isolatedPawns[BLACK] = blackIsolatedPawnCount;

    eval += (mobility / 2);
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);

    return  eval * color;
}

template <> int eval<NO_TRACE>(Board &board, moveList &mList) {
    const auto whitePawnScore = popcount(board.pieces(PAWN, WHITE));
    const auto whiteKnightScore = popcount(board.pieces(KNIGHT, WHITE));
    const auto whiteBishopScore = popcount(board.pieces(BISHOP, WHITE));
    const auto whiteRookScore = popcount(board.pieces(ROOK, WHITE));
    const auto whiteQueenScore = popcount(board.pieces(QUEEN, WHITE));

    const auto blackPawnScore = popcount(board.pieces(PAWN, BLACK));
    const auto blackKnightScore = popcount(board.pieces(KNIGHT, BLACK));
    const auto blackBishopScore = popcount(board.pieces(BISHOP, BLACK));
    const auto blackRookScore = popcount(board.pieces(ROOK, BLACK));
    const auto blackQueenScore = popcount(board.pieces(QUEEN, BLACK));

    const int wMaterial =
        (PAWN_VAL * whitePawnScore) + (KNIGHT_VAL * whiteKnightScore) + (BISHOP_VAL * whiteBishopScore) + (ROOK_VAL * whiteRookScore) + (QUEEN_VAL * whiteQueenScore);

    const int bMaterial =
        (PAWN_VAL * blackPawnScore) + (KNIGHT_VAL * blackKnightScore) + (BISHOP_VAL * blackBishopScore) + (ROOK_VAL * blackRookScore) + (QUEEN_VAL * blackQueenScore);


    const auto color = (board.turn == WHITE) ? 1 : -1;

    int eval = TEMPO;
    const auto pcSqEval = pieceSquare<NO_TRACE, WHITE>(board) - pieceSquare<NO_TRACE, BLACK>(board);
    const auto passed = passedPawnScore<NO_TRACE, WHITE>(board) - passedPawnScore<NO_TRACE, BLACK>(board);
    const auto doubledPenalty = doubledPawnPenalty<NO_TRACE, WHITE>(board) - doubledPawnPenalty<NO_TRACE, BLACK>(board);
    const auto whiteIsolatedPawnCount = isolatedPawnCount<WHITE>(board);
    const auto blackIsolatedPawnCount = isolatedPawnCount<BLACK>(board);
    const auto isolatedPenalty = ISOLATED_PENALTY * (whiteIsolatedPawnCount - blackIsolatedPawnCount);
    const auto backwardPenalty = backwardPawnScore<NO_TRACE, WHITE>(board) - backwardPawnScore<NO_TRACE, BLACK>(board);
    const auto pawnStructureScore = passed + doubledPenalty + (1.25 * isolatedPenalty) + backwardPenalty;
    const auto mobility = mobilityScore<NO_TRACE, WHITE>(board) - mobilityScore<NO_TRACE, BLACK>(board);

    eval += (mobility / 2);
    eval += (wMaterial - bMaterial);
    eval += (1.2 * pcSqEval);
    eval += (0.3 * pawnStructureScore);

    return  eval * color;
}

void Trace::print() {
    int numTerms = 0;
    std::cout << pawnScore[WHITE] - pawnScore[BLACK] << std::endl;
    numTerms++;
    std::cout << knightScore[WHITE] - knightScore[BLACK] << std::endl;
    numTerms++;
    std::cout << bishopScore[WHITE] - bishopScore[BLACK] << std::endl;
    numTerms++;
    std::cout << rookScore[WHITE] - rookScore[BLACK] << std::endl;
    numTerms++;
    std::cout << queenScore[WHITE] - queenScore[BLACK] << std::endl;
    numTerms++;

    numTerms += 6 * 64;
    std::cout << "PIECE SQUARE: " << std::endl;
    for (int i = 0; i < 64; i++) {
        if (pawnPcSq[WHITE][i] || pawnPcSq[BLACK][i])
            std::cout << pawnPcSq[WHITE][i] - pawnPcSq[BLACK][i] << std::endl;

        else if (knightPcSq[WHITE][i] || knightPcSq[BLACK][i])
            std::cout << knightPcSq[WHITE][i] - knightPcSq[BLACK][i] << std::endl;

        else if (bishopPcSq[WHITE][i] || bishopPcSq[BLACK][i])
            std::cout << bishopPcSq[WHITE][i] - bishopPcSq[BLACK][i] << std::endl;

        else if (rookPcSq[WHITE][i] || rookPcSq[BLACK][i])
            std::cout << rookPcSq[WHITE][i] - rookPcSq[BLACK][i] << std::endl;

        else if (queenPcSq[WHITE][i] || queenPcSq[BLACK][i])
            std::cout << queenPcSq[WHITE][i] - queenPcSq[BLACK][i] << std::endl;

        else if (kingPcSq[WHITE][i] || kingPcSq[BLACK][i])
            std::cout << kingPcSq[WHITE][i] - kingPcSq[BLACK][i] << std::endl;

        else
            std::cout << 0 << std::endl;
    }

    numTerms += 8;
    std::cout << "PASSED PAWN: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << passedPawn[i][WHITE] - passedPawn[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "PASSED PAWN BLOCKED/ATK: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << passedPawnBlockedAttacked[i][WHITE] - passedPawnBlockedAttacked[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "PASSED PAWN BLOCKED/DEF: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << passedPawnBlockedDefended[i][WHITE] - passedPawnBlockedDefended[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "PASSED PAWN BLOCKED: " << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << passedPawnBlocked[i][WHITE] - passedPawnBlocked[i][BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "DOUBLED PAWN" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << doubledPawns[WHITE] - doubledPawns[BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "ISOLATED PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << isolatedPawns[WHITE] - isolatedPawns[BLACK] << std::endl;
    }

    numTerms += 8;
    std::cout << "BACKWARD PAWNS:" << std::endl;
    for (int i = 0; i < 8; i++) {
        std::cout << backwardPawns[WHITE] - backwardPawns[BLACK] << std::endl;
    }

    numTerms += 9;
    std::cout << "knightMobility:" << std::endl;
    for (int i = 0; i < 9; i++) {
        std::cout << knightMobility[WHITE][i] - knightMobility[BLACK][i] << std::endl;
    }

    numTerms += 14;
    std::cout << "bishopMobility:" << std::endl;
    for (int i = 0; i < 14; i++) {
        std::cout << bishopMobility[WHITE][i] - bishopMobility[BLACK][i] << std::endl;
    }

    numTerms += 15;
    std::cout << "rookMobility:" << std::endl;
    for (int i = 0; i < 15; i++) {
        std::cout << rookMobility[WHITE][i] - rookMobility[BLACK][i] << std::endl;
    }

    numTerms += 28;
    std::cout << "queenMobility:" << std::endl;
    for (int i = 0; i < 28; i++) {
        std::cout << queenMobility[WHITE][i] - queenMobility[BLACK][i] << std::endl;
    }

    std::cout << "total number of terms: " << numTerms << std::endl;
}
}
