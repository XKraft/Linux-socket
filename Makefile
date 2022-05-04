server_app: ./server/server.cpp ./src/*.cpp 
	g++ -o server_app ./server/server.cpp ./src/*.cpp -lpthread

client_app: ./client/client.cpp ./src/protocol.cpp ./src/myStr.cpp
	gcc -o client_app ./client/client.cpp ./src/protocol.cpp ./src/myStr.cpp -lpthread