#!/bin/bash
FILE=$1
if [ -f "$FILE" ]; then
    echo "File exists"
else 
    echo "File does not exist"
fi
