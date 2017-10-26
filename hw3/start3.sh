terminal='gnome-terminal -e'
property='--window-with-profile=NAMEOFTHEPROFILE'
${terminal} "./fbsd -s server3" ${property} 
${terminal} -e "./fbc -s server3 -u user7" ${property} 
# ${terminal} -e "./fbc -s server3 -u user8" ${property} 
# ${terminal} -e "./fbc -s server3 -u user9" ${property} 