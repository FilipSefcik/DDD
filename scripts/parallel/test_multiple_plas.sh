#!/usr/bin/env bash
#
# batch_run.sh
# Usage: ./batch_run.sh
#
# Make sure run_experiment.sh is in the same directory and is executable.
# !!! In order to work this file has to be in build/ directory !!!!

set -e

# Check for associative‐array support
if ! (declare -A &>/dev/null); then
  echo "Error: your Bash doesn't support associative arrays." >&2
  exit 1
fi

# Define PLA files and their variable‑counts here:
declare -A PLA_VCOUNTS=(
  #[alu4]=14
  [apex1]=45
  #[apex2]=39
  #[apex4]=9
  #[apex5]=117
  #[b12]=15
  [cordic]=23
  #[cps]=24
  #[ex4]=128
  #[ex5]=8
  #[ex1010]=10
  [misex3]=14
  [misex3c]=14
  #[pdc]=16
  #[seq]=41
  #[spla]=16
  [t481]=16
)

# Loop over each entry
for PLA in "${!PLA_VCOUNTS[@]}"; do
  VCOUNT=${PLA_VCOUNTS[$PLA]}
  echo "=== Running experiment for ${PLA}.pla with VCOUNT=${VCOUNT} ==="
  ./run_experiment.sh "$PLA" "$VCOUNT"
  ./time_parser.sh "script_output_${PLA}.txt" "parsed_times_${PLA}.csv"
  echo
done
