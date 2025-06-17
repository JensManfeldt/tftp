# tftp
A small tftp I have written. The should be compliant with RFC 1350.

# Build and run:
Running 'make' should just build and run the server. Normally the tftp default port would be 69 but when running 'make' the server will serve on localhost (127.0.0.1) port 8888 to avoid needing sudo.

# Running test
Running test can be done by running 'sudo make test' the test requires sudo because the test uses unshare to isolate the network.
