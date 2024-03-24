#!/bin/bash

#SBATCH --job-name=g09-tests
#SBATCH --output=mpi_%j.out
#SBATCH --error=mpi_%j.err
#SBATCH --ntasks=20

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

function die() {
    cat <<EOF
Usage: speedup.sh

Runs the mpi version and compares the results to the expected ones.
EOF

    exit 1
}

function compile() {
    cd mpi
    echo "Compiling mpi version"
    make clean && make|| (echo "Cmpiilation failed" && exit 1)
    cd ..
}

# Parse args

if [[ $# -gt 0 ]]; then
    die
fi
else
    die  
fi

# Cmpiile code

echo "Compiling life3d-mpi"
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
    out8=$name.out8
    err_mpi8=$name.err_mpi8
    myout_mpi8=$name.myout_mpi8
    diff_mpi8=$name.diff_mpi8

    out16=$name.out16
    err_mpi16=$name.err_mpi16
    myout_mpi16=$name.myout_mpi16
    diff_mpi16=$name.diff_mpi16

    out32=$name.out32
    err_mpi32=$name.err_mpi32
    myout_mpi32=$name.myout_mpi32
    diff_mpi32=$name.diff_mpi32

    out64=$name.out64
    err_mpi64=$name.err_mpi64
    myout_mpi64=$name.myout_mpi64
    diff_mpi64=$name.diff_mpi64

    printf "\n$name:\n"

    # Run with mpi version with multiple mpi tasks: 8, 16, 32, 64
    echo "Running mpi version with 8 mpi tasks:"
    srun -n 8 ../mpi/life3d-mpi $(cat $in) > $myout_mpi8 2> $err_mpi8
    diff $out8 $myout_mpi8 > $diff_mpi8

    if [ -s $diff_mpi8 ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_mpi8=$(tail -n 1 $err_mpi8 | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time mpi: $time_mpi8 seconds"

    echo "Running mpi version with 16 mpi tasks:"
    srun -n 16 ../mpi/life3d-mpi $(cat $in) > $myout_mpi16 2> $err_mpi16
    diff $out16 $myout_mpi16 > $diff_mpi16

    if [ -s $diff_mpi16 ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_mpi16=$(tail -n 1 $err_mpi16 | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time mpi: $time_mpi16 seconds"

    echo "Running mpi version with 32 mpi tasks:"
    srun -n 32 ../mpi/life3d-mpi $(cat $in) > $myout_mpi32> $err_mpi32
    diff $out32 $myout_mpi32 > $diff_mpi32

    if [ -s $diff_mpi32 ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_mpi32=$(tail -n 1 $err_mpi32 | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time mpi: $time_mpi32 seconds"

    echo "Running mpi version with 64 mpi tasks:"
    srun -n 64 ../mpi/life3d-mpi $(cat $in) > $myout_mpi64 2> $err_mpi64
    diff $out64 $myout_mpi64 > $diff_mpi64

    if [ -s $diff_mpi64 ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_mpi64=$(tail -n 1 $err_mpi64 | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time mpi: $time_mpi64 seconds"
done

printf "\nFailed $failed/$total\n"

cd ..

if [[ $failed -gt 0 ]]; then
    exit 1
else
    exit 0
fi
