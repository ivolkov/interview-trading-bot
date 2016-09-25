#!/bin/bash

watch -n 10 'tail -n 15 ./error.log; echo ""; tail -n 15 ./info.log; echo ""; ./list.sh'
