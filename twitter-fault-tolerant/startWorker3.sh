host1="lenss-comp1.cse.tamu.edu"
host2="lenss-comp3.cse.tamu.edu"
host3="lenss-comp4.cse.tamu.edu"

./fbsd -z ${host3} -y ${host2} -x ${host1} -l -i 7 &
export pid1=$!
./fbsd -z ${host3} -y ${host2} -x ${host1} -i 8 &
export pid2=$!
./fbsd -z ${host3} -y ${host2} -x ${host1} -i 9 &
export pid3=$!

echo "__________________"
echo "Press Enter to kill all processes started by this script..."
echo "Server PIDs:"
echo $pid1
echo $pid2
echo $pid3

echo "-------------------------------------------------------------------------"
read input
kill $pid1
kill $pid2
kill $pid3

#./fbc -s server1 -u user1 
#./fbc -s server1 -u user2"
# ${terminal} -e "./fbc -s server1 -u user3" ${property} 
