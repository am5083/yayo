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
#include "thread.hpp"
#include "eval.hpp"
#include <cassert>
#include <stdio.h>
#include <vector>

namespace Yayo {

void Search::startSearch(Info *_info) {
    if (searched)
        searchThread->join();
    searched = 1;
    info = _info;
    nodes = 0;
    stopCount = 0;
    stopFlag = 0;
    info->uciStop = false;
    info->uciQuit = false;
    numRep = 0;

    memset(&historyMoves, 0, sizeof(historyMoves));
    memset(&killerMoves, NO_MOVE, sizeof(killerMoves));
    memset(&killerMates, NO_MOVE, sizeof(killerMates));
    memset(&pvTableLen, NO_MOVE, sizeof(pvTableLen));
    memset(&pvTable, NO_MOVE, sizeof(pvTable));
    memset(&Hist, 0, sizeof(Hist));

    searchThread = std::make_unique<std::thread>(&Search::search, this);
}

const bool Search::checkForStop() const {
    stopCount++;
    stopCount &= 1023;

    if (info->uciStop) {
        return true;
    }

    if (!stopCount) {
        if (info->stopTime <= get_time() && info->timeGiven) {
            stopFlag = 1;
            info->uciStop = true;
        }
    }

    return false;
}

moveList Search::generateMoves() {
    moveList mList = {0};
    generate(_board, &mList);
    return mList;
}

Board Search::getBoard() { return _board; }

void Search::scoreMoves(moveList *mList, unsigned ttMove) {
    for (int i = 0; i < mList->nMoves; i++) {
        unsigned move = mList->moves[i].move;

        if (move == pvTable[_board.ply][0]) {
            mList->moves[i].score = 500000;
            continue;
        } else if (ttMove && move == ttMove) {
            mList->moves[i].score = 200000;
            continue;
        }

        int moveFlag = getCapture(move);

        if (moveFlag >= P_KNIGHT) {
            if (moveFlag >= CP_ROOK)
                mList->moves[i].score = 20000 + 1400 + (moveFlag - 10);
            else if (moveFlag >= CP_KNIGHT)
                mList->moves[i].score = 20000 + 1200 + (moveFlag - 10);
            else
                mList->moves[i].score = 19000 + 75 + moveFlag - 10;
        }

        else if (moveFlag >= CAPTURE && moveFlag < P_KNIGHT) {
            Square fromSq = getFrom(move), toSq = getTo(move);
            Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

            if (getCapture(move) == EP_CAPTURE) {
                toPc = _board.board[toSq ^ 8];
            }

            if (fromPc > toPc) {
                int see = _board.see(toSq, toPc, fromSq, fromPc);

                if (see > 0) {
                    see += 20000;
                }

                mList->moves[i].score = see;
            }
        }

        else if (moveFlag < CAPTURE) {
            if (killerMates[_board.ply][0] == move) {
                mList->moves[i].score = 18000 + 100;
            }

            else if (killerMates[_board.ply][1] == move) {
                mList->moves[i].score = 18000 + 95;
            }

            else if (killerMoves[_board.ply][0] == move) {
                mList->moves[i].score = 16000 + 90;
            }

            else if (killerMoves[_board.ply][1] == move) {
                mList->moves[i].score = 16000 + 80;
            }

            else {
                mList->moves[i].score =
                      historyMoves[_board.turn][getFrom(move)][getTo(move)];
            }
        }
    }
}

int Search::quiescent(int alpha, int beta) {
    int hashFlag = TP_ALPHA;
    int ply = _board.ply;

    // if (_board.checkPcs)
    //     return negaMax(alpha, beta, 1, false, true);

    if (checkForStop()) {
        stopFlag = 1;
        return ABORT_SCORE;
    }

    selDepth = std::max(selDepth, ply);

    tt.prefetch(_board.key);
    bool pvNode = (beta - alpha) < 1;
    pvTableLen[_board.ply] = 0;

    int flag = -1;
    int evalScore = INF;
    int ttScore = 0;
    unsigned tpMove = 0;
    TTHash entry = {0};

    if (tt.probe(_board.key, entry)) {
        ttScore = entry.score(_board.ply);
        tpMove = entry.move();

        evalScore = entry.eval();

        flag = entry.flag();

        if (!pvNode &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= beta) ||
             (flag == TP_ALPHA && ttScore <= alpha))) {
            return ttScore;
        }
    }

    Eval eval(_board);

    if (evalScore == INF) {
        evalScore = eval.eval();
        Hist[ply].eval = evalScore;
    } else {
        Hist[ply].eval = evalScore;

        if (!pvNode &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= evalScore) ||
             (flag == TP_ALPHA && ttScore <= evalScore))) {
            evalScore = ttScore;
        }
    }

    int score = INF, best = evalScore, oldAlpha = alpha;
    unsigned bestMove = 0;

    if (best >= beta)
        return best;

    int queenValue = (eval.mgPhase * MgScore(queenScore) +
                      eval.egPhase * EgScore(queenScore)) /
                     24;

    int deltaMargin = best + 200 + queenValue;
    if (deltaMargin < alpha) {
        return alpha;
    }

    if (alpha < best)
        alpha = best;

    constexpr auto pcVal =
          std::array{0, PAWN_VAL, KNIGHT_VAL, BISHOP_VAL, ROOK_VAL, QUEEN_VAL};

    moveList mList = {0};
    generateCaptures(_board, &mList);
    scoreMoves(&mList, tpMove);

    int standPat = best;

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);

        unsigned move = mList.moves[i].move;
        Square fromSq = getFrom(move), toSq = getTo(move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = _board.board[toSq ^ 8];
        }

        if (mList.moves[i].score < 0)
            continue;

        int dMargin = standPat + 200 + pcVal[getPcType(toPc)];
        if (dMargin < alpha && getCapture(move) < P_KNIGHT)
            continue;

        Hist[ply].move = move;

        nodes++;
        make(_board, mList.moves[i].move);
        score = -quiescent(-beta, -alpha);
        unmake(_board, mList.moves[i].move);

        if (score > best) {
            best = score;
            bestMove = mList.moves[i].move;

            if (score > alpha) {
                hashFlag = TP_EXACT;
                alpha = score;

                if (alpha >= beta) {
                    hashFlag = TP_BETA;
                    break;
                }
            }
        }
    }

    tt.record(_board.key, _board.ply, bestMove, 0, Hist[ply].eval, best, pvNode,
              hashFlag);

    return best;
}

int Search::negaMax(int alpha, int beta, int depth, bool cutNode,
                    bool isExtension) {
    int hashFlag = TP_ALPHA;
    const int ply = _board.ply;
    pvTableLen[ply] = 0;

    tt.prefetch(_board.key);
    if (checkForStop()) {
        stopFlag = 1;
        return ABORT_SCORE;
    }

    bool inCheck = popcount(_board.checkPcs) > 0;
    if (inCheck && !isExtension) {
        depth++;
        isExtension = true;
    }

    if (depth <= 0)
        return quiescent(alpha, beta);

    bool futilityPrune = false;
    bool pvNode = alpha < (beta - 1);

    if (_board.ply > 0) {
        if (_board.halfMoves >= 100 || _board.isDraw() || _board.isTMR())
            return 1 - (nodes & 3);

        alpha = std::max(alpha, -INF + _board.ply);
        beta = std::min(beta, INF - _board.ply);

        if (alpha >= beta) {
            if (killerMates[ply][0] != pvTable[ply][0]) {
                killerMates[ply][1] = killerMates[ply][0];
                killerMates[ply][0] = pvTable[ply][0];
            }

            return alpha;
        }
    }

    int evalScore = INF;
    bool ttHit = false;
    int ttScore = 0;
    unsigned ttMove = 0;
    int flag = -1;
    TTHash entry = {0};
    if (tt.probe(_board.key, entry)) {
        ttHit = true;
        ttScore = entry.score(_board.ply);
        ttMove = entry.move();
        flag = entry.flag();
        evalScore = entry.eval();
        if (!pvNode && entry.depth() >= depth) {
            if ((flag == TP_EXACT || (flag == TP_BETA && ttScore >= beta) ||
                 (flag == TP_ALPHA && ttScore <= alpha))) {
                return ttScore;
            }
        }
    }

    int futilityMargin[] = {0, 100, 300, 700};

    Eval eval(_board);
    int best = -INF;
    unsigned move = 0;
    int score = 0;

    if (evalScore == INF) {
        if (ply >= 1 && !Hist[ply - 1].move) {
            evalScore = Hist[ply - 1].eval;
            Hist[ply].eval = evalScore;
        } else {
            evalScore = eval.eval();
            Hist[ply].eval = evalScore;
        }
    } else {
        Hist[ply].eval = evalScore;
        // try tt score

        if (!pvNode &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= evalScore) ||
             (flag == TP_ALPHA && ttScore <= evalScore))) {
            evalScore = ttScore;
        }
    }

    bool improving =
          (!inCheck && ply >= 2 && Hist[ply].eval > Hist[ply - 2].eval);

    // static NMP
    int evalMargin = evalScore - (75 - 28 * improving) * depth;
    if (!pvNode && !_board.checkPcs && depth <= 8 && evalMargin > beta &&
        std::abs(alpha) < CHECKMATE) {
        return evalMargin;
    }

    if (depth > 1 && !_board.checkPcs && !pvNode && Hist[ply - 1].move &&
        evalScore >= beta &&
        (_board.pieces(_board.turn) ^ _board.pieces(PAWN, _board.turn) ^
         _board.pieces(KING, _board.turn)) &&
        (!ttHit || flag == TP_BETA || (ttScore >= beta && ttScore != 0))) {
        int R = 4 + depth / 6 +
                std::min(3, (evalScore - beta) / (135 - 45 * improving)) +
                improving;

        Hist[ply].move = NO_MOVE;
        makeNullMove(_board);
        score = -negaMax(-beta, -beta + 1, depth - R, !cutNode, isExtension);
        unmakeNullMove(_board);

        if (score >= beta) {
            if (std::abs(score) > CHECKMATE) {
                return beta;
            } else {
                return score;
            }
        }
    }

    // if (!pvNode && depth <= 3 &&
    //     (evalScore <= (alpha - 119 - 182 * (depth - 1)))) {
    //     return quiescent(alpha, beta);
    // }

    moveList mList = {{{0}}};
    generate(_board, &mList);
    scoreMoves(&mList, ttMove);

    if (depth <= 3 && !pvNode && evalScore + futilityMargin[depth] <= alpha &&
        std::abs(alpha) < CHECKMATE && mList.nMoves > 1)
        futilityPrune = true;

    bool rootNode = (_board.ply == 0);
    unsigned bestMove = move;
    int movesSearched = 0;
    int skip = 0;

    if (!ttHit && depth >= 4)
        depth--;

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const unsigned curr_move = mList.moves[i].move;
        bool inCheck = _board.checkPcs;
        bool isQuiet = (getCapture(curr_move) < CAPTURE);

        Square fromSq = getFrom(curr_move), toSq = getTo(curr_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = _board.board[toSq ^ 8];
        }

        if (_board.ply > 0 && best > -CHECKMATE) {
            if (getCapture(curr_move) < CAPTURE) {
                if (skip)
                    continue;

                int reducedDepth = std::max(
                      0,
                      std::min(
                            depth,
                            depth -
                                  int(lmrDepthReduction[std::min(
                                        63, depth)][std::min(63,
                                                             movesSearched)])));
                if (reducedDepth <= 8 && !inCheck &&
                    evalScore + 125 + 100 * reducedDepth +
                                historyMoves[_board.turn][fromSq][toSq] / 5 <
                          alpha) {
                    skip = true;
                }

                if (depth <= 8 && !_board.checkPcs &&
                    _board.see(toSq, toPc, fromSq, fromPc) < -80 * depth)
                    continue;
            }
        }

        make(_board, mList.moves[i].move);

        // if (skip && !_board.checkPcs && movesSearched >= 1) {
        //     unmake(_board, curr_move);
        //     continue;
        // }

        // if (!pvNode && !_board.checkPcs && movesSearched >= 1 &&
        //     getCapture(curr_move) < CAPTURE && depth <= 8 &&
        //     see < -50 * depth) {
        //     unmake(_board, curr_move);
        //     continue;
        // }

        // if (!pvNode && !inCheck && depth >= 3 && movesSearched >= 4 &&
        //     (mList.moves[i].score < 0 || getCapture(curr_move) < CAPTURE) &&
        //     !_board.checkPcs) {
        //     unmake(_board, mList.moves[i].move);
        //     continue;
        // }
        //

        if (futilityPrune && getCapture(curr_move) < CAPTURE &&
            !_board.checkPcs) {
            unmake(_board, mList.moves[i].move);
            continue;
        }

        int R = 1;
        if (movesSearched >= (1 + (2 * rootNode)) && depth >= 3 && isQuiet) {
            R = lmrDepthReduction[std::min(63, depth)]
                                 [std::min(63, movesSearched)];
            R += !pvNode;
            R += !improving;
            R += cutNode;

            R -= 2 * (mList.moves[i].score > 19500);
            int mHist = historyMoves[_board.turn][fromSq][toSq] / 125;
            R -= std::min(2, mHist) * isQuiet;

            R = std::min(depth - 1, std::max(1, R));
        }

        Hist[ply].move = curr_move;

        nodes++;
        movesSearched++;
        int score = -INF;

        if (R != 1) {
            score = -negaMax(-alpha - 1, -alpha, depth - R, true);
        }

        if ((R != 1 && score > alpha) ||
            (R == 1 && !(pvNode && movesSearched == 1))) {
            score = -negaMax(-alpha - 1, -alpha, depth - 1, !cutNode);
        }

        if (pvNode && (movesSearched == 1 || score > alpha)) {
            score = -negaMax(-beta, -alpha, depth - 1, false);
        }

        unmake(_board, mList.moves[i].move);

        if (score > best) {
            best = score;
            bestMove = curr_move;

            if (score > alpha) {
                updatePv(ply, curr_move);

                alpha = score;
                hashFlag = TP_EXACT;

                if (alpha >= beta) {
                    hashFlag = TP_BETA;
                    break;
                }
            }
        }
    }

    tt.prefetch(_board.key);

    if (mList.nMoves == 0) {
        if (_board.checkPcs) {
            return -INF + ply;
        }

        return 0;
    }

    if (best >= beta && getCapture(bestMove) < CAPTURE) {
        if (killerMoves[ply][0] != bestMove) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = bestMove;
        }

        historyMoves[_board.turn][getFrom(bestMove)][getTo(bestMove)] +=
              depth * depth;
    }

    tt.record(_board.key, _board.ply, bestMove, depth, Hist[ply].eval, best,
              pvNode, hashFlag);

    return best;
}

int Search::search() {
    abortDepth = -1;
    int depth = info->depth;
    int num = 2;

    int alpha = -INF, beta = INF;
    int score = 0, prevScore = -INF;
    unsigned bestMove = 0;

    double totalTime = 0;
    for (int j = 1; j <= depth; j++) {
        double start = get_time();
        int window = 10;

        if (j >= 7) {
            alpha = std::max(-INF, prevScore - window);
            beta = std::min(INF, prevScore + window);
        } else {
            alpha = -INF;
            beta = INF;
        }

        num = 0;
        int numFailed = 0;
        int aspirationDepth = j;

        while (true) {

            if (checkForStop()) {
                abortDepth = aspirationDepth;
                break;
            }

            num++;
            aspirationDepth = std::max(1, aspirationDepth);
            selDepth = 0;
            score = negaMax(alpha, beta, aspirationDepth, false);

            if (score <= alpha) {
                numFailed = 0;

                beta = (alpha + beta) / 2;
                alpha = std::max(-INF, alpha - window);
                aspirationDepth = j;
            } else if (beta <= score) {
                numFailed++;

                beta = std::min(INF, beta + window);

                if (numFailed > 1) {
                    numFailed = 0;
                    aspirationDepth = j;
                    beta = INF;
                    alpha = -INF;
                    continue;
                }

                if (std::abs(score) < CHECKMATE)
                    aspirationDepth--;

                if (pvTableLen[0] && !bestMove)
                    bestMove = pvTable[0][0];
            } else {
                if (pvTableLen[0])
                    bestMove = pvTable[0][0];

                double end = ((get_time() - start) + 1) / 1000.0;
                totalTime += end;
                long double nps = (nodes / (totalTime));

                std::cout << std::fixed << "info depth " << aspirationDepth;
                std::cout << " seldepth " << selDepth;
                std::cout << " hashfull " << tt.percentFull();
                std::cout << " score";

                if (std::abs(score) > (INF - MAX_PLY)) {
                    int tscore = 0;
                    if (score < 0)
                        tscore = -1;
                    else
                        tscore = 1;
                    std::cout << " mate "
                              << tscore * ((INF - std::abs(score)) / 2);
                } else {
                    std::cout << " cp " << score;

                    if (score >= beta)
                        std::cout << "lowerbound";
                    if (score <= alpha)
                        std::cout << "upperbound";
                }

                std::cout << " nodes " << nodes;
                std::cout << " nps " << int(nps) << " time "
                          << int(totalTime * 1000) << " pv ";
                printPv();
                std::cout << std::endl;

                break;
            }

            window += window / 3;
        }

        prevScore = score;
    }

    if (!bestMove) {
        moveList mList = {{0}};
        generate(_board, &mList);
        mList.swapBest(0);
        bestMove = mList.moves[0].move;
    }

    std::cout << "bestmove ";
    print_move(bestMove);
    std::cout << std::endl;
    bench_nodes += nodes;

    return 0;
}

void Search::updatePv(int ply, unsigned move) {
    pvTable[ply][0] = move;

    for (int i = 0; i < pvTableLen[ply + 1]; i++)
        pvTable[ply][i + 1] = pvTable[ply + 1][i];

    pvTableLen[ply] = 1 + pvTableLen[ply + 1];
}

void Search::printPv() {
    for (int i = 0; i < pvTableLen[0]; i++) {
        print_move(pvTable[0][i]);
        std::cout << " ";
    }
}

std::vector<int> Search::getPv() {
    std::vector<int> x;

    for (int i = 0; i < pvTableLen[0]; i++) {
        x.push_back(pvTable[0][i]);
    }

    return x;
}

void Search::isReady() {
    if (searched) {
        info->uciStop = true;
        searchThread->join();
        searched = 0;
    }

    std::cout << "readyok" << std::endl;
}

void Search::wait() {
    if (searched) {
        searchThread->join();
        searched = 0;
    }
}

void Search::_make(std::uint16_t move) {
    make(_board, move);
    _board.ply = 0;
}

void Search::_setFen(std::string fen) { _board.setFen(std::move(fen)); }

void Search::printBoard() const { _board.print(); }

void Search::stopSearch() {
    if (info == nullptr)
        return;
    if (info->timeGiven) {
        info->stopTime = 0;
    } else {
        info->uciStop = true;
    }
}

void Search::joinThread() {
    if (searched) {
        searchThread->join();
    }
}

void Search::clearTT(int size) { tt.init(size); }

} // namespace Yayo
