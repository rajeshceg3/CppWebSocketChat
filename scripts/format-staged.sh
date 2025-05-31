#!/bin/sh
#
# This script formats staged C++ files (.cpp, .h) using clang-format.
# It's intended to be used as a pre-commit hook.
#
# To use this as a pre-commit hook:
# 1. Ensure clang-format is installed and in your PATH.
# 2. Make this script executable: chmod +x scripts/format-staged.sh
# 3. Create a symbolic link in your .git/hooks directory:
#    ln -s ../../scripts/format-staged.sh .git/hooks/pre-commit

# Check if clang-format is installed
if ! command -v clang-format > /dev/null; then
    echo "clang-format not found. Please install it and ensure it's in your PATH."
    exit 1
fi

# Get list of staged C++ files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM "*.cpp" "*.h" "*.hpp" "*.c" "*.cc")

if [ -z "$STAGED_FILES" ]; then
    exit 0 # No C++ files to format
fi

echo "Formatting staged C++ files..."

# Format each file
for FILE in $STAGED_FILES; do
    if [ -f "$FILE" ]; then # Check if file exists (it should, as it's staged)
        clang-format -i "$FILE"
        git add "$FILE" # Re-stage the formatted file
    fi
done

echo "Formatting complete."
exit 0
