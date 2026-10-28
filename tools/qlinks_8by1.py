#!/usr/bin/env python3
"""Make "Q-Links - 8by1.json" (Force: 8 Q-Link knobs in one row) from "Q-Links.json" (the 4x4 / 4-knob map).

    python3 qlinks_8by1.py "<Plugin Skins dir>"

How the Force reads a sub-page (worked out from three runs on the device): knob k (0-based, bank 1 = 0-7, bank 2 = 8-15) edits
JSON slot FORCE[k] = [12,8,4,0,13,9,5,1, 14,10,6,2,15,11,7,3], i.e. the 4x4 grid read by columns, each column from the bottom
row up. The 4x4 map's step pages therefore show 1 5 9 13 2 6 10 14 | 3 7 11 15 4 8 12 16 on the Force.

Here the slots are permuted so the knobs read in order: bank 1 = cells 1-8, bank 2 = cells 9-16 (left to right). A page with
gaps (SETUP, MOD) is compacted: its used parameters in order, 8 per bank. Q-Links.json is left alone (MPC Live / X).
"""
import json, sys, os

def slot_of(i): return (3 - i // 4) + 4 * (i % 4)          # JSON slot i -> layout slot (column-major, top first)

FORCE = [12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3]   # the JSON slot each Force knob (bank 1, then bank 2) edits

def convert(q):
    L = [q.get('Q-Link %d' % k, -1) for k in range(1, 17)]
    S = [None] * 16
    for i, v in enumerate(L): S[slot_of(i)] = v               # layout order: column 1 top..bottom, column 2 ...
    if all(v >= 0 for v in S):                                # step page: reading order of the 4x4 cell grid = steps 1..16
        D = [S[4 * c + m] for m in range(4) for c in range(4)]
    else:                                                     # SETUP / MOD: drop the gaps
        used = [v for v in S if v >= 0]; D = used + [-1] * (16 - len(used))
    N = [None] * 16
    for k in range(16): N[FORCE[k]] = D[k]                    # knob k of the Force reads slot FORCE[k]
    return {'Q-Link %d' % (k + 1): N[k] for k in range(16)}

d = sys.argv[1]
j = json.load(open(os.path.join(d, 'Q-Links.json')))
for e in j['Screen Mode Q-Links']['map']: e['Q-Links'] = convert(e['Q-Links'])
j['Program Mode Q-Links'] = convert(j['Program Mode Q-Links'])
json.dump(j, open(os.path.join(d, 'Q-Links - 8by1.json'), 'w'), indent=4)
print('wrote Q-Links - 8by1.json:', len(j['Screen Mode Q-Links']['map']), 'sub-pages')
