server_app: ./server/server.cpp ./src/*.cpp
	gcc -o server_app ./server/server.cpp ./src/*.cpp

client_app: ./client/client.cpp ./src/*.cpp
	gcc -o client_app ./client/client.cpp ./src/*.cpp