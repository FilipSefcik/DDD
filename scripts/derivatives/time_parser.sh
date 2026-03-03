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

# Check if the output file already exists
if [ ! -f "$out" ]; then
  # If the file doesn't exist, create it and add headers
  echo "Cores;Run1;Run2;Run3;Run4;Run5;Run6;Run7;Run8;Run9;Run10" > "$out"
fi

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

  # Add an empty row before appending new data
  if (NR > 1) {
    print "" >> OUTPUT
  }

  # Print the core data with cores in the first column
  for (i = 1; i <= n; i++) {
    core = numcores[i]
    row = core
    for (r = 1; r <= maxruns; r++) {
      v = times[core, r]
      gsub(/\./, ",", v)
      row = row ";" v
    }
    print row >> OUTPUT  # Append to the existing CSV
  }
}
' "$raw"
