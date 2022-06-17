gcc -o tools/myqueue.o -c tools/myqueue.c -g
gcc -o tools/myUnixSocketServer.o -c tools/myUnixSocketServer.c -g
gcc -o openflowAP.exe openflowAP.c tools/myqueue.o tools/myUnixSocketServer.o -g -lpthread
