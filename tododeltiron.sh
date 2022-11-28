#! /usr/bin/env bash

make
./bolos

bolosps=$(ps -f | grep "./bolos")
apid=$(echo "$bolosps" | grep "A ./bolos" | sed 's/[ ][ ]*/ /g' | cut -d ' ' -f 2)

# echo "$bolosps"
kill -TERM "$apid"

