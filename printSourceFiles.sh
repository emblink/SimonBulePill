#!/bin/bash

# Find all .c files recursively from the current directory and print each on a new line
find . -type f -name "*.c" | while read -r file; do
    echo "$file"
done
