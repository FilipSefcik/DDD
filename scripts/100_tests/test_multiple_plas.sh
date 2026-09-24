#!/usr/bin/env bash
#
# batch_run.sh
# Usage: ./batch_run.sh

set -e

OUTPUT_FILE=test_outputs/test_stats.txt
# Clear or initialize the output file
> "$OUTPUT_FILE"

# Global counter to keep track of total experiments (0 to 299)
GLOBAL_ID=0

# Define the three test configurations: "min_val max_val depth"
CONFIGS=(
  "1 10 5"
  # "1 10 9"
  # "11 20 4"
  # "21 30 3"
)

for config in "${CONFIGS[@]}"; do
  # Split the config string into variables
  read -r MIN_VAL MAX_VAL DEPTH <<< "$config"

  echo "--- Starting Batch: Interval $MIN_VAL-$MAX_VAL, Depth $DEPTH ---" | tee -a "$OUTPUT_FILE"

  # Run each configuration 100 times
  for run in $(seq 1 100); do
    
    # Generate a random seed for each unique run
    RANDOM_SEED=$(od -An -N4 -tu4 /dev/urandom | awk '{print $1 % 2000000000}')
    
    # Define the path for the PLA file using the global ID
    PLA="../load_files/divider_test/divider_test_$GLOBAL_ID"

    echo "=== Running experiment number $GLOBAL_ID (Run $run of Config $MIN_VAL-$MAX_VAL) ===" | tee -a "$OUTPUT_FILE"

    # Run tree_generator with the dynamic parameters
    # Parameters: depth min_val max_val min_val max_val seed path
    echo "Running tree generator: $DEPTH $MIN_VAL $MAX_VAL $MIN_VAL $MAX_VAL $RANDOM_SEED $PLA" | tee -a "$OUTPUT_FILE"
    
    ./tree_generator "$DEPTH" "$MIN_VAL" "$MAX_VAL" "$MIN_VAL" "$MAX_VAL" "$RANDOM_SEED" "$PLA" | tee -a "$OUTPUT_FILE"

    # Execute the experiment script
    ./run_experiment.sh "$PLA" "$GLOBAL_ID"
    
    echo "" | tee -a "$OUTPUT_FILE"

    # Increment global ID so next run has a unique file name/index
    GLOBAL_ID=$((GLOBAL_ID + 1))
  done
done

echo "All experiments completed."