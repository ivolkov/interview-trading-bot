#!/bin/bash

./stop.sh

cd ticker_btc
./ticker_btc &

sleep 10

cd ../trader0
./trader0 &

sleep 10

cd ../trader1
./trader1 &

sleep 10

cd ../trader2
./trader2 &

sleep 10

cd ../trader3
./trader3 &

sleep 10

cd ../trader4
./trader4 &

sleep 10

cd ../trader5
./trader5 &

sleep 10

cd ../trader6
./trader6 &

sleep 10

cd ../trader7
./trader7 &

sleep 10

cd ../trader8
./trader8 &

sleep 10

cd ../trader9
./trader9 &

sleep 10

cd ../trader10
./trader10 &

sleep 10

cd ../trader11
./trader11 &

cd ..

./list.sh

