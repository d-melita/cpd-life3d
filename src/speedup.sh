#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

function die() {
    cat <<EOF
Usage: test [version]

    - version: serial or omp or mpi

Runs tests in test directory for specified version.
EOF

    exit 1
}

function compile() {
    cd $version
    make life3d || (echo "Compilation failed" && exit 1)
    cd ..
}

# Parse args

if [[ $# -gt 1 ]]; then
    die
fi

if [[ $# -eq 0 ]]; then
    version=$(cat ../version);
else
    if [[ $1 != serial ]] && [[ $1 != omp ]] && [[ $1 != mpi ]]; then
        echo "Invalid version. Must be serial, omp or mpi."
        die
    fi

    version=$1
fi

# Compile code

echo "Compiling life3d"
compile

# Run tests

total=0
failed=0

printf "\nRunning tests\n"
cd tests || exit 1
for file in $(ls -- *.in); do
    total=$((total+1))

    name="${file%.*}"
    in=$name.in
    out=$name.out
    err_serial=$name.err_serial
    err_omp=$name.err_omp
    myout_serial=$name.myout_serial
    myout_omp=$name.myout_omp
    diff_serial=$name.diff_serial
    diff_omp=$name.diff_omp

    printf "\n$name:\n"

    # Run with serial version
    ../serial/life3d $(cat $in) > $myout_serial 2> $err_serial
    diff $out $myout_serial > $diff_serial

    if [ -s $diff_serial ]; then
        echo -e "$RED serial failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN serial success$ENDCOLOR"
    fi

    # Calculate time
    time_serial=$(tail -n 1 $err_serial | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time serial: $time_serial seconds"

    # Run with omp version
    ../omp/life3d $(cat $in) > $myout_omp 2> $err_omp
    diff $out $myout_omp > $diff_omp

    if [ -s $diff_omp ]; then
        echo -e "$RED omp failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN omp success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_omp=$(tail -n 1 $err_omp | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time omp: $time_omp seconds"

    if [[ $time_omp == 0.0 ]]; then
        echo "Cannot calculate speedup. Time omp is 0 seconds."
        speedup="0"
        continue
    else
        speedup=$(echo "scale=7; $time_serial / $time_omp" | bc)
    fi

    echo "Speedup: $speedup"
done

printf "\nFailed $failed/$total\n"

cd ..

if [[ $failed -gt 0 ]]; then
    exit 1
else
    exit 0
fi
