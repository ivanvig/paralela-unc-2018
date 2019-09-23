FNAME="thd_tiempo.csv"
rm thd_tiempo.csv 2> /dev/null
# N_THREADS=(1 2 4 6 8 12 16 32 64)
N_THREADS=6
DEPTH=(1 10 100 1000 10000 100000 1000000 10000000 100000000 1000000000)
N_ITER=30
for index in ${!DEPTH[*]}; do
    row=""
    # for i in {1..$((N_ITER-1))}; do
    for ((i=1; i<$N_ITER; i++)); do
        echo "$i iteracion con ${DEPTH[$index]} depth"
        row=$row$(./jupiter_script.sh $N_THREADS ${DEPTH[index]} | grep -oP "(?<=TIEMPO: ).*")", "
        trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT
    done
    echo "$N_ITER iteracion con ${DEPTH[$index]} depth"
    row=$row$(./jupiter_script.sh $N_THREADS ${DEPTH[index]} | grep -oP "(?<=TIEMPO: ).*")
    echo $row >> $FNAME
done

