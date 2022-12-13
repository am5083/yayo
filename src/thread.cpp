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
    memset(&pvTableLen, NO_MOVE, sizeof(pvTableLen));
    memset(&pvTable, NO_MOVE, sizeof(pvTable));
    memset(&Hist, 0, sizeof(Hist));

    searchThread = std::make_unique<std::thread>(&Search::search, this);
}

const bool Search::checkForStop() const {
    stopCount++;
    stopCount &= 1023;

    if (info->uciStop || stopFlag || info->uciQuit) {
        stopFlag = true;
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

        if (move == ttMove) {
            mList->moves[i].score = 9000000;
            continue;
        }

        // if (move == pvTable[_board.ply - 1][0]) {
        //             mList->moves[i].score = 500000;
        //             continue;
        //         }

        int moveFlag = getCapture(move);

        if (moveFlag >= P_KNIGHT) {
            if (moveFlag >= CP_ROOK)
                mList->moves[i].score = 200000 + 1400 + (moveFlag - 10);
            else if (moveFlag >= CP_KNIGHT)
                mList->moves[i].score = 20425 + (moveFlag - 10);
            else if (moveFlag < CP_KNIGHT && moveFlag >= P_ROOK)
                mList->moves[i].score = 20195 + 75 + moveFlag - 10;
            else
                mList->moves[i].score = 19800 + 75 + moveFlag - 10;
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
                int historyScore =
                      historyMoves[_board.turn][getFrom(move)][getTo(move)];
                mList->moves[i].score =
                      (historyScore <= 16000) ? historyScore : historyScore;
            }
        }
    }
}

int Search::quiescent(int alpha, int beta) {
    int hashFlag = TP_ALPHA;
    int ply = _board.ply;
    pvTableLen[ply] = 0;

    if (checkForStop()) {
        stopFlag = 1;
    }

    if (_board.halfMoves >= 100 || _board.isDraw() || _board.isTMR()) {
        return 1 - (nodes & 2);
    }

    selDepth = std::max(selDepth, ply);

    tt.prefetch(_board.key);
    bool pvNode = (beta - alpha) < 1;

    bool nullMove = (_board.ply >= 1 && !Hist[ply - 1].move) ? true : false;
    bool ttHit = false;
    int flag = -1;
    int evalScore = INF;
    int ttScore = 0;
    unsigned tpMove = 0;
    TTHash entry = {0};

    if (!nullMove && tt.probe(_board.key, entry)) {
        ttHit = true;
        ttScore = entry.score(_board.ply);
        tpMove = entry.move();
        evalScore = entry.eval();
        flag = entry.flag();
    }

    Eval eval(_board);

    if (evalScore == INF) {
        if (ply >= 1 && !Hist[ply - 1].move) {
            evalScore = -Hist[ply - 1].eval + 2 * TEMPO;
            Hist[ply].eval = evalScore;
        } else {
            evalScore = eval.eval();
            Hist[ply].eval = evalScore;
        }
    } else {
        Hist[ply].eval = evalScore;

        if (ttHit &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= evalScore) ||
             (flag == TP_ALPHA && ttScore <= evalScore))) {
            evalScore = ttScore;
        }
    }

    int score = INF, best = evalScore, oldAlpha = alpha;
    unsigned bestMove = 0;

    alpha = std::max(best, alpha);

    if (best >= beta)
        return best;

    if (!pvNode && (flag == TP_EXACT || (flag == TP_BETA && ttScore >= beta) ||
                    (flag == TP_ALPHA && ttScore <= alpha))) {
        return ttScore;
    }

    int deltaMargin = best + 200 + QUEEN_VAL;
    if (deltaMargin < alpha) {
        return alpha;
    }

    moveList mList = {0};
    generateCaptures(_board, &mList);
    scoreMoves(&mList, tpMove);

    int standPat = best;

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);

        unsigned move = mList.moves[i].move;
        Hist[ply].move = move;

        nodes++;
        make(_board, move);
        score = -quiescent(-beta, -alpha);
        unmake(_board, move);

        if (stopFlag)
            return 0;

        if (score > best) {
            best = score;
            bestMove = move;

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

    tt.prefetch(_board.key);

    if (checkForStop()) {
        stopFlag = 1;
    }

    bool inCheck = popcount(_board.checkPcs) > 0;
    if (inCheck && !isExtension) {
        depth++;
        isExtension = true;
    }

    if (depth <= 0)
        return quiescent(alpha, beta);

    pvTableLen[ply] = 0;
    bool futilityPrune = false;
    bool pvNode = alpha < (beta - 1);

    if (_board.ply > 0) {
        if (_board.halfMoves >= 100 || _board.isDraw() || _board.isTMR())
            return 1 - (nodes & 2);

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

    bool nullMove = (_board.ply >= 1 && !Hist[ply - 1].move) ? true : false;
    int evalScore = INF;
    bool ttHit = false;
    int ttScore = 0;
    unsigned ttMove = 0;
    int flag = -1;
    TTHash entry = {0};

    if (!nullMove && tt.probe(_board.key, entry)) {
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

    killerMoves[ply + 1][0] = 0;
    killerMoves[ply + 1][1] = 0;
    killerMates[ply + 1][0] = 0;
    killerMates[ply + 1][1] = 0;

    int evalMargin = -INF;
    bool improving = false;
    Eval eval(_board);
    int best = -INF;
    unsigned move = 0;
    int score = 0;

    if (!ttHit && depth >= 4)
        depth--;

    if (inCheck) {
        evalScore = INF;
        Hist[ply].eval = INF;
        improving = false;
        goto move_loop;
    }

    if (evalScore == INF) {
        if (ply >= 1 && !Hist[ply - 1].move) {
            evalScore = -Hist[ply - 1].eval + 2 * TEMPO;
            Hist[ply].eval = evalScore;
        } else {
            evalScore = eval.eval();
            Hist[ply].eval = evalScore;
        }
    } else {
        Hist[ply].eval = evalScore;
        // try tt score

        if (ttHit &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= evalScore) ||
             (flag == TP_ALPHA && ttScore <= evalScore))) {
            evalScore = ttScore;
        }
    }

    improving = (!inCheck && ply >= 2 && Hist[ply].eval > Hist[ply - 2].eval);

    // static NMP
    evalMargin = evalScore - (75 - 28 * improving) * depth;
    if (!pvNode && !_board.checkPcs && depth <= 8 && evalMargin > beta &&
        std::abs(alpha) < CHECKMATE) {
        return evalMargin;
    }

    if (depth > 2 && !_board.checkPcs && !pvNode && Hist[ply - 1].move &&
        evalScore >= beta &&
        (_board.pieces(_board.turn) ^ _board.pieces(PAWN, _board.turn) ^
         _board.pieces(KING, _board.turn)) &&
        (!ttHit || flag == TP_BETA || ttScore >= beta)) {

        int R =
              4 + depth / 6 + std::min(3, (evalScore - beta) / 100) + improving;

        Hist[ply].move = NO_MOVE;
        makeNullMove(_board);
        score = -negaMax(-beta, -beta + 1, depth - R, !cutNode);
        unmakeNullMove(_board);

        if (stopFlag)
            return 0;

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

move_loop:

    moveList mList = {{{0}}};
    generate(_board, &mList);
    scoreMoves(&mList, ttMove);

    bool rootNode = (_board.ply == 0);
    unsigned bestMove = move;
    int movesSearched = 0;
    int numQuiets = 0;
    int skip = 0;

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const unsigned curr_move = mList.moves[i].move;
        bool inCheck = _board.checkPcs;
        bool isQuiet = (getCapture(curr_move) < CAPTURE);
        bool isCapture = (getCapture(curr_move) == CAPTURE ||
                          getCapture(curr_move) == EP_CAPTURE ||
                          getCapture(curr_move) >= CP_KNIGHT);

        if (skip && isQuiet)
            continue;

        Square fromSq = getFrom(curr_move), toSq = getTo(curr_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = _board.board[toSq ^ 8];
        }

        if (_board.ply > 0 && best > -CHECKMATE) {
            if (isQuiet) {
                int reducedDepth =
                      depth - int(lmrDepthReduction[std::min(
                                    63, depth)][std::min(63, movesSearched)]);

                if (reducedDepth <= 8 && !inCheck &&
                    evalScore + 125 + 100 * reducedDepth < alpha) {
                    skip = true;
                }

                if (reducedDepth <= 8 &&
                    numQuiets > lmpThresholds[improving][reducedDepth]) {
                    skip = true;
                }

                if (depth <= 8 &&
                    _board.see(toSq, toPc, fromSq, fromPc) < -60 * depth)
                    continue;
            } else if (isCapture) {
                if (depth <= 8 &&
                    _board.see(toSq, toPc, fromSq, fromPc) < -80 * depth)
                    continue;
            }
        }

        Hist[ply].move = curr_move;

        make(_board, mList.moves[i].move);

        int R = 1;
        if (movesSearched > (1 + (2 * rootNode)) && depth >= 3 && isQuiet) {
            R = lmrDepthReduction[std::min(63, depth)]
                                 [std::min(63, movesSearched)];
            R += !pvNode;
            R += !improving;
            R += cutNode;

            int mHist = historyMoves[_board.turn][fromSq][toSq] / 1024;
            R -= std::min(2, mHist);

            R = std::min(depth - 1, std::max(1, R));
        }

        if (isQuiet) {
            numQuiets++;
        }

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

        if (stopFlag)
            return 0;

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
    int alpha = -INF, beta = INF;
    int score = 0;
    unsigned bestMove = 0;

    double totalTime = 0;
    int depth = info->depth;

    int alphaWindow = -12;
    int betaWindow = 12;
    int numFailed = 0;
    int failDepth = -1;
    for (int i = 1; i <= depth; i++) {
        double start = get_time();

        score = negaMax(alpha, beta, i, false);

        if (score <= alpha) {
            if (numFailed > 5) {
                alpha = -INF;
            }

            beta = (alpha + beta) / 2;
            alphaWindow *= 1.45;
            alpha += alphaWindow + 1;
            numFailed++;
            i--;
            continue;
        }

        if (score >= beta) {
            if (numFailed > 5) {
                beta = INF;
            }

            betaWindow *= 1.45;
            beta += betaWindow + 1;
            numFailed++;
            i--;
            continue;
        }

        if (i >= 3) {
            alpha = score + alphaWindow;
            beta = score + betaWindow;
        }

        if (info->uciStop) {
            break;
        }

        if (pvTableLen[0])
            bestMove = pvTable[0][0];

        double end = ((get_time() - start) + 1) / 1000.0;
        totalTime += end;
        long double nps = (nodes / (totalTime));

        std::cout << std::fixed << "info depth " << i;
        std::cout << " seldepth " << selDepth;
        std::cout << " hashfull " << tt.percentFull();
        std::cout << " score";

        if (std::abs(score) > (INF - MAX_PLY)) {
            int tscore = 0;
            if (score < 0)
                tscore = -1;
            else
                tscore = 1;
            std::cout << " mate " << tscore * ((INF - std::abs(score)) / 2);
        } else {
            std::cout << " cp " << score;

            if (score >= beta)
                std::cout << "lowerbound";
            if (score <= alpha)
                std::cout << "upperbound";
        }

        std::cout << " nodes " << nodes;
        std::cout << " nps " << int(nps) << " time " << int(totalTime * 1000)
                  << " pv ";
        printPv();
        std::cout << std::endl;
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
