terminal='gnome-terminal -e'
property='--window-with-profile=NAMEOFTHEPROFILE'
${terminal} "./fbsd -s server2" ${property} 
${terminal} -e "./fbc -s server2 -u user4" ${property} 
# ${terminal} -e "./fbc -s server2 -u user5" ${property} 
# ${terminal} -e "./fbc -s server2 -u user6" ${property} 