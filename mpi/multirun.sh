BLOCKFNAME="blocking_tiempo.csv"
NONBLOCKFNAME="nonblocking_tiempo.csv"
MATRIX_SIZE=(400 800 1200 1600 2000)
NPROCS=4
N_ITER=30

rm $BLOCKFNAME 2> /dev/null
rm $NONBLOCKFNAME 2> /dev/null
ulimit -S -s 131072

for dim in ${MATRIX_SIZE[*]}; do
    val=""
    for ((i=1; i<$N_ITER; i++)); do
        echo "$i iteracion bloqueante con matriz $dim x $dim"
        val=$val$(MAX_DIM=$dim mpirun -np $NPROCS build/mpi_blocking | grep -oP "(?<=Runtime = ).*")", "
    done
    echo "$N_ITER iteracion bloqueante con matriz $dim x $dim"
    val=$val$(MAX_DIM=$dim mpirun -np $NPROCS build/mpi_blocking | grep -oP "(?<=Runtime = ).*")
    echo $val >> $BLOCKFNAME

    val=""
    for ((i=1; i<$N_ITER; i++)); do
        echo "$i iteracion no bloqueante con matriz $dim x $dim"
        val=$val$(MAX_DIM=$dim mpirun -np $NPROCS build/mpi_nonblocking | grep -oP "(?<=Runtime = ).*")", "
    done
    echo "$N_ITER iteracion no bloqueante con matriz $dim x $dim"
    val=$val$(MAX_DIM=$dim mpirun -np $NPROCS build/mpi_nonblocking | grep -oP "(?<=Runtime = ).*")
    echo $val >> $NONBLOCKFNAME
done
