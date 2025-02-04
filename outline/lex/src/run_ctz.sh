#!/bin/bash

# Check if a filename was provided.
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename.ctz>"
    exit 1
fi

FILE="$1"
OUTDIR="./out"

# Create the out directory if it doesn't exist.
if [ ! -d "$OUTDIR" ]; then
    mkdir -p "$OUTDIR"
fi

# Step 1: Generate LLVM IR from the .ctz file.
echo "Compiling $FILE to LLVM IR..."
./ctriz "$FILE" > "$OUTDIR/output.ll" 2>&1

# Optional: Check if output.ll was created.
if [ ! -s "$OUTDIR/output.ll" ]; then
    echo "Error: $OUTDIR/output.ll is empty. Compilation may have failed."
    exit 1
fi

# Step 2: Convert LLVM IR to assembly.
echo "Converting LLVM IR to assembly..."
llc "$OUTDIR/output.ll" -o "$OUTDIR/output.s"
if [ $? -ne 0 ]; then
    echo "Error: llc failed."
    exit 1
fi

# Step 3: Compile the assembly to an executable.
echo "Linking assembly to create executable..."
clang "$OUTDIR/output.s" -o "$OUTDIR/my_program"
if [ $? -ne 0 ]; then
    echo "Error: clang linking failed."
    exit 1
fi

# Step 4: Run the resulting executable.
echo "Running the program..."
"$OUTDIR/my_program"