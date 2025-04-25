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
  FS=": ";
}
# record cores
/^Cores used:/ {
  core = $2
  runs[core] = 0
  if (!(core in seen)) {
    seen[core]=1; cores[++nc]=core
  }
  next
}
# record merged section
/^Merged/ {
  core = "merged"
  runs[core] = 0
  if (!("merged" in seen)) {
    seen["merged"]=1; cores[++nc]="merged"
  }
  next
}
# capture time values
/^Time:/ {
  val = $2
  runs[core]++
  times[core, runs[core]] = val
  if (runs[core] > maxruns) maxruns = runs[core]
}
END {
  # sort numeric cores (exclude merged)
  for (i=1; i<=nc; i++) {
    c=cores[i]
    if (c!="merged") numcores[++cn]=c
  }
  n=asort(numcores)

  # write header: cores then merged
  header=""
  for(i=1;i<=n;i++){
    header = header numcores[i] ";"
  }
  header = header "merged"
  print header > OUTPUT

  # write each run row
  for(r=1; r<=maxruns; r++){
    row=""
    for(i=1;i<=n;i++){
      c=numcores[i]
      v = times[c, r]
      gsub(/\./, ",", v)
      row = row v ";"
    }
    row = row times["merged", r]
    gsub(/\./, ",", row)
    print row > OUTPUT
  }
}
' "$raw"
