### Using PXYDRIVE to perform transaction requiring caching. A caching proxy will respond to r1 without any action by the server.
# Source: usermanual.pdf figure 7.
serve s1
generate random-text.txt 10K
fetch f1 random-text.txt s1
delay 100
check f1
request r1 random-text.txt s1
delay 100
check r1 #A caching server responds without r1 without any action by the server
quit