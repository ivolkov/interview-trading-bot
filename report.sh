#!/bin/bash

python ./report.py

tail -n 100 ./info.log > message_text
./list.sh >> message_text
cat message_text | mutt -s "Trades report" -a trades.pdf -- ivolkov

rm message_text
rm trades.pdf
