#!/bin/bash

QEMU=qemu-system-p16

# 1. Initialize counters
PASS=0
FAIL=0
TOTAL=0

echo "--- Starting test suite ---"

for test_file in */*.S; do
    base_name=$(basename "$test_file" .S)
    
    p16-none-elf-as "$test_file" -o "${base_name}.o" -g
    p16-none-elf-ld "${base_name}.o" -o "${base_name}.elf" -g
    
    $QEMU -machine p16-testboard -bios "${base_name}.elf" -monitor none -display none
    
    if [ $? -eq 0 ]; then
        echo -e "[\e[32mPASS\e[0m] $test_file"
        PASS=$((PASS+1))
    else
        echo -e "[\e[31mFAIL\e[0m] $test_file"
        FAIL=$((FAIL+1))
    fi
    
    TOTAL=$((TOTAL+1))
    
done
rm *.elf *.o

# 2. Print Final Summary
echo ""
echo "========================================="
echo "             FINAL SUMMARY               "
echo "========================================="
echo -e "Total tests executed: \e[1m$TOTAL\e[0m"
echo -e "Tests PASSED:         \e[32m$PASS\e[0m"

# If there are failures, show them in red. If not, show them in green!
if [ $FAIL -eq 0 ]; then
    echo -e "Tests FAILED:         \e[32m0\e[0m 🎉"
    echo "========================================="
    echo -e "\e[32mSUCCESS: Your architecture is completely validated!\e[0m"
else
    echo -e "Tests FAILED:         \e[31m$FAIL\e[0m ⚠️"
    echo "========================================="
    echo -e "\e[31mWARNING: There are still bugs to fix in QEMU.\e[0m"
    exit 1
fi
