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
  FS=": "
}
# first non-empty line is the PLA name
NR==1 {
  # trim whitespace, in case
  pla = $1
  gsub(/^[ \t]+|[ \t]+$/, "", pla)
  # print header
  print pla > OUTPUT
  next
}
# capture only the Time: lines
/^Time:/ {
  val = $2
  # convert decimal point to comma
  gsub(/\./, ",", val)
  # print as single-column semicolon‐delimited CSV
  print val > OUTPUT
}
' "$raw"
