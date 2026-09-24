#!/usr/bin/env bash
#
# run_experiment.sh
# Usage: ./run_experiment.sh <PLA_NAME> <VCOUNT>
#
#  <PLA_NAME> base name of the .pla file (e.g. "pdc" or "apex1")
# !!! In order to work this file has to be in build/ directory !!!!

set -e

if [ $# -ne 2 ]; then
  echo "Usage: $0 <PLA_NAME> <RUN_ID>" >&2
  exit 1
fi

PLA_NAME=$1
RUN_ID=$2


# append all output (stdout+stderr) into script_output.txt
exec >> test_outputs/script_output_divider_"$RUN_ID".txt 2>&1

# --- experiment_map.conf section ---
CONFIG="$PLA_NAME/system.conf"

# Suppress the PMIx compression warning
export PMIX_MCA_pcompress_base_silence_warning=1

echo "==Running experiment with MAP structure=="

# First two runs on experiment_map.conf
for i in 1 2 4 8 16; do
    echo "Cores used: $i"
    for j in 0 1 2; do
      echo "Divider used: $j"
      mpirun -n $i DDD/main "$CONFIG" $j 0 2 y
    done
    echo "==============================="
done

echo "============================================"

