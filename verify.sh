#!/bin/bash

# File containing the list of paths
FILE_LIST="libgfxrender.txt"

# Read each line from the file
while IFS= read -r FILE_PATH; do

    # Check if the file exists
    if [[ ! -f "$FILE_PATH" ]]; then
        echo "MISSING: $FILE_PATH"
    else
        echo "FOUND: $FILE_PATH"
    fi
done < "$FILE_LIST"
