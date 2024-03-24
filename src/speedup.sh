#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

function die() {
    cat <<EOF
Usage: speedup.sh [n]

    - n: number of mpi_THREADS to use in mpi version

Runs both serial and mpi versions and cmpiares the speedup.
EOF

    exit 1
}

function cmpiile() {
    cd serial
    echo "Cmpiiling serial version"
    make clean && make|| (echo "Cmpiilation failed" && exit 1)
    cd ../mpi
    echo "Cmpiiling mpi version"
    make clean && make|| (echo "Cmpiilation failed" && exit 1)
    cd ..
}

# Parse args

if [[ $# -gt 1 ]]; then
    die
fi

if [[ $# -eq 1 ]]; then
    n=$1
else
    die  
fi

# Cmpiile code

echo "Cmpiiling life3d"
cmpiile

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
    err_mpi=$name.err_mpi
    myout_serial=$name.myout_serial
    myout_mpi=$name.myout_mpi
    diff_serial=$name.diff_serial
    diff_mpi=$name.diff_mpi

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

    # Run with mpi version
    OMP_NUM_THREADS=2 mpirun -np $n ../mpi/life3d-mpi $(cat $in) > $myout_mpi 2> $err_mpi
    diff $out $myout_mpi > $diff_mpi

    if [ -s $diff_mpi ]; then
        echo -e "$RED mpi failed$ENDCOLOR"
        failed=$((failed+1))
    else
        echo -e "$GREEN mpi success$ENDCOLOR"
    fi

    # Calculate time and display speedup
    time_mpi=$(tail -n 1 $err_mpi | grep -oE "[0-9]+(\.[0-9]+)?")
    echo "Time mpi: $time_mpi seconds"

    if [[ $time_mpi == 0.0 ]]; then
        echo "Cannot calculate speedup. Time mpi is 0 seconds."
        continue
    else
	    python3 -c "print('Speedup: ' + str($time_serial/$time_mpi))"
    fi
done

printf "\nFailed $failed/$total\n"

cd ..

if [[ $failed -gt 0 ]]; then
    exit 1
else
    exit 0
fi