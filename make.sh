#!/bin/sh

# update code
git pull origin dev

# ticker
cd ticker

rm -rf ./bin
rm -rf ./obj

mkdir bin
mkdir bin/Release
mkdir obj
mkdir obj/Release
mkdir obj/Release/api
mkdir obj/Release/api/jsmn

gcc -O3 -Wall -c ../api/api.c 			-o obj/Release/api/api.o
gcc -O3 -Wall -c ../api/api_parser.c		-o obj/Release/api/api_parser.o
gcc -O3 -Wall -c ../api/api_parser_routines.c	-o obj/Release/api/api_parser_routines.o
gcc -O3 -Wall -c ../api/api_routines.c		-o obj/Release/api/api_routines.o
gcc -O3 -Wall -c ../api/api_nonce.c		-o obj/Release/api/api_nonce.o
gcc -O3 -Wall -c ../api/api_sign.c		-o obj/Release/api/api_sign.o
gcc -O3 -Wall -c ../api/jsmn/jsmn.c		-o obj/Release/api/jsmn/jsmn.o
gcc -O3 -Wall -c conf.c				-o obj/Release/conf.o
gcc -O3 -Wall -c main.c				-o obj/Release/main.o

g++ -o bin/Release/ticker -s -lcrypto -lcurl -lconfig \
	obj/Release/api/api.o \
	obj/Release/api/api_parser.o \
	obj/Release/api/api_parser_routines.o \
	obj/Release/api/api_routines.o \
	obj/Release/api/api_nonce.o \
	obj/Release/api/api_sign.o \
	obj/Release/api/jsmn/jsmn.o \
	obj/Release/conf.o \
	obj/Release/main.o



# trader
cd ../trader

rm -rf ./bin
rm -rf ./obj

mkdir bin
mkdir bin/Release
mkdir obj
mkdir obj/Release
mkdir obj/Release/api
mkdir obj/Release/api/jsmn

gcc -O3 -Wall -c ../api/api.c 			-o obj/Release/api/api.o
gcc -O3 -Wall -c ../api/api_parser.c		-o obj/Release/api/api_parser.o
gcc -O3 -Wall -c ../api/api_parser_routines.c	-o obj/Release/api/api_parser_routines.o
gcc -O3 -Wall -c ../api/api_routines.c		-o obj/Release/api/api_routines.o
gcc -O3 -Wall -c ../api/api_nonce.c		-o obj/Release/api/api_nonce.o
gcc -O3 -Wall -c ../api/api_sign.c		-o obj/Release/api/api_sign.o
gcc -O3 -Wall -c ../api/jsmn/jsmn.c		-o obj/Release/api/jsmn/jsmn.o
gcc -O3 -Wall -c conf.c				-o obj/Release/conf.o
gcc -O3 -Wall -c exchange.c			-o obj/Release/exchange.o
gcc -O3 -Wall -c ipc.c				-o obj/Release/ipc.o
gcc -O3 -Wall -c main.c				-o obj/Release/main.o
gcc -O3 -Wall -c tmr.c				-o obj/Release/tmr.o

g++ -o bin/Release/trader -s -lcrypto -lcurl -lconfig \
	obj/Release/api/api.o \
	obj/Release/api/api_parser.o \
	obj/Release/api/api_parser_routines.o \
	obj/Release/api/api_routines.o \
	obj/Release/api/api_nonce.o \
	obj/Release/api/api_sign.o \
	obj/Release/api/jsmn/jsmn.o \
	obj/Release/conf.o \
	obj/Release/exchange.o \
	obj/Release/ipc.o \
	obj/Release/main.o \
	obj/Release/tmr.o
