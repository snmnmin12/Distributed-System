#host1 where master node located
host1="lenss-comp1.cse.tamu.edu"
host2="lenss-comp3.cse.tamu.edu"
host3="lenss-comp4.cse.tamu.edu"

./fbsd -z ${host3} -y ${host2} -x ${host1} -m -l -i 0 &
export pid1=$!
./fbsd -z ${host3} -y ${host2} -x ${host1} -m -i 1 &
export pid2=$!
./fbsd -z ${host3} -y ${host2} -x ${host1} -l -i 2 &
export pid3=$!
./fbsd -z ${host3} -y ${host2} -x ${host1} -i 3 &
export pid4=$!

echo "__________________"
echo "Press Enter to kill all processes started by this script..."
echo "Server PIDs:"
echo $pid1
echo $pid2
echo $pid3
echo $pid4

echo "-------------------------------------------------------------------------"
read input
kill $pid1
kill $pid2
kill $pid3
kill $pid4
