terminal='gnome-terminal -e'
property='--window-with-profile=NAMEOFTHEPROFILE'
${terminal} "./fbsd -s server1" ${property} 
${terminal} -e "./fbc -s server1 -u user1" ${property} 
${terminal} -e "./fbc -s server1 -u user2" ${property} 
# ${terminal} -e "./fbc -s server1 -u user3" ${property} 