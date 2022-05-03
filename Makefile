server_app: ./server/server.cpp ./src/*.cpp
	gcc -o server_app ./server/server.cpp ./src/*.cpp

client_app: ./client/client.cpp ./src/protocol.cpp ./src/myStr.cpp
	gcc -o client_app ./client/client.cpp ./src/protocol.cpp ./src/myStr.cpp -lpthread