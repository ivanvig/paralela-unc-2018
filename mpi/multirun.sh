BLOCKFNAME="blocking_tiempo.csv"
NONBLOCKFNAME="nonblocking_tiempo.csv"
NPROCS=4
N_ITER=30

rm $BLOCKFNAME 2> /dev/null
rm $NONBLOCKFNAME 2> /dev/null
ulimit -S -s 131072

for ((i=1; i<$N_ITER; i++)); do
    echo "$i iteracion bloqueante"
    val=$(mpirun -np $NPROCS build/mpi_blocking | grep -oP "(?<=Runtime = ).*")
    echo $val >> $BLOCKFNAME
done
for ((i=1; i<$N_ITER; i++)); do
    echo "$i iteracion no bloqueante"
    val=$(mpirun -np $NPROCS build/mpi_nonblocking | grep -oP "(?<=Runtime = ).*")
    echo $val >> $NONBLOCKFNAME
done
