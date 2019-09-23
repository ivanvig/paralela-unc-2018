FNAME="thd_tiempo.csv"
rm thd_tiempo.csv 2> /dev/null
N_THREADS=(1 2 4 6 8 12 16 32 64)
DEPTH=4000000000
N_ITER=30
for index in ${!N_THREADS[*]}; do
    row=""
    # for i in {1..$((N_ITER-1))}; do
    for ((i=1; i<$N_ITER; i++)); do
        echo "$i iteracion con ${N_THREADS[$index]} threads"
        row=$row$(./jupiter_script.sh ${N_THREADS[index]} $DEPTH | grep -oP "(?<=TIEMPO: ).*")", "
        trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT
    done
    echo "$N_ITER iteracion con ${N_THREADS[$index]} threads"
    row=$row$(./jupiter_script.sh ${N_THREADS[index]} $DEPTH | grep -oP "(?<=TIEMPO: ).*")
    echo $row >> $FNAME
done

