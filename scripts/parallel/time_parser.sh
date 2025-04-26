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
# whenever we see "Cores used: X", start a new core bucket
/^Cores used:/ {
  core = $2
  runs[core] = 0
  if (!(core in seen)) {
    seen[core] = 1
    cores[++nc] = core
  }
  next
}
# whenever we see "Time: value", stash it
/^Time:/ {
  val = $2
  runs[core]++
  times[core, runs[core]] = val
  if (runs[core] > maxruns) maxruns = runs[core]
}
END {
  # sort the numeric core list
  for (i = 1; i <= nc; i++) {
    numcores[++cn] = cores[i]
  }
  n = asort(numcores)

  # print header: "1;2;3;...;N"
  header = numcores[1]
  for (i = 2; i <= n; i++) {
    header = header ";" numcores[i]
  }
  print header > OUTPUT

  # print each run row
  for (r = 1; r <= maxruns; r++) {
    row = ""
    for (i = 1; i <= n; i++) {
      c = numcores[i]
      v = times[c, r]
      gsub(/\./, ",", v)
      row = row (i>1 ? ";" : "") v
    }
    print row > OUTPUT
  }
}
' "$raw"
