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

namespace Yayo {

void Search::startSearch(Info *_info) {
    if (searched)
        searchThread->join();
    searched      = 1;
    info          = _info;
    nodes         = 0;
    info->uciStop = false;
    info->uciQuit = false;
    numRep        = 0;
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

void Search::scoreMoves(moveList *mList) {
    for (int i = 0; i < mList->nMoves; i++) {
        int move = mList->moves[i].move;

        if (move == pvTable[0][_board.ply]) {
            mList->moves[i].score = 10000000 * 2;
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
                mList->moves[i].score = historyMoves[_board.turn][getFrom(move)][getTo(move)];
            }
        }
    }
}

int Search::quiescent(int alpha, int beta) {
    int ply = _board.ply;
    if (ply > selDepth)
        selDepth = ply;

    // mate distance pruning
    int mate_val = INF - ply;
    if (mate_val <= beta) {
        beta = mate_val;

        if (alpha >= beta)
            return alpha;
    }

    mate_val = -INF + ply;
    if (mate_val >= alpha) {
        alpha = mate_val;

        if (beta <= mate_val)
            return beta;
    }

    if (_board.checkPcs) {
        return negaMax(alpha, beta, 1);
    }

    Eval eval(_board);
    int standPat = eval.eval();

    if (standPat >= beta)
        return beta;

    int delta = (MgScore(queenScore) * eval.mgPhase + EgScore(queenScore) * eval.egPhase) / 24;
    if (standPat < alpha - delta)
        return alpha;

    if (alpha < standPat)
        alpha = standPat;

    pvTableLen[ply] = 0;

    moveList mList = {0};
    generate(_board, &mList);

    for (int i = 0; i < mList.nMoves; i++) {
        if (mList.moves[i].score == 0) {
            continue;
        }

        const int c_move = mList.moves[i].move;

        Square fromSq = getFrom(c_move), toSq = getTo(c_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getPcType(fromPc) > getPcType(toPc)) {
            int see = _board.see(toSq, toPc, fromSq, fromPc);
            if (see < 0) {
                mList.moves[i].score = (see / 1000) + 50;
            } else {
                mList.moves[i].score = see;
            }
        }
    }

    int score;
    for (int i = 0; i < mList.nMoves; i++) {
        if (mList.moves[i].score == 0)
            continue;

        mList.swapBest(i);

        make(_board, mList.moves[i].move);
        score = -quiescent(-beta, -alpha);
        unmake(_board, mList.moves[i].move);

        if (score >= beta)
            return beta;
        if (score > alpha) {
            updatePv(_board.ply, mList.moves[i].move);
            alpha = score;
        }
    }

    if (mList.nMoves == 0) {
        if (__builtin_popcountll(_board.checkPcs) > 0) {
            return -INF + _board.ply;
        }
    }

    return alpha;
}

int Search::negaMax(int alpha, int beta, int depth) {
    int hashFlag  = TP_ALPHA;
    const int ply = _board.ply;

    if (ply > selDepth)
        selDepth = ply;

    if (checkForStop())
        return ABORT_SCORE;

    if (depth <= 0)
        return quiescent(alpha, beta);

    pvTableLen[ply] = 0;

    if (_board.ply > 0) {
        alpha = std::max(alpha, -INF + _board.ply);
        beta  = std::min(beta, INF - _board.ply);

        if (alpha >= beta) {
            if (killerMates[ply][0] != pvTable[ply][0]) {
                killerMates[ply][1] = killerMates[ply][0];
                killerMates[ply][0] = pvTable[ply][0];
            }

            return alpha;
        }

        if (_board.halfMoves >= 100 || _board.isDraw())
            return 1 - (nodes & 2);

        if (_board.isRepetition()) {
            numRep++;
            if (_board.numRepetition() >= 2 || numRep > 2) {
                return 1 - (nodes & 2);
            }
        }
    }

    moveList mList = {{{0}}};
    generate(_board, &mList);
    scoreMoves(&mList);

    int score = 0;
    int move  = mList.moves[0].move;

    if (depth != abortDepth) {
        if ((score = tpTbl.probeHash(_board.ply, _board.key, &move, depth, alpha, beta)) != TP_UNKNOWN) {
            if (!(alpha < beta - 1))
                return score;
        }
    }

    if (!_board.checkPcs && canNullMove) {
        canNullMove = false;
        makeNullMove(_board);
        score = -negaMax(-beta, -beta + 1, depth - 1 - 2);
        unmakeNullMove(_board);

        if (checkForStop())
            return ABORT_SCORE;

        if (score >= beta)
            return beta;
    }

    int bestMove      = move;
    int movesSearched = 0;
    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const int curr_move = mList.moves[i].move;
        nodes++;

        make(_board, mList.moves[i].move);
        movesSearched++;
        if (movesSearched == 1) {
            score = -negaMax(-beta, -alpha, depth - 1);
        } else {
            if (movesSearched >= 5 && depth >= 3 && canReduce(alpha, curr_move, mList)) {
                score = -negaMax(-alpha - 1, -alpha, depth - 2);
            } else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -negaMax(-alpha - 1, -alpha, depth - 1);
                if (score > alpha && score < beta) {
                    score = -negaMax(-beta, -alpha, depth - 1);
                }
            }
        }

        unmake(_board, mList.moves[i].move);

        if (score >= beta) {
            if (getCapture(curr_move) < CAPTURE) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = mList.moves[i].move;

                historyMoves[_board.turn][getFrom(curr_move)][getTo(curr_move)] += depth * depth;
            }

            if (!checkForStop())
                tpTbl.recordHash(_board.fen(), _board.ply, _board.hash(), curr_move, depth, beta, TP_BETA);

            return score;
        }

        if (score > alpha) {
            updatePv(_board.ply, mList.moves[i].move);
            alpha    = score;
            bestMove = curr_move;
            hashFlag = TP_EXACT;
            if (!checkForStop())
                tpTbl.recordHash(_board.fen(), _board.ply, _board.hash(), curr_move, depth, alpha, hashFlag);
        }
    }

    if (mList.nMoves == 0) {
        if (_board.checkPcs) {
            return -INF + _board.ply;
        }

        return 0;
    }

    if (!checkForStop()) {
        tpTbl.recordHash(_board.fen(), _board.ply, _board.hash(), bestMove, depth, score, hashFlag);
    }

    return alpha;
}

int Search::search() {
    abortDepth = 0;
    int depth  = info->depth;

    int score;
    double start = get_time();
    for (int j = 1; j <= depth; j++) {
        canNullMove = true;
        score       = negaMax(-(INF * 2), INF * 2, j);

        if (checkForStop()) {
            abortDepth = j;
            break;
        }

        double end      = ((get_time() - start) + 1) / 1000.0;
        long double nps = (nodes / (end));

        std::cout << std::fixed << "info depth " << j << " seldepth " << selDepth;
        std::cout << " hashfull " << tpTbl.hashfull();
        std::cout << " score";

        if (std::abs(score) > (INF - MAX_PLY)) {
            int tscore = score;
            if (score < 0)
                score = -1;
            else
                score = 1;
            std::cout << " mate " << score * ((INF - std::abs(tscore)) / 2);
        } else {
            std::cout << " cp " << score;
        }
        std::cout << " nodes " << nodes;
        std::cout << " nps " << int(nps) << " time " << int(end * 1000) << " pv ";
        printPv();
        std::cout << std::endl;
    }

    std::cout << "bestmove ";
    print_move(pvTable[_board.ply][0]);
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
        if (pvTable[0][i] == NO_MOVE)
            break;
        print_move(pvTable[0][i]);
        std::cout << " ";
    }
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

void Search::clearTT() { tpTbl.clear(); }

} // namespace Yayo
