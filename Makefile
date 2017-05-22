all: Server Client Test

Server: server.o list.o
	g++ -o server.exe server.o list.o -lpthread
	
Client: client.o network_layer.o list.o
	g++ -o client.exe client.o network_layer.o list.o -lpthread

Test: test_app.o network_layer.o list.o	
	g++ -o test_app.exe test_app.o network_layer.o list.o -lpthread
	
clean:
	rm -rf *.o *.exe