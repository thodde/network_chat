# NetworkProject_CS513


To compile, use "make all" from top level. Be sure to run "make clean" before committing changes to git.

******* Compile by hand ***********

To compile client:
    gcc client.c list.c -o client -lpthread

To compile server:
    gcc server.c list.c -o server -lpthread


# TODO: 
- Add comments to everything
- Add GUI for client
- Could clean up list output for clients (low priority)
- Test using different computers on the same network (not just localhost as the server) low priority
