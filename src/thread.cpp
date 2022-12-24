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
#include "movegen.hpp"
#include "tt.hpp"
#include "util.hpp"
#include <string>
#include <thread>
#include <vector>

namespace Yayo {

void Search::startSearch(Info *_info) {
    if (searched)
        searchThread->join();
    searched = true;
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

bool Search::checkForStop() const {
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
    generate(board, &mList);
    return mList;
}

Board Search::getBoard() { return board; }

void Search::scoreMoves(moveList *mList, unsigned ttMove) {
    for (int i = 0; i < mList->nMoves; i++) {
        unsigned move = mList->moves[i].move;

        if (move == ttMove) {
            mList->moves[i].score = 9000000;
            continue;
        }

        int moveFlag = getCapture(move);

        if (moveFlag >= CAPTURE && moveFlag < P_KNIGHT) {
            Square fromSq = getFrom(move), toSq = getTo(move);
            Piece fromPc = board.board[fromSq], toPc = board.board[toSq];

            if (getCapture(move) == EP_CAPTURE) {
                toPc = board.board[toSq ^ 8];
            }

            if (fromPc > toPc || mList->moves[i].score <= 0) {
                int see = board.see(toSq, toPc, fromSq, fromPc);
                if (see > 0)
                    see += 100000;
                else
                    see += 71395;
                mList->moves[i].score = see;
            }
        }

        else if (moveFlag < CAPTURE) {
            if (killerMates[board.ply][0] == move) {
                mList->moves[i].score = 72000 + 100;
            }

            else if (killerMates[board.ply][1] == move) {
                mList->moves[i].score = 72000 + 95;
            }

            else if (killerMoves[board.ply][0] == move) {
                mList->moves[i].score = 71000 + 90;
            }

            else if (killerMoves[board.ply][1] == move) {
                mList->moves[i].score = 66000 + 80;
            }

            else {
                Square fromSq = getFrom(move), toSq = getTo(move);
                Piece fromPc = board.board[fromSq], toPc = board.board[toSq];
                int historyScore =
                      historyMoves[fromPc][getFrom(move)][getTo(move)];
                mList->moves[i].score = historyScore;
            }
        }
    }
}

int Search::quiescent(int alpha, int beta) {
    int hashFlag = TP_ALPHA;
    int ply = board.ply;
    pvTableLen[ply] = 0;

    if (checkForStop()) {
        stopFlag = 1;
    }

    if (board.halfMoves >= 100 || board.isDraw() || board.isTMR()) {
        return 1 - (nodes & 2);
    }

    selDepth = std::max(selDepth, ply);

    tt.prefetch(board.key);
    bool pvNode = (beta - alpha) < 1;

    bool nullMove = (board.ply >= 1 && !Hist[ply - 1].move);
    bool ttHit = false;
    int flag = -1;
    int evalScore = INF;
    int ttScore = 0;
    unsigned tpMove = 0;
    TTHash entry = {0};

    if (!nullMove && tt.probe(board.key, entry)) {
        ttHit = true;
        ttScore = entry.score(board.ply);
        tpMove = entry.move();
        evalScore = entry.eval();
        flag = entry.flag();
    }

    Eval eval(board);

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

    int score = INF, best = evalScore;
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
    generateCaptures(board, &mList);
    scoreMoves(&mList, tpMove);

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);

        unsigned move = mList.moves[i].move;
        Hist[ply].move = move;

        nodes++;
        make(board, move);
        score = -quiescent(-beta, -alpha);
        unmake(board, move);

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

    tt.record(board.key, board.ply, bestMove, 0, Hist[ply].eval, best, pvNode,
              hashFlag);

    return best;
}

int Search::negaMax(int alpha, int beta, int depth, bool cutNode,
                    bool isExtension) {
    int hashFlag = TP_ALPHA;
    const int ply = board.ply;

    tt.prefetch(board.key);

    if (checkForStop()) {
        stopFlag = 1;
    }

    bool inCheck = popcount(board.checkPcs) > 0;
    if (inCheck && !isExtension) {
        depth++;
        isExtension = true;
    }

    if (depth <= 0)
        return quiescent(alpha, beta);

    pvTableLen[ply] = 0;
    bool futilityPrune = false;
    bool pvNode = alpha < (beta - 1);

    if (board.ply > 0) {
        if (board.halfMoves >= 100 || board.isDraw() || board.isTMR())
            return 1 - (nodes & 2);

        alpha = std::max(alpha, -INF + board.ply);
        beta = std::min(beta, INF - board.ply);

        if (alpha >= beta) {
            if (killerMates[ply][0] != pvTable[ply][0]) {
                killerMates[ply][1] = killerMates[ply][0];
                killerMates[ply][0] = pvTable[ply][0];
            }

            return alpha;
        }
    }

    bool nullMove = (board.ply >= 1 && !Hist[ply - 1].move);
    int evalScore = INF;
    bool ttHit = false;
    int ttScore = 0;
    unsigned ttMove = 0;
    int flag = -1;
    TTHash entry = {0};

    if (!nullMove && tt.probe(board.key, entry)) {
        ttHit = true;
        ttScore = entry.score(board.ply);
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
    Eval eval(board);
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
    if (!pvNode && !board.checkPcs && depth <= 8 && evalMargin > beta &&
        std::abs(alpha) < CHECKMATE) {
        return evalMargin;
    }

    if (depth > 2 && !board.checkPcs && !pvNode && Hist[ply - 1].move &&
        evalScore >= beta &&
        (board.pieces(board.turn) ^ board.pieces(PAWN, board.turn) ^
         board.pieces(KING, board.turn)) &&
        (!ttHit || flag == TP_BETA || ttScore >= beta)) {

        int R =
              4 + depth / 6 + std::min(3, (evalScore - beta) / 100) + improving;

        Hist[ply].move = NO_MOVE;
        makeNullMove(board);
        score = -negaMax(-beta, -beta + 1, depth - R, !cutNode);
        unmakeNullMove(board);

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

    // if (depth <= 3 && (evalScore + 130 + 150 * depth <= alpha)) {
    //     return quiescent(alpha, beta);
    // }

move_loop:

    moveList mList = {{{0}}};
    generate(board, &mList);
    scoreMoves(&mList, ttMove);

    bool rootNode = (board.ply == 0);
    unsigned bestMove = move;
    int movesSearched = 0;
    int numQuiets = 0;
    bool skip = false;

    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const unsigned move = mList.moves[i].move;
        bool inCheck = board.checkPcs;
        bool isQuiet = (getCapture(move) < CAPTURE);
        bool isCapture =
              (getCapture(move) == CAPTURE || getCapture(move) == EP_CAPTURE ||
               getCapture(move) >= CP_KNIGHT);

        if (skip && isQuiet)
            continue;

        Square fromSq = getFrom(move), toSq = getTo(move);
        Piece fromPc = board.board[fromSq], toPc = board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = board.board[toSq ^ 8];
        }

        if (board.ply > 0 && best > -CHECKMATE) {
            if (isQuiet) {
                int reducedDepth =
                      depth - int(lmrDepthReduction[std::min(
                                    63, depth)][std::min(63, movesSearched)]);

                if (reducedDepth <= 8 && !inCheck &&
                    evalScore + 125 + 100 * reducedDepth < alpha) {
                    skip = true;
                }

                // if (depth <= 8 && !inCheck && !pvNode &&
                //     numQuiets > lmpThresholds[improving][depth]) {
                //     skip = true;
                // }

                if (depth <= 8 &&
                    board.see(toSq, toPc, fromSq, fromPc) < -60 * depth)
                    continue;
            } else if (isCapture) {
                if (depth <= 8 &&
                    board.see(toSq, toPc, fromSq, fromPc) < -80 * depth)
                    continue;
            }
        }

        Hist[ply].move = move;

        make(board, mList.moves[i].move);

        int R = 1;
        if (movesSearched > (1 + (2 * rootNode)) && depth >= 3 && isQuiet) {
            R = lmrDepthReduction[std::min(63, depth)]
                                 [std::min(63, movesSearched)];
            R += !pvNode;
            R += !improving;
            R += cutNode;

            // int mHist = historyMoves[_board.turn][fromSq][toSq] / 1024;
            // R -= std::min(2, mHist);

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

        unmake(board, mList.moves[i].move);

        if (stopFlag)
            return 0;

        if (score > best) {
            best = score;
            bestMove = move;

            if (score > alpha) {
                updatePv(ply, move);

                alpha = score;
                hashFlag = TP_EXACT;

                if (alpha >= beta) {
                    hashFlag = TP_BETA;
                    break;
                }
            }
        }
    }

    tt.prefetch(board.key);

    if (mList.nMoves == 0) {
        if (board.checkPcs) {
            return -INF + ply;
        }

        return 0;
    }

    if (best >= beta && getCapture(bestMove) < CAPTURE) {
        Square fromSq = getFrom(bestMove);
        Square toSq = getTo(bestMove);
        Piece fromPc = board.board[fromSq];
        int *hm = &historyMoves[fromPc][fromSq][toSq];

        if (killerMoves[ply][0] != bestMove) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = bestMove;
        }

        *hm += depth * depth;
    }

    tt.record(board.key, board.ply, bestMove, depth, Hist[ply].eval, best,
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
        generate(board, &mList);
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
    make(board, move);
    board.ply = 0;
}

void Search::_setFen(std::string fen) { board.setFen(fen); }

void Search::printBoard() const { board.print(); }

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
