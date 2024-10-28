### Using PXYDRIVE to perform transaction requiring concurrent proxy. A serial proxy would be held up waiting for the server to respond to request r1.
# Source: usermanual.pdf figure 6.
serve s1
generate random-text1.txt 2K #File1
generate random-text2.txt 4K #File2
request r1 random-text1.txt s1 # Request r1
request r2 random-text2.txt s1 # Request r2
delay 100
respond r2 # Respond out of order
delay 100
check r2
quit