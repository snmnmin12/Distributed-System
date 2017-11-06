#terminal='gnome-terminal -e'
#property='--window-with-profile=NAMEOFTHEPROFILE'
./fbsd -s localhost -l -i 4 &
export pid1=$!
./fbsd -s localhost -i 5 &
export pid2=$!
./fbsd -s localhost -i 6 &
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
