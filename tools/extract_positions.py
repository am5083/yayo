#!/usr/bin/env python3
"""Make a labeled position file for `yayo tune` from a self-play PGN.

usage: python3 tools/extract_positions.py selfplay_games.pgn selfplay.pgn

Output line format:  <fen> [1.0]   (result from the White point of view)
Rules: skip the first 8 plies, skip positions in check, skip positions where
the played move is a capture, skip games without a 1-0 / 0-1 / 1/2-1/2 result.
"""
import sys
import chess
import chess.pgn

RESULT = {"1-0": "1.0", "0-1": "0.0", "1/2-1/2": "0.5"}

src, dst = sys.argv[1], sys.argv[2]
n_games = n_pos = 0
with open(src) as fin, open(dst, "w") as fout:
    while True:
        game = chess.pgn.read_game(fin)
        if game is None:
            break
        label = RESULT.get(game.headers.get("Result", ""))
        if label is None:
            continue
        n_games += 1
        board = game.board()
        ply = 0
        for move in game.mainline_moves():
            if ply >= 8 and not board.is_check() and not board.is_capture(move):
                fout.write("%s [%s]\n" % (board.fen(), label))
                n_pos += 1
            board.push(move)
            ply += 1
print("games %d positions %d" % (n_games, n_pos))
