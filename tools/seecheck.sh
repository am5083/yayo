#!/bin/bash
BIN=$1
for p in "4k3/8/4p3/3p4/1N6/2N5/8/4K3 w - - 0 1:c3d5:-100" "4k3/8/4p3/3p4/8/2N5/8/4K3 w - - 0 1:c3d5:-200" "4k3/8/4p3/3p4/8/2N5/8/3RK3 w - - 0 1:c3d5:-100" "4k3/8/5p2/4b3/8/8/8/4RK2 w - - 0 1:e1e5:-180" "4k3/8/8/4p3/8/8/8/4RK2 w - - 0 1:e1e5:100" "4k3/8/8/8/8/8/8/4K3 w - - 0 1:e1e2:0"; do
  IFS=: read fen mv exp <<< "$p"
  got=$(printf "position fen $fen\nsee $mv\nquit\n" | timeout 20 $BIN 2>&1 | grep static | awk '{print $NF}')
  echo "$mv expect $exp got $got $( [ "$got" = "$exp" ] && echo OK || echo FAIL)"
done
