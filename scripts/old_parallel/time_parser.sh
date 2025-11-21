#!/usr/bin/env bash
#
# parse_times.sh
# Usage: ./parse_times.sh raw_data.txt parsed_times.csv

if [ $# -ne 2 ]; then
  echo "Usage: $0 <raw_input> <csv_output>" >&2
  exit 1
fi

raw="$1"
out="$2"

awk -v OUTPUT="$out" '
BEGIN {
  FS = ": "
}
NR == 1 {
  # first line is our one-column header (e.g. "apex4")
  print $0 > OUTPUT
  next
}
/^Cores used:/ {
  core = $2
  runs[core] = 0
  if (!(core in seen)) {
    seen[core] = 1
    cores[++nc] = core
  }
  next
}
/^Time:/ {
  runs[core]++
  v = $2
  gsub(/\./, ",", v)
  times[core, runs[core]] = v
  if (runs[core] > maxruns) maxruns = runs[core]
}
END {
  # sort numeric cores ascending
  for (i=1; i<=nc; i++) numcores[i] = cores[i]
  n = asort(numcores)

  # print header: 1;2;...;N
  printf "%s", numcores[1] > OUTPUT
  for (i=2; i<=n; i++) printf ";%s", numcores[i] >> OUTPUT
  printf "\n" >> OUTPUT

  # for each run r, print columns for each core in order
  for (r=1; r<=maxruns; r++) {
    printf "%s", times[numcores[1], r] > OUTPUT
    for (i=2; i<=n; i++) {
      printf ";%s", times[numcores[i], r] >> OUTPUT
    }
    printf "\n" >> OUTPUT
  }
}
' "$raw"
