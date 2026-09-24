#!/bin/bash

# Function to calculate the polynomial
calculate_polynomial() {
    local power=$1
    local result=0
    local a=1
    for ((i = 0; i <= power; i++)); do
        result=$((result + a))
        a=$((a * $2))
    done
    echo "$result"
}

module_count=$(calculate_polynomial "$1" "$2")
parent_count=$(calculate_polynomial $(($1 - 1)) "$2")

# echo "$module_count"
# echo "$parent_count"

#remove ../ when working in build/ directory
output_file="../../load_files/conf_files/experiment_map.conf"

# Truncate the file
> "$output_file"
#$((RANDOM % 45)) output count insted of 45
for ((i = 0; i < $module_count; i++)); do
    echo "M$i ../load_files/conf_files/$3 0" >> "$output_file"
done

echo "" >> "$output_file"

for ((i = 0; i < $module_count; i++)); do
    echo -n "M$i " >> "$output_file"
    for ((j = 1; j <= $2; j++)); do
        if [ "$i" -lt $parent_count ]; then
            echo -n "M$(($j + $2 * $i))" >> "$output_file"
        else
            echo -n "V" >> "$output_file"
        fi
    done
    echo "" >> "$output_file"
done

# echo "Done."
