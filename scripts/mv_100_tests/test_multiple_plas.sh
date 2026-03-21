#!/usr/bin/env bash
set -e

OUTPUT_FILE=test_outputs/test_stats.txt

CONFIGS=(
  "1 10 7 3"
)

for config in "${CONFIGS[@]}"; do
  read -r MIN_VAL MAX_VAL DEPTH NUM_STATES <<< "$config"

  GLOBAL_ID=0
  > "$OUTPUT_FILE"

  echo "Cleaning old test data..."
  rm -f test_outputs/script_output_divider_*.txt
  rm -rf ../load_files/divider_test/divider_test_*

  echo "--- Starting Batch: Interval $MIN_VAL-$MAX_VAL, Depth $DEPTH ---"

  for run in $(seq 1 2); do
    RANDOM_SEED=$(od -An -N4 -tu4 /dev/urandom | awk '{print $1 % 2000000000}')
    PLA="../load_files/divider_test/divider_test_$GLOBAL_ID"

    # 1. Header line
    echo "=== Running experiment number $GLOBAL_ID (Run $run of Config $MIN_VAL-$MAX_VAL) ===" | tee -a "$OUTPUT_FILE"
    
    # 2. Generator Info line
    echo "Running tree generator: $DEPTH $MIN_VAL $MAX_VAL $MIN_VAL $MAX_VAL $NUM_STATES $RANDOM_SEED $PLA" | tee -a "$OUTPUT_FILE"

    # 3. Execute generator and capture its output (Modules, Variables, etc.)
    # Make sure mv_tree_generator prints "Number of modules: X", etc.
    ./mv_tree_generator "$DEPTH" "$MIN_VAL" "$MAX_VAL" "$MIN_VAL" "$MAX_VAL" "$NUM_STATES" "$RANDOM_SEED" "$PLA" | tee -a "$OUTPUT_FILE"

    # 4. Explicit Seed line (in case generator doesn't print it clearly)
    # echo "Seed: $RANDOM_SEED" | tee -a "$OUTPUT_FILE"

    # Run the experiment
    ./run_experiment.sh "$PLA" "$GLOBAL_ID"
    
    echo "" | tee -a "$OUTPUT_FILE"
    GLOBAL_ID=$((GLOBAL_ID + 1))
  done

  # Generate Excel
  EXCEL_NAME="Report_${MIN_VAL}-${MAX_VAL}_Depth${DEPTH}_States${NUM_STATES}.xlsx"
  python3 time_parser.py "test_outputs" "$OUTPUT_FILE" "$EXCEL_NAME"
done

echo "All batches completed."