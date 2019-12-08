SHAREDFNAME="shared_tiempo.csv"
GLOBALFNAME="global_tiempo.csv"
N_ITER=30

rm $SHAREDFNAME 2> /dev/null
rm $GLOBALFNAME 2> /dev/null

for ((i=1; i<$N_ITER; i++)); do
    echo "$i iteracion"
    val=$(build/main)
    echo $(grep "[0-9]*(\.[0-9]*)*" -oP <<< $val | head -1) >> $GLOBALFNAME
    echo $(grep "[0-9]*(\.[0-9]*)*" -oP <<< $val | tail -1) >> $SHAREDFNAME
done
