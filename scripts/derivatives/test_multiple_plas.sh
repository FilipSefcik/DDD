#!/usr/bin/env bash
#
# batch_run.sh
# Usage: ./batch_run.sh
#
# Make sure run_experiment.sh is in the same directory and is executable.
# !!! In order to work this file has to be in build/ directory !!!

set -e

# Loop over the different numbers (10, 20, 30, 40)
# for number in 10 20 30 40; do
for number in 10; do
  # Loop over the IDs (0 to 9)
  # for id in {0..9}; do
  for id in 0 1; do
    PLA="generated_${number}_${id}"
    echo "=== Running experiment for ${PLA}.pla ==="

    # Set the first parameter for create_conf.sh based on the number
    if [[ "$number" -eq 10 ]]; then
      param1=4
    elif [[ "$number" -eq 20 ]]; then
      param1=3
    elif [[ "$number" -eq 30 ]]; then
      param1=2
    elif [[ "$number" -eq 40 ]]; then
      param1=1
    fi

    # Create the configuration file
    ./create_conf.sh "$param1" "$number" "$PLA.pla"
    
    # Run the experiment
    ./run_experiment.sh "$PLA"
    
    # Parse the output times
    # ./time_parser.sh "script_output_${PLA}.txt" "parsed_times_${number}.csv"
    echo
  done
done
