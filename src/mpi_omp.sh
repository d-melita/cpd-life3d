#!/bin/bash

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
    out=$name.out

    #err_mpi8=$name.err_mpi8
    #myout_mpi8=$name.myout_mpi8
    #diff_mpi8=$name.diff_mpi8

    err_mpi16=$name.err_mpi16
    myout_mpi16=$name.myout_mpi16
    diff_mpi16=$name.diff_mpi16

    #err_mpi32=$name.err_mpi32
    #myout_mpi32=$name.myout_mpi32
    #diff_mpi32=$name.diff_mpi32

    #err_mpi64=$name.err_mpi64
    #myout_mpi64=$name.myout_mpi64
    #diff_mpi64=$name.diff_mpi64

    printf "\n$name:\n"

    # Run with mpi version with multiple mpi tasks: 8, 16, 32, 64
    #echo "Running mpi version with 8 mpi tasks:"
    #srun -n 8 -w lab3p[1,2,4-7,9,10] ../mpi/life3d-mpi $(cat $in) > $myout_mpi8 2> $err_mpi8
    #diff $out $myout_mpi8 > $diff_mpi8

    #if [ -s $diff_mpi8 ]; then
        #echo -e "$RED mpi failed$ENDCOLOR"
        #failed=$((failed+1))
    #else
        #echo -e "$GREEN mpi success$ENDCOLOR"
    #fi
    #cat $err_mpi8

    #sleep 2

    echo "Running mpi version with 16 mpi tasks:"
    OMP_NUM_THREADS=4 srun -n 16 -w lab3p[1,2,4-7,9,10] --cpus-per-task=4 ../mpi/life3d-mpi $(cat $in) > $myout_mpi16 2> $err_mpi16
    diff $out $myout_mpi16 > $diff_mpi16

    if [ -s $diff_mpi16 ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    cat $err_mpi16

    #sleep 2

    #echo "Running mpi version with 32 mpi tasks:"
    #srun -n 32 -w lab3p[1,2,4-7,9,10] ../mpi/life3d-mpi $(cat $in) > $myout_mpi32 2> $err_mpi32
    #diff $out $myout_mpi32 > $diff_mpi32

    #if [ -s $diff_mpi32 ]; then
        #echo -e "$RED mpi failed$ENDCOLOR"
        #failed=$((failed+1))
    #else
        #echo -e "$GREEN mpi success$ENDCOLOR"
    #fi

    #cat $err_mpi32

    #sleep 2

    #echo "Running mpi version with 64 mpi tasks:"
    #srun -n 64 -w lab3p[1,2,4-7,9,10] ../mpi/life3d-mpi $(cat $in) > $myout_mpi64 2> $err_mpi64
    #diff $out $myout_mpi64 > $diff_mpi64

    #if [ -s $diff_mpi64 ]; then
        #echo -e "$RED mpi failed$ENDCOLOR"
        #failed=$((failed+1))
    #else
        #echo -e "$GREEN mpi success$ENDCOLOR"
    #fi

    #cat $err_mpi64
done

printf "\nFailed $failed/$total\n"

cd ..

if [[ $failed -gt 0 ]]; then
    exit 1
else
    exit 0
fi