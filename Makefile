all: proxy.exe terminator.exe

proxy.exe: proxy.o Makefile proxy_threads.o proxy_sockets.o
	g++ -o proxy.exe proxy.o proxy_threads.o proxy_sockets.o

proxy.o: proxy.cpp Makefile proxy_threads.h proxy_sockets.h
	g++ -c -Wall -o proxy.o proxy.cpp

terminator.exe: terminator.o Makefile proxy_threads.o proxy_sockets.o
	g++ -o terminator.exe terminator.o proxy_threads.o proxy_sockets.o

terminator.o: terminator.cpp Makefile proxy_threads.h proxy_sockets.h
	g++ -c -Wall -o terminator.o terminator.cpp

proxy_sockets.o: proxy_sockets.h proxy_sockets.cpp
	g++ -c -Wall -o proxy_sockets.o proxy_sockets.cpp

proxy_threads.o: proxy_threads.h proxy_threads.cpp
	g++ -c -Wall -o proxy_threads.o proxy_threads.cpp

clean:
	rm -f *.exe *.o *.stackdump
