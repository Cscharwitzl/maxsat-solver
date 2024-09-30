#!/bin/sh
die () {
  echo "*** mkall.sh: $*" 1>&2
  exit 1
}
msg () {
  echo "$*"
}

sats="`ls sat/*/makefile|sed -e 's,sat/,,' -e 's,/makefile,,'`"

## Uncomment and update to only build selected solver-app pairs
#apps="genipalsp genipaessentials"
# sats="picosat961"

[ x"$sats" = x ] && die "no 'sat/*/makefile' found"
for sat in $sats
  do
    msg "scripts/mkone.sh experiments $sat"
    scripts/mkone.sh experiments $sat
  done
