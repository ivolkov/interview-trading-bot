#!/bin/sh

./stop.sh

rm -f ~/bot/ticker_btc/ticker_btc
rm -f ~/bot/trader0/trader0
rm -f ~/bot/trader1/trader1
rm -f ~/bot/trader2/trader2
rm -f ~/bot/trader3/trader3
rm -f ~/bot/trader4/trader4
rm -f ~/bot/trader5/trader5
rm -f ~/bot/trader6/trader6
rm -f ~/bot/trader7/trader7
rm -f ~/bot/trader8/trader8
rm -f ~/bot/trader9/trader9
rm -f ~/bot/trader10/trader10
rm -f ~/bot/trader11/trader11
rm ~/bot/watch.sh
rm ~/bot/clean_logs.sh
rm ~/bot/list.sh
rm ~/bot/start.sh
rm ~/bot/stop.sh
rm ~/bot/report.sh

cp ./ticker/bin/Release/ticker ~/bot/ticker_btc/ticker_btc
cp ./trader/bin/Release/trader ~/bot/trader0/trader0
cp ./trader/bin/Release/trader ~/bot/trader1/trader1
cp ./trader/bin/Release/trader ~/bot/trader2/trader2
cp ./trader/bin/Release/trader ~/bot/trader3/trader3
cp ./trader/bin/Release/trader ~/bot/trader4/trader4
cp ./trader/bin/Release/trader ~/bot/trader5/trader5
cp ./trader/bin/Release/trader ~/bot/trader6/trader6
cp ./trader/bin/Release/trader ~/bot/trader7/trader7
cp ./trader/bin/Release/trader ~/bot/trader8/trader8
cp ./trader/bin/Release/trader ~/bot/trader9/trader9
cp ./trader/bin/Release/trader ~/bot/trader10/trader10
cp ./trader/bin/Release/trader ~/bot/trader11/trader11
cp ./watch.sh ~/bot/watch.sh
cp ./clean_logs.sh ~/bot/clean_logs.sh
cp ./list.sh ~/bot/list.sh
cp ./start.sh ~/bot/start.sh
cp ./stop.sh ~/bot/stop.sh
cp ./report.sh ~/bot/report.sh
cp ./report.py ~/bot/report.py
