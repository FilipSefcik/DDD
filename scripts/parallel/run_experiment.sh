#!/usr/bin/env bash
#
# run_experiment.sh
# Usage: ./run_experiment.sh <PLA_NAME> <VCOUNT>
#
#  <PLA_NAME> base name of the .pla file (e.g. "pdc" or "apex1")
#  <VCOUNT>   number of 'V' characters for M1 (M0 will get VCOUNT-1)
# !!! In order to work this file has to be in build/ directory !!!!

set -e

if [ $# -ne 2 ]; then
  echo "Usage: $0 <PLA_NAME> <VCOUNT>" >&2
  exit 1
fi

PLA_NAME=$1
VCOUNT=$2

if (( VCOUNT < 1 )); then
  echo "VCOUNT must be at least 1" >&2
  exit 1
fi

# append all output (stdout+stderr) into script_output.txt
exec >> script_output_"$PLA_NAME".txt 2>&1

# --- experiment_map.conf section ---
CONFIG=../load_files/modules/Experiments/experiment_map.conf

cat > "$CONFIG" <<EOF
M0 ../load_files/modules/Experiments/${PLA_NAME}.pla 0
M1 ../load_files/modules/Experiments/${PLA_NAME}.pla 0

M0 M1$(printf 'V%.0s' $(seq 1 $((VCOUNT-1))))
M1 $(printf 'V%.0s' $(seq 1 $VCOUNT))
EOF

# Suppress the PMIx compression warning
export PMIX_MCA_pcompress_base_silence_warning=1

echo "$PLA_NAME"

# First two runs on experiment_map.conf
for i in {1..16}; do
    echo "Cores used: $i"
    for j in {1..10}; do
    mpirun -n $i DDD/main "$CONFIG" 2 0 0 y
    done
done
#mpirun -n 2 DDD/main "$CONFIG" 2 1 0 y

### merging ###
mpirun -n 2 DDD/main "$CONFIG" 1 1 1 n
### merged ###

# Build merged_map.conf with only M0 and 2*VCOUNT-1 Vs
MERGED_VCOUNT=$(( 2 * VCOUNT - 1 ))
CONFIG=../load_files/modules/merged_map.conf

cat > "$CONFIG" <<EOF
M0 ../merged/M0.pla 0

M0 $(printf 'V%.0s' $(seq 1 $MERGED_VCOUNT))
EOF

echo "Merged $PLA_NAME"
# And the two runs on merged_map.conf
for i in {1..10}; do
    ./DDD/main "$CONFIG" 2 0 0 y
done
#mpirun -n 2 DDD/main "$CONFIG" 2 1 0 y
