#!/usr/bin/env bash
#
# run_experiment.sh
# Usage: ./run_experiment.sh <PLA_NAME> <VCOUNT>
#
#  <PLA_NAME> base name of the .pla file (e.g. "pdc" or "apex1")
# !!! In order to work this file has to be in build/ directory !!!!

set -e

if [ $# -ne 1 ]; then
  echo "Usage: $0 <PLA_NAME>" >&2
  exit 1
fi

PLA_NAME=$1


# append all output (stdout+stderr) into script_output.txt
exec >> script_output_"$PLA_NAME".txt 2>&1

# --- experiment_map.conf section ---
CONFIG=../load_files/conf_files/experiment_map.conf

# Suppress the PMIx compression warning
export PMIX_MCA_pcompress_base_silence_warning=1

echo "$PLA_NAME"

# First two runs on experiment_map.conf
for i in 1 2 4 8 16; do
    echo "Cores used: $i"
    for j in {1..10}; do
    mpirun -n $i DDD/main "$CONFIG" 2 0 2 y
    done
done

