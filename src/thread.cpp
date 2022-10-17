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
#include <vector>

namespace Yayo {

void Search::startSearch(Info *_info) {
    if (searched)
        searchThread->join();
    searched = 1;
    info = _info;
    nodes = 0;
    info->uciStop = false;
    info->uciQuit = false;
    numRep = 0;
    for (int i = 0; i < MAX_PLY + 6; i++) {
        killerMoves[i][0] = NO_MOVE;
        killerMoves[i][1] = NO_MOVE;

        for (int j = 0; j < MAX_PLY + 6; j++) {

            if (i < 64 && j < 64) {
                historyMoves[0][i][j] = 0;
                historyMoves[1][i][j] = 0;
            }
        }
    }

    searchThread = std::make_unique<std::thread>(&Search::search, this);
}

const bool Search::checkForStop() const {
    if (info->stopTime <= get_time() && info->timeGiven) {
        return true;
    }

    if (info->uciStop) {
        return true;
    }

    return false;
}

moveList Search::generateMoves() {
    moveList mList = {0};
    generate(_board, &mList);
    return mList;
}

Board Search::getBoard() { return _board; }

void Search::scoreMoves(moveList *mList, int ttMove) {
    for (int i = 0; i < mList->nMoves; i++) {
        int move = mList->moves[i].move;

        if (move == pvTable[0][_board.ply]) {
            mList->moves[i].score = 10000000 * 2;
            continue;
        } else if (ttMove && move == ttMove) {
            mList->moves[i].score = INF * 2;
            continue;
        }

        if (int(getCapture(move)) < int(CAPTURE)) {
            if (killerMates[_board.ply][0] == move) {
                mList->moves[i].score = 100000;
            }

            else if (killerMates[_board.ply][1] == move) {
                mList->moves[i].score = 95000;
            }

            else if (killerMoves[_board.ply][0] == move) {
                mList->moves[i].score = 90000;
            }

            else if (killerMoves[_board.ply][1] == move) {
                mList->moves[i].score = 80000;
            }

            else {
                mList->moves[i].score =
                      historyMoves[_board.turn][getFrom(move)][getTo(move)];
            }
        }
    }
}

int Search::quiescent(int alpha, int beta, bool probe) {
    int hashFlag = TP_ALPHA;
    int ply = _board.ply;

    pvTableLen[ply] = 0;

    if (_board.halfMoves >= 100 || _board.isDraw())
        return 1 - (nodes & 2);

    if (_board.isTMR()) {
        return 1 - (nodes & 2);
    }

    if (checkForStop())
        return ABORT_SCORE;

    selDepth = std::max(selDepth, ply);
    nodes++;

    tt.prefetch(_board.key);
    bool pvNode = (beta - alpha) < 1;
    Eval eval(_board);
    int score = 0, best = eval.eval(), oldAlpha = alpha;
    int bestMove = 0;

    int ttScore = 0, tpMove = 0;
    TTHash entry = {0};
    if (probe && tt.probe(_board.key, entry)) {
        ttScore = entry.score(_board.ply);
        tpMove = entry.move();
        int flag = entry.flag();

        if (!pvNode && flag == TP_EXACT ||
            (flag == TP_BETA && ttScore >= beta) ||
            (flag == TP_ALPHA && ttScore <= alpha)) {
            return ttScore;
        }
    }

    if (best >= beta)
        return best;

    if (alpha < best)
        alpha = best;

    moveList mList = {0};
    generate(_board, &mList);

    for (int i = 0; i < mList.nMoves; i++) {
        if (tpMove && mList.moves[i].move == tpMove) {
            mList.moves[i].score = INF;
            continue;
        }

        if (mList.moves[i].score == 0) {
            continue;
        }

        const int c_move = mList.moves[i].move;

        Square fromSq = getFrom(c_move), toSq = getTo(c_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (!_board.checkPcs &&
            (best + _board.see(toSq, toPc, fromSq, fromPc) + 200) <= alpha) {
            mList.moves[i].score = -2000;
            continue;
        }

        if (getPcType(fromPc) >= getPcType(toPc)) {
            int see = _board.see(toSq, toPc, fromSq, fromPc);
            mList.moves[i].score = see;
        }
    }

    for (int i = 0; i < mList.nMoves; i++) {
        if (mList.moves[i].score <= 0)
            continue;

        mList.swapBest(i);

        make(_board, mList.moves[i].move);
        score = -quiescent(-beta, -alpha);
        unmake(_board, mList.moves[i].move);

        if (score > best) {
            best = score;
            bestMove = mList.moves[i].move;

            if (score > alpha) {
                hashFlag = TP_EXACT;
                alpha = score;
                updatePv(ply, bestMove);

                if (alpha >= beta) {
                    hashFlag = TP_BETA;
                    break;
                }
            }
        }
    }

    if (mList.nMoves == 0) {
        if (__builtin_popcountll(_board.checkPcs) > 0) {
            return -INF;
        }
    }

    if (!probe) {
        tt.record(_board.key, _board.ply, bestMove, 0, best, hashFlag);
    }

    return best;
}

int Search::negaMax(int alpha, int beta, int depth, bool nullMove, bool isPv) {

    nodes++;
    int hashFlag = TP_ALPHA;
    const int ply = _board.ply;

    if (ply > selDepth)
        selDepth = ply;

    if (checkForStop())
        return ABORT_SCORE;

    bool inCheck = popcount(_board.checkPcs) > 0;
    if (inCheck)
        depth++;

    if (depth <= 0)
        return quiescent(alpha, beta);

    bool futilityPrune = false;
    bool pvNode = alpha < (beta - 1);

    int best = -INF;
    int move = 0;

    tt.prefetch(_board.key);

    pvTableLen[ply] = 0;
    if (_board.ply > 0) {

        if (_board.halfMoves >= 100 || _board.isDraw())
            return 1 - (nodes & 2);

        if (_board.numRepetition() >= 2) {
            return 1 - (nodes & 2);
        }

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

    int ttScore = 0, ttMove = 0;
    TTHash entry = {0};

    if (probe) {
        if (tt.probe(_board.key, entry)) {
            ttScore = entry.score(_board.ply);
            ttMove = entry.move();
            int flag = entry.flag();
            if (entry.depth() > depth) {
                if (!pvNode && flag == TP_EXACT ||
                    (flag == TP_BETA && ttScore >= beta) ||
                    (flag == TP_ALPHA && ttScore <= alpha)) {
                    return ttScore;
                }
            }
        }
    }

    if (depth > 2 && !_board.checkPcs && !pvNode && !nullMove) {
        makeNullMove(_board);
        int score = -negaMax(-beta, -beta + 1, depth - 1 - 2, true, false);
        unmakeNullMove(_board);

        // if (checkForStop())
        //     return ABORT_SCORE;

        if (score >= beta)
            return beta;
    }

    moveList mList = {{{0}}};
    generate(_board, &mList);
    scoreMoves(&mList, ttMove);

    int futilityMargin[] = {0, 100, 500, 900};
    if (depth <= 3 && !pvNode && std::abs(alpha) < 9000 &&
        Eval(_board).eval() + futilityMargin[depth] <= alpha)
        futilityPrune = true;

    int score = 0;
    int bestMove = move;
    int movesSearched = 0;
    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const int curr_move = mList.moves[i].move;
        make(_board, mList.moves[i].move);

        if (!pvNode && depth >= 3 && movesSearched >= 5 &&
            getCapture(curr_move) < CAPTURE && !_board.checkPcs) {
            unmake(_board, mList.moves[i].move);
            continue;
        }

        if (futilityPrune && getCapture(curr_move) < CAPTURE &&
            !_board.checkPcs) {
            unmake(_board, mList.moves[i].move);
            continue;
        }

        movesSearched++;
        if (movesSearched == 1) {
            score = -negaMax(-beta, -alpha, depth - 1, false, false);
        } else {
            if (movesSearched >= 5 && depth >= 3 &&
                canReduce(alpha, curr_move, mList)) {
                score = -negaMax(-alpha - 1, -alpha, depth - 2, false, false);
            } else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -negaMax(-alpha - 1, -alpha, depth - 1, false, false);
                if (score > alpha && score < beta) {
                    score = -negaMax(-beta, -alpha, depth - 1, false, false);
                }
            }
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
            return -INF;
        }

        return 0;
    }

    if (best >= beta && getCapture(bestMove) < CAPTURE) {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = bestMove;

        historyMoves[_board.turn][getFrom(bestMove)][getTo(bestMove)] +=
              depth * depth;
    }

    tt.record(_board.key, _board.ply, bestMove, depth, best, hashFlag);

    // if (!checkForStop()) {
    //     tpTbl.recordHash(_board.fen(), _board.ply, _board.key, bestMove,
    //     depth,
    //                      best, hashFlag);
    // }

    return alpha;
}

int Search::search() {
    abortDepth = -1;
    int depth = info->depth;
    int num = 2;

    int alpha = -INF, beta = INF;
    int score = 0, prevScore = 0;
    int bestMove = 0;

    double totalTime = 0;
    for (int j = 1; j <= depth; j++) {
        double start = get_time();
        int window = 5;

        if (j >= 5) {
            alpha = std::max(-INF, prevScore - window);
            beta = std::min(INF, prevScore + window);
        } else {
            alpha = -INF;
            beta = INF;
        }

        int numFailed = 0;
        int aspirationDepth = j;

        while (true) {
            aspirationDepth = std::max(1, aspirationDepth);
            selDepth = 0;
            score = negaMax(alpha, beta, aspirationDepth, false, false);

            if (checkForStop()) {
                abortDepth = j;
                break;
            }

            if (score <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(-INF, alpha - window);
                aspirationDepth = j;
            } else if (beta <= score) {
                if (std::abs(score) < (INF / 2))
                    aspirationDepth--;
                beta = std::min(INF, beta + window);

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

            window += window / 2;
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

void Search::updatePv(int ply, int move) {
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

void Search::clearTT() { tt.init(256); }

} // namespace Yayo
