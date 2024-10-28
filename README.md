Compiling the proxy:

    make

You might need to run the following to clean out old object files.

    make clean

Running the proxy:

    ./proxy <port_number>

Instead of your browser, you can use the following command-line tool to test your proxy.

    curl -v --proxy http://localhost:5000 http://google.com

Run Tests:

    ../pxedrive24/pxy/pxydrive.py -f ../pxedrive24/s01*.cmd -S 3 -p ./proxy


Suppose you have cloned the Pxedrive repository to the same location that you have cloned the syslab repository. Then, inside your syslab repository, you can run pxedrive to test your proxy as follows:

../pxedrive24/pxy/pxydrive.py -f ../pxedrive24/s01*.cmd -S 3 -p ./proxy

This will spawn an instance of your proxy, listening on a randomly chosen open port, run the specified test (.cmd file) on it.

Your code needs to pass 5 tests, s01*.cmd, …, s05*.cmd.

Handout & Handin
Here.

Same procedure as prflab; handout by making a fork, handin by submittinga pull request.

Acknowledgment
This lab is based on proxylab from the Introduction to Computer Systems course at Carnegie Mellon University.