#!/bin/bash

sed -i '/new nonce set to/d' ./info.log
sed -i '/curl_easy_perform() return error (No error)/d' ./info.log
sed -i '/invalid character detected inside JSON string/d' ./info.log
sed -i '/data is corrupted/d' ./info.log
sed -i '/server return corrupted data/d' ./info.log
