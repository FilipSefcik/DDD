#!/usr/bin/env bash
set -e

OUTPUT_FILE=test_outputs/test_stats.txt

CONFIGS=(
  "1 10 5 6"
  "11 20 5 3"
  "21 30 5 2"
)

SEEDS=(
  86220372
  1608629100
  525832857
  638580178
  754509009
  1307792562
  666914346
  829731821
  1937119320
  63624270
  154740963
  1875912736
  1979379792
  772158537
  520786946
  979335055
  1813449714
  261976376
  1050635140
  550413515
  1777972467
  1480589933
  1603873195
  661034077
  1433400319
  693375399
  418556060
  903481228
  1124000185
  1923539500
  1512321747
  53499989
  932020472
  1031439063
  1511187628
  955659491
  1607908423
  28825313
  1779179354
  1320708505
  1925311014
  9872261
  1190216606
  1150757021
  99134172
  110648053
  522496810
  499789344
  856970383
  637063272
  270465810
  851448862
  1695992358
  646738954
  397155442
  1002343897
  1912826649
  1585954794
  545229928
  62920531
  552466043
  1470418825
  737758940
  840061216
  617420961
  495083461
  1966680408
  623024524
  131640687
  812845965
  44670144
  1709142161
  207114089
  397980196
  318722398
  644358068
  1907017997
  491439916
  136557637
  216085820
  1560770947
  1299782268
  1209549117
  1305847060
  548360981
  1308499438
  145655975
  177135181
  1150377018
  723061477
  1871398064
  1212631429
  1564820190
  1493967616
  1813462391
  204063912
  517419570
  1388813807
  1660451796
  1297962058
)

for config in "${CONFIGS[@]}"; do
  read -r MIN_VAL MAX_VAL NUM_STATES DEPTH <<< "$config"

  GLOBAL_ID=0
  > "$OUTPUT_FILE"

  echo "Cleaning old test data..."
  rm -f test_outputs/script_output_divider_*.txt
  rm -rf ../load_files/divider_test/divider_test_*

  echo "--- Starting Batch: Interval $MIN_VAL-$MAX_VAL, Depth $DEPTH ---"

  for RANDOM_SEED in "${SEEDS[@]}"; do
    PLA="../load_files/divider_test/divider_test_$GLOBAL_ID"

    # 1. Header line
    echo "=== Running experiment number $GLOBAL_ID (Run $run of Config $MIN_VAL-$MAX_VAL) ===" | tee -a "$OUTPUT_FILE"
    
    # 2. Generator Info line
    echo "Running tree generator: $DEPTH $MIN_VAL $MAX_VAL $MIN_VAL $MAX_VAL $NUM_STATES $RANDOM_SEED $PLA" | tee -a "$OUTPUT_FILE"

    # 3. Execute generator and capture its output (Modules, Variables, etc.)
    # Make sure mv_tree_generator prints "Number of modules: X", etc.
    ./mv_tree_generator "$DEPTH" "$MIN_VAL" "$MAX_VAL" "$MIN_VAL" "$MAX_VAL" "$NUM_STATES" "$RANDOM_SEED" "$PLA" | tee -a "$OUTPUT_FILE"

    # ./tree_generator "$DEPTH" "$MIN_VAL" "$MAX_VAL" "$MIN_VAL" "$MAX_VAL" "$RANDOM_SEED" "$PLA" | tee -a "$OUTPUT_FILE"
    # 4. Explicit Seed line (in case generator doesn't print it clearly)
    # echo "Seed: $RANDOM_SEED" | tee -a "$OUTPUT_FILE"

    # Run the experiment
    ./run_experiment.sh "$PLA" "$GLOBAL_ID"
    
    echo "" | tee -a "$OUTPUT_FILE"
    GLOBAL_ID=$((GLOBAL_ID + 1))
  done

  # Generate Excel
  EXCEL_NAME="Report_${MIN_VAL}-${MAX_VAL}_Depth${DEPTH}_States${NUM_STATES}_TRUE_DENSITY.xlsx"
  python3 time_parser.py "test_outputs" "$OUTPUT_FILE" "$EXCEL_NAME"
done

echo "All batches completed."