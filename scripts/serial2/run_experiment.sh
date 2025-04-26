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

# cd ..
# git add .
# git checkout serial
# make
# cd build

# append all output (stdout+stderr) into script_output.txt
exec >> output_"$PLA_NAME".txt 2>&1

# --- generate experiment_map.conf dynamically ---
CONFIG=../load_files/modules/Experiments/experiment_map.conf

# Truncate or create the file
> "$CONFIG"

# 1) module definitions M0 … M$VCOUNT
for i in $(seq 0 $VCOUNT); do
  echo "M$i ../load_files/modules/Experiments/${PLA_NAME}.pla 0" >> "$CONFIG"
done

# 2) blank line
echo >> "$CONFIG"

# 3) root relationship: M0 followed by all children concatenated
{
  printf "M0 "
  for i in $(seq 1 $VCOUNT); do
    printf "M%d" "$i"
  done
  printf "\n"
} >> "$CONFIG"

# 4) each child Mi gets VCOUNT “V”s
for i in $(seq 1 $VCOUNT); do
  # build a string of $VCOUNT “V”s
  Vs=$(printf 'V%.0s' $(seq 1 $VCOUNT))
  echo "M$i $Vs" >> "$CONFIG"
done


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

# cd ..
# git add .
# git checkout devel
# make
# cd build

### merging ###
#mpirun -n 2 DDD/main "$CONFIG" 1 1 1 n
### merged ###

#cd .. 
#git add .
#git checkout serial
#make
#cd build

# Build merged_map.conf with only M0 and 2*VCOUNT-1 Vs
# MERGED_VCOUNT=$(( 2 * VCOUNT - 1 ))
# #MERGED_VCOUNT=$(( 3 * VCOUNT - 2 ))
# CONFIG=../load_files/modules/serial/"$PLA_NAME"_map.conf

# if [ -f ../merged/M0.pla ]; then
#   mv ../merged/M0.pla ../merged/M0_"$PLA_NAME".pla
# fi

# cat > "$CONFIG" <<EOF
# M0 ../merged/M0_$PLA_NAME.pla 0

# M0 $(printf 'V%.0s' $(seq 1 $MERGED_VCOUNT))
# EOF

# echo "Merged $PLA_NAME"
# # And the two runs on merged_map.conf
# for i in {1..10}; do
#     ./DDD/main "$CONFIG" 0 y
# done
#mpirun -n 2 DDD/main "$CONFIG" 2 1 0 y
