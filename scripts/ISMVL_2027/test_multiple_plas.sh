#!/usr/bin/env bash
set -e

OUTPUT_FILE=test_outputs/test_stats.txt
SEEDS_FILE=test_outputs/seeds.txt
NUM_SEEDS=10

CONFIGS=(
  "1 3 3 3"
  # "11 20 3 2"
  # "21 30 3 1"
)

mkdir -p test_outputs

# ---- Seed generation -------------------------------------------------------
# Seeds must fit into an int (mv_tree_generator uses std::stoi): 1 .. 2147483647.
# RANDOM is only 15 bits, so two calls are combined into one ~31-bit number.
# Optional: MASTER_SEED=123 ./batch.sh  -> the same 100 seeds every time.
generate_seeds() {
  local -A seen=()
  SEEDS=()
  while [ "${#SEEDS[@]}" -lt "$NUM_SEEDS" ]; do
    local s=$(( (RANDOM << 16) | RANDOM ))
    if [ "$s" -gt 0 ] && [ -z "${seen[$s]}" ]; then   # positive and unique
      seen[$s]=1
      SEEDS+=("$s")
    fi
  done
}

if [ -n "$MASTER_SEED" ]; then
  RANDOM=$MASTER_SEED
fi

generate_seeds
printf '%s\n' "${SEEDS[@]}" > "$SEEDS_FILE"
echo "Generated ${#SEEDS[@]} seeds (saved to $SEEDS_FILE)"

# The same seeds are used for every config, so the configs stay comparable.
for config in "${CONFIGS[@]}"; do
  read -r MIN_VAL MAX_VAL NUM_STATES DEPTH <<< "$config"

  GLOBAL_ID=0
  > "$OUTPUT_FILE"

  echo "Cleaning old test data..."
  rm -f test_outputs/script_output_ISMVL_*.txt
  rm -rf ../load_files/ISMVL_test/ISMVL_test_*

  echo "--- Starting Batch: Interval $MIN_VAL-$MAX_VAL, Depth $DEPTH ---"

  for RANDOM_SEED in "${SEEDS[@]}"; do
    PLA="../load_files/ISMVL_test/ISMVL_test_$GLOBAL_ID"

    # 1. Header line
    echo "=== Running experiment number $GLOBAL_ID (Config $MIN_VAL-$MAX_VAL) ===" | tee -a "$OUTPUT_FILE"

    # 2. Generator Info line
    echo "Running tree generator: $DEPTH $MIN_VAL $MAX_VAL $MIN_VAL $MAX_VAL $NUM_STATES $RANDOM_SEED $PLA" | tee -a "$OUTPUT_FILE"

    # 3. Execute generator and capture its output (Modules, Variables, etc.)
    ./mv_tree_generator "$DEPTH" "$MIN_VAL" "$MAX_VAL" "$MIN_VAL" "$MAX_VAL" "$NUM_STATES" "$RANDOM_SEED" "$PLA" | tee -a "$OUTPUT_FILE"

    # 4. Run the experiment
    ./run_experiment.sh "$PLA" "$GLOBAL_ID"

    echo "" | tee -a "$OUTPUT_FILE"
    GLOBAL_ID=$((GLOBAL_ID + 1))
  done

  # Generate Excel + CSV
  EXCEL_NAME="Report_${MIN_VAL}-${MAX_VAL}_Depth${DEPTH}_States${NUM_STATES}_MV_SI.xlsx"
  python3 time_parser.py "test_outputs" "$OUTPUT_FILE" "$EXCEL_NAME"
done

echo "All batches completed."