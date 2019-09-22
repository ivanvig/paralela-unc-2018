make clean > /dev/null
make > /dev/null
mkfifo fifocom
# exec 3<> /tmp/asdasd
(tail -f fifocom & (echo $!>/tmp/asdasd)) | MAX_DEPTH="$2" OMP_NUM_THREADS="$1" ./jupiter &

pid=$!
echo "uci" >  fifocom
echo "position startpos" >  fifocom
echo "go depth 5" >  fifocom
echo "quit" >  fifocom
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT
wait $pid
kill $(cat /tmp/asdasd)
rm -f fifocom || true
# pkill tail
