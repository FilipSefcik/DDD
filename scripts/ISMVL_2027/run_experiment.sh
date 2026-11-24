#!/usr/bin/env bash
#
# run_experiment.sh
# Usage: ./run_experiment.sh <PLA_DIR> <RUN_ID>
#
# Compares the computation time on the decomposed structure with the time
# on the single merged module.
# !!! In order to work this file has to be in build/ directory !!!!

set -e

if [ $# -ne 2 ]; then
  echo "Usage: $0 <PLA_DIR> <RUN_ID>" >&2
  exit 1
fi

PLA_DIR=$1
RUN_ID=$2

# append all output (stdout+stderr) into the per-experiment file
exec >> test_outputs/script_output_divider_"$RUN_ID".txt 2>&1

CONFIG="$PLA_DIR/system.conf"
MERGED_DIR="../merged"
MERGED_PLA="$MERGED_DIR/M0.pla"
NEW_CONFIG="$MERGED_DIR/merged_experiment.conf"

# Suppress the PMIx compression warning
export PMIX_MCA_pcompress_base_silence_warning=1

# Never read a stale result left over from the previous experiment
mkdir -p "$MERGED_DIR"
rm -f "$MERGED_DIR"/*.pla "$NEW_CONFIG"

# 1) calculation on the decomposed structure
echo "==DECOMPOSED=="
mpirun -n 1 DDD/main "$CONFIG" 0 0 2 y

# 2) merge the PLAs into ../merged/M0.pla
echo "==MERGING=="
mpirun -n 16 DDD/main "$CONFIG" 0 0 1 n

if [ ! -f "$MERGED_PLA" ]; then
  echo "ERROR: $MERGED_PLA was not created by the merge step"
  exit 1
fi

# 3) number of variables = first number after .mv minus 1 (the output).
#    No binary variables are expected.
MV_COUNT=$(awk '$1 == ".mv" { print $2; exit }' "$MERGED_PLA")
N_VARS=$((MV_COUNT - 1))
if [ "$N_VARS" -lt 1 ]; then
  echo "ERROR: could not read a valid .mv line from $MERGED_PLA"
  exit 1
fi

# 4) config with one module and N_VARS plain variables
V_STRING=$(printf 'V%.0s' $(seq 1 "$N_VARS"))
{
  echo "M0 $MERGED_PLA 0"
  echo ""
  echo "M0 $V_STRING"
} > "$NEW_CONFIG"

# 5) calculation on the merged module
echo "==MERGED=="
echo "Merged module has $N_VARS variables"
mpirun -n 1 DDD/main "$NEW_CONFIG" 0 0 2 y

echo "==END=="