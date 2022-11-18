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
    stopCount = 0;
    stopFlag = 0;
    quiescentDepth = 0;
    qnodes = 0;
    info->uciStop = false;
    info->uciQuit = false;
    numRep = 0;

    memset(&pvTable, 0, sizeof(pvTable));
    memset(&pvTableLen, 0, sizeof(pvTableLen));
    memset(&killerMoves, 0, sizeof(killerMoves));
    memset(&killerMates, 0, sizeof(killerMates));
    memset(&historyMoves, 0, sizeof(historyMoves));
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

inline bool isCapture(unsigned short move) {
    if (move < CAPTURE)
        return false;

    if (move >= CP_KNIGHT)
        return true;

    if (move == CAPTURE)
        return true;

    if (move == EP_CAPTURE)
        return true;

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
        unsigned short move = mList->moves[i].move;

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
    selDepth = std::max(selDepth, ply);

    if (checkForStop() || stopFlag) {
        stopFlag = 1;
        return ABORT_SCORE;
    }

    tt.prefetch(_board.key);
    bool pvNode = (beta - alpha) < 1;
    pvTableLen[_board.ply] = 0;

    if (_board.ply) {
        int upper = INF - _board.ply;
        int lower = -INF + _board.ply;

        if (upper < beta) {
            beta = upper;
            if (alpha >= upper)
                return upper;
        }

        if (lower > alpha) {
            alpha = lower;
            if (beta <= lower) {
                return lower;
            }
        }
    }

    int flag = -1;
    bool ttHit = false;
    int ttScore = -INF * 2;
    unsigned short tpMove = 0;
    TTHash entry = {0};
    if (tt.probe(_board.key, entry)) {
        ttHit = true;
        ttScore = entry.score(_board.ply);
        tpMove = entry.move();
        flag = entry.flag();

        if ((flag == TP_EXACT || (flag == TP_BETA && ttScore >= beta) ||
             (flag == TP_ALPHA && ttScore <= alpha))) {
            return ttScore;
        }
    }

    Eval eval(_board);
    int score = 0, best = INF, oldAlpha = alpha;
    unsigned short bestMove = tpMove;

    int futilityMargin = -INF;
    if (_board.checkPcs) {
        futilityMargin = -INF;
        best = -INF;
        Hist[ply].eval = INF;
    } else if (ply > 1 && !Hist[ply - 1].move) {
        best = -Hist[ply - 1].eval + 2 * TEMPO;
        Hist[ply].eval = best;
        futilityMargin = 205 + best;
    } else {
        best = eval.eval();
        Hist[ply].eval = best;
        futilityMargin = 205 + best;

        if (!pvNode && ttHit &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= best) ||
             (flag == TP_ALPHA && ttScore <= best))) {
            best = ttScore;
        }
    }

    if (best >= beta)
        return best;

    int queenValue = (eval.mgPhase * MgScore(queenScore) +
                      eval.egPhase * EgScore(queenScore)) /
                     24;

    int deltaMargin = best + 200 + queenValue;
    if (!_board.checkPcs && deltaMargin <= alpha) {
        return alpha;
    }

    if (alpha < best)
        alpha = best;

    constexpr auto pcVal =
          std::array{0, PAWN_VAL, KNIGHT_VAL, BISHOP_VAL, ROOK_VAL, QUEEN_VAL};

    auto evalVal = std::array{S(0, 0),     pawnScore, knightScore,
                              bishopScore, rookScore, queenScore};

    moveList mList = {0};

    generate(_board, &mList);

    scoreMoves(&mList, tpMove);

    int standPat = best;

    auto isQuiet = [](unsigned short move) {
        switch (getCapture(move)) {
        case QUIET:
        case DOUBLE_PAWN:
        case K_CASTLE:
        case Q_CASTLE:
            return true;
            break;
        default:
            return false;
        }
    };

    bool quietsSeen = false;
    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);

        unsigned short move = mList.moves[i].move;
        Square fromSq = getFrom(move), toSq = getTo(move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = _board.board[toSq ^ 8];
        }

        if (best > -CHECKMATE) {

            // if (mList.moves[i].score < 0 && _board.checkPcs)
            //     continue;

            // if (!isQuiet(move) && mList.moves[i].score < 0) {
            //     continue;
            // }

            if (isQuiet(move) && _board.checkPcs) {
                int see = _board.see(toSq, toPc, fromSq, fromPc);
                if (see < 0)
                    continue;
            }

            if (futilityMargin > -CHECKMATE) {
                if (isQuiet(move))
                    continue;

                int idx = (toPc == NO_PC) ? 0 : int(getPcType(toPc));
                int pcValue = (eval.mgPhase * MgScore(evalVal[idx]) +
                               eval.egPhase * EgScore(evalVal[idx])) /
                              24;

                if (mList.moves[i].score < -pcValue)
                    continue;

                int val = futilityMargin + pcValue;
                if (val <= alpha && getCapture(move) < P_KNIGHT) {
                    best = std::max(best, val);
                    continue;
                }
            }
        }

        // int dMargin = standPat + pcVal[getPcType(toPc)];
        // if (dMargin < alpha && !isQuiet(move) && !_board.checkPcs)
        //     continue;

        Hist[ply].move = move;
        Hist[ply].piece = fromPc;

        qnodes++;
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

    if (!mList.nMoves) {
        if (_board.checkPcs) {
            return -INF;
        }

        return 1 - (nodes & 5);
    }

    tt.record(_board.key, _board.ply, bestMove, 0, best, hashFlag);

    return best;
}

int Search::negaMax(int alpha, int beta, int depth, bool nullMove, bool isPv,
                    bool cutNode, bool isExtension) {
    int hashFlag = TP_ALPHA;
    const int ply = _board.ply;
    pvTableLen[ply] = 0;

    tt.prefetch(_board.key);
    if (checkForStop()) {
        stopFlag = 1;
        return ABORT_SCORE;
    }

    bool inCheck = _board.checkPcs;
    if (inCheck && !isExtension) {
        depth++;
        isExtension = true;
    }

    if (depth <= 0)
        return quiescent(alpha, beta);

    bool futilityPrune = false;
    bool pvNode = alpha < (beta - 1);
    bool isRoot = (ply == 0);

    if (_board.ply > 0) {
        if (_board.halfMoves >= 100 || _board.isDraw())
            return 1 - (nodes & 2);

        if (_board.isTMR())
            return 1 - (nodes & 2);

        int upper = INF - _board.ply;
        int lower = -INF + _board.ply;

        if (upper < beta) {
            beta = upper;
            if (alpha >= upper)
                return upper;
        }

        if (lower > alpha) {
            alpha = lower;
            if (beta <= lower) {
                return lower;
            }
        }
    }

    bool ttHit = false;
    int ttScore = -INF, ttMove = 0, flag = -1;
    TTHash entry = {0};
    if (probe) {
        if (tt.probe(_board.key, entry)) {
            ttHit = true;
            ttScore = entry.score(_board.ply);
            ttMove = entry.move();
            flag = entry.flag();
            if (!pvNode && entry.depth() >= depth) {
                if ((flag == TP_EXACT || (flag == TP_BETA && ttScore >= beta) ||
                     (flag == TP_ALPHA && ttScore <= alpha))) {
                    return ttScore;
                }
            }
        }
    }

    int futilityMargin[] = {0, 100, 300, 700};

    Eval eval(_board);
    int best = -INF;
    int evalScore = -INF;
    unsigned short move = ttMove;
    int score = ttScore;

    if (_board.checkPcs) {
        evalScore = -INF;
        Hist[ply].eval = INF;
    } else if (ply > 1 && !Hist[ply - 1].move) {
        evalScore = -Hist[ply - 1].eval + 2 * TEMPO;
        Hist[ply].eval = evalScore;
    } else {
        evalScore = eval.eval();
        Hist[ply].eval = evalScore;

        if (ttHit &&
            (flag == TP_EXACT || (flag == TP_BETA && ttScore >= evalScore) ||
             (flag == TP_ALPHA && ttScore <= evalScore))) {
            evalScore = ttScore;
        }
    }

    killerMoves[ply + 1][1] = 0;
    killerMoves[ply + 1][0] = 0;
    killerMates[ply + 1][1] = 0;
    killerMates[ply + 1][0] = 0;

    bool improving =
          (!inCheck && ply >= 2 && Hist[ply].eval > Hist[ply - 2].eval);

    if (!pvNode && !inCheck && !isExtension && depth < 2) {
        if (Hist[ply].eval + (150 * improving) + 175 < alpha)
            return quiescent(alpha, beta);
    }

    // static NMP
    if (!pvNode && !_board.checkPcs && depth <= 8 &&
        evalScore - (97 - 74 * improving) * depth > beta &&
        std::abs(alpha) < CHECKMATE)
        return evalScore - (97 - 74 * improving) * depth;

    int R = 0;
    if (depth > 1 && !_board.checkPcs && !pvNode && evalScore >= beta &&
        (_board.pieces(_board.turn) ^ _board.pieces(PAWN, _board.turn) ^
         _board.pieces(KING, _board.turn)) &&
        (!ttHit || !(flag == TP_ALPHA || flag == TP_EXACT) ||
         ttScore >= beta)) {
        R = 4 + (depth / 6);
        R += std::min(3, (evalScore - beta) / 150);
        R += improving;
        R -= !improving;
        // std::min(2, (evalScore - beta) / 100) - !improving;

        Hist[ply].move = NO_MOVE;
        Hist[ply].piece = NO_PC;

        makeNullMove(_board);
        score = -negaMax(-beta, -beta + 1, depth - R, true, false, !cutNode,
                         isExtension);
        unmakeNullMove(_board);

        if (score >= beta)
            return std::abs(score) > CHECKMATE ? beta : score;
    }

    moveList mList = {{{0}}};
    generate(_board, &mList);
    scoreMoves(&mList, ttMove);

    if (depth <= 3 && !pvNode && std::abs(alpha) < CHECKMATE &&
        evalScore + futilityMargin[depth] <= alpha && mList.nMoves > 0)
        futilityPrune = true;

    if (!pvNode && !inCheck && depth >= 4 && !ttHit)
        depth--;

    auto isQuiet = [](unsigned short move) {
        switch (getCapture(move)) {
        case QUIET:
        case DOUBLE_PAWN:
        case K_CASTLE:
        case Q_CASTLE:
            return true;
            break;
        default:
            return false;
        }
    };

    bool skip = false;
    unsigned short bestMove = move;
    int movesSearched = 0;
    int nQuiets = 0;
    for (int i = 0; i < mList.nMoves; i++) {
        mList.swapBest(i);
        const int curr_move = mList.moves[i].move;
        bool inCheck = _board.checkPcs;

        skip *= ((best > -CHECKMATE) && !improving);
        if (skip && getCapture(curr_move) < CAPTURE) {
            continue;
        }

        Square fromSq = getFrom(curr_move), toSq = getTo(curr_move);
        Piece fromPc = _board.board[fromSq], toPc = _board.board[toSq];

        if (getCapture(move) == EP_CAPTURE) {
            toPc = _board.board[toSq ^ 8];
        }

        if (!isRoot && best > -CHECKMATE) {
            if (getCapture(curr_move) < CAPTURE) {
                int reducedDepth = std::max(
                      2, (int)lmrDepthReduction[std::min(63, depth)]
                                               [std::min(63, movesSearched)]);
                if (depth <= 8 && !inCheck &&
                    evalScore + 200 + 225 * reducedDepth + 100 * improving <=
                          alpha) {
                    skip = 1;
                }

                reducedDepth = std::max(1, reducedDepth);
                if (depth <= 8 && !pvNode &&
                    nQuiets >= lmpThresholds[improving][depth])
                    skip = true;

                // if (futilityPrune && !inCheck && !pvNode)
                //     skip = true;

                int see = _board.see(toSq, toPc, fromSq, fromPc);

                if (depth <= 8 && see < -100 * depth)
                    continue;
            } else {
                int see = _board.see(toSq, toPc, fromSq, fromPc);

                // if (depth <= 4 &&
                //     see < -(10 + 14 * improving + 15 * pvNode) * depth *
                //     depth) continue;

                if (depth <= 7 &&
                    see < -(18 + 5 * improving + 12 * pvNode) * depth * depth)
                    continue;
            }
        }

        Hist[ply].move = curr_move;
        Hist[ply].piece = fromPc;

        make(_board, mList.moves[i].move);

        // if (skip && !_board.checkPcs) {
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

        // if (futilityPrune && getCapture(curr_move) < CAPTURE &&
        //     !_board.checkPcs) {
        //     unmake(_board, mList.moves[i].move);
        //     continue;
        // }
        //
        if (getCapture(curr_move) < CAPTURE)
            nQuiets++;

        nodes++;
        movesSearched++;
        if (movesSearched == 1) {
            score = -negaMax(-beta, -alpha, depth - 1, false, false, false,
                             isExtension);
        } else {
            if (movesSearched > (2 + (1 * isRoot)) && depth >= 3) {
                // int R = 2 + (depth / 10);
                // R += movesSearched / 15;
                R = lmrDepthReduction[std::min(63, depth)]
                                     [std::min(63, movesSearched)];

                R += cutNode;
                R += !improving;
                R += !pvNode;
                R += (mList.moves[i].score < 0);
                R += ttHit;

                // R -= pvNode;
                // R -= !cutNode;
                R -= improving;
                // R -= _board.checkPcs;
                // R -= nullMove;
                R -= 2 * (mList.moves[i].score > 10000);
                R += 2 * (mList.moves[i].score < 0);
                int refutationMove =
                      historyMoves[_board.turn][fromSq][toSq] / 2048;
                if (isQuiet(move))
                    R -= std::max(-2, std::min(2, refutationMove));

                // R -= 2 * (mList.moves[i].score > 10000 &&
                //           mList.moves[i].score < 20000);
                // R -= 3 * (mList.moves[i].score > 20000);

                R = std::min(depth - 1, std::max(1, R));
                score = -negaMax(-alpha - 1, -alpha, depth - R, false, false,
                                 true, isExtension);
            } else {
                score = alpha + 1;
            }

            if (score > alpha) {
                score = -negaMax(-alpha - 1, -alpha, depth - 1, false, false,
                                 !cutNode, isExtension);
                if (score > alpha && score < beta) {
                    score = -negaMax(-beta, -alpha, depth - 1, false, false,
                                     false, isExtension);
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

        return 1 - (nodes & 2);
    }

    if (best >= beta && getCapture(bestMove) < CAPTURE) {
        if (killerMoves[ply][0] != bestMove) {
            killerMoves[ply][1] = killerMoves[ply][0];
            killerMoves[ply][0] = bestMove;
        }

        historyMoves[_board.turn][getFrom(bestMove)][getTo(bestMove)] +=
              depth * depth;
    }

    tt.record(_board.key, _board.ply, bestMove, depth, best, hashFlag);

    return best;
}

int Search::search() {
    abortDepth = -1;
    int depth = info->depth;
    int num = 2;

    int alpha = -INF, beta = INF;
    int score = 0, prevScore = -INF;
    unsigned short bestMove = 0;

    double totalTime = 0;
    for (int j = 1; j <= depth; j++) {

        double start = get_time();
        int window = 9;

        if (j >= 2) {
            alpha = std::max(-INF, prevScore - window);
            beta = std::min(INF, prevScore + window);
        } else {
            alpha = -INF;
            beta = INF;
        }

        num = 0;
        int numFailed = 0;
        int aspirationDepth = j;
        _board.ply = 0;

        while (true) {

            if (checkForStop()) {
                abortDepth = aspirationDepth;
                break;
            }

            num++;
            aspirationDepth = std::max(1, aspirationDepth);
            selDepth = 0;
            quiescentDepth = 0;
            score = negaMax(alpha, beta, aspirationDepth, false, true, false,
                            false);

            if (score <= alpha) {
                numFailed = 0;

                beta = (alpha + beta) / 2;
                alpha = std::max(-INF, alpha - window);
                aspirationDepth = j;
            } else if (beta <= score) {
                // std::cout << "beta failllllll\n"
                //           << "beta: " << beta << ", bw: " << beta + window
                //           << ", window: " << window << ", score: " << score
                //           << "\n\n";
                numFailed++;
                beta = std::min(INF, beta + window);

                if (std::abs(score) < CHECKMATE)
                    aspirationDepth--;

                if (numFailed > 1) {
                    window += window / 2;
                    alpha = -INF;
                    beta = INF;
                    continue;
                }

                if (pvTableLen[0] && !bestMove)
                    bestMove = pvTable[0][0];

            } else {
                if (aspirationDepth < j) {
                    aspirationDepth = j;
                    continue;
                }

                if (pvTableLen[0])
                    bestMove = pvTable[0][0];

                double end = ((get_time() - start) + 1) / 1000.0;
                totalTime += end;
                long double nps = (nodes / (totalTime));

                std::cout << std::fixed << "info depth " << aspirationDepth;
                std::cout << " seldepth " << selDepth;
                std::cout << " qdepth " << selDepth - j;
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

                std::cout << " qnodes " << qnodes;
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

void Search::updatePv(int ply, unsigned short move) {
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
