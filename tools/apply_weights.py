#!/usr/bin/env python3
"""Rewrite src/weights.hpp from the last weight block printed by `yayo tune`.

usage: python3 tools/apply_weights.py tune.log src/weights.hpp

The tuner prints a block that starts with 'constexpr Score pawnScore' every
REPORTING epochs. This script takes the LAST such block and writes a complete
new weights.hpp (global constexpr arrays and the EvalWeights struct, both).
"""
import re
import sys

NAMES = [
    ("pawnScore", None), ("knightScore", None), ("bishopScore", None),
    ("rookScore", None), ("queenScore", None),
    ("taperedPawnPcSq", "SQUARE_CT"), ("taperedKnightPcSq", "SQUARE_CT"),
    ("taperedBishopPcSq", "SQUARE_CT"), ("taperedRookPcSq", "SQUARE_CT"),
    ("taperedQueenPcSq", "SQUARE_CT"), ("taperedKingPcSq", "SQUARE_CT"),
    ("passedPawnRankBonus", "8"), ("doubledPawnRankBonus", "8"),
    ("isolatedPawnRankBonus", "8"), ("backwardPawnRankBonus", "8"),
    ("KnightMobilityScore", "9"), ("BishopMobilityScore", "14"),
    ("RookMobilityScore", "15"), ("QueenMobilityScore", "28"),
]
SIZES = {"SQUARE_CT": 64, "8": 8, "9": 9, "14": 14, "15": 15, "28": 28}


def parse_last_block(text):
    starts = [m.start() for m in re.finditer(r"constexpr Score pawnScore", text)]
    if not starts:
        sys.exit("ERROR: no weight block found in the log")
    block = text[starts[-1]:]
    out = {}
    for name, size in NAMES:
        if size is None:
            m = re.search(r"constexpr Score %s\s*=\s*S\(\s*(-?\d+),\s*(-?\d+)\)" % name, block)
            if not m:
                sys.exit("ERROR: scalar %s not found" % name)
            out[name] = [(int(m.group(1)), int(m.group(2)))]
        else:
            m = re.search(r"constexpr Score %s\[[^\]]*\]\s*=\s*\{(.*?)\};" % name, block, re.S)
            if not m:
                sys.exit("ERROR: array %s not found" % name)
            pairs = re.findall(r"S\(\s*(-?\d+),\s*(-?\d+)\)", m.group(1))
            if len(pairs) != SIZES[size]:
                sys.exit("ERROR: %s has %d entries, expected %d" % (name, len(pairs), SIZES[size]))
            out[name] = [(int(a), int(b)) for a, b in pairs]
    return out


def fmt_array(pairs, per_line):
    lines = []
    for i in range(0, len(pairs), per_line):
        lines.append("      " + " ".join("S(%d, %d)," % p for p in pairs[i:i + per_line]))
    return "\n".join(lines)


def render(w):
    g = ["#ifndef WEIGHTS_H_", "#define WEIGHTS_H_", '#include "util.hpp"', ""]
    s = ["struct EvalWeights {"]
    for name, size in NAMES:
        if size is None:
            mg, eg = w[name][0]
            g.append("constexpr Score %s = S(%d, %d);" % (name, mg, eg))
            s.append("    const Score %s = S(%d, %d);" % (name, mg, eg))
        else:
            body = fmt_array(w[name], 8 if size == "SQUARE_CT" else 4)
            g.append("constexpr Score %s[%s] = {\n%s\n};" % (name, size, body))
            s.append("    const Score %s[%s] = {\n%s\n    };" % (name, size, body.replace("      ", "          ")))
    s.append("};")
    return "\n".join(g) + "\n\n" + "\n".join(s) + "\n#endif // WEIGHTS_H_\n"


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    w = parse_last_block(open(sys.argv[1]).read())
    open(sys.argv[2], "w").write(render(w))
    print("wrote %s with %d weight tables" % (sys.argv[2], len(NAMES)))
