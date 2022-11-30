#! /usr/bin/env bash

trap -- '' SIGINT

make

for i in {1..1000} ; do
  ./bolos
  apid=$(pgrep -f 'A ./bolos')
  #apid=$(pgrep -f '^A$')
  ps -p "$apid" 

  kill -15 "$apid"
  echo "Sent SIGTERM to $apid"

  # wait for "$apid" to die
  while ps -p "$apid" >/dev/null; do
      sleep 1.5
  done
  echo "A dead"

  pgrep -f './bolos'
  if [ $? -eq 0 ]; then
    echo "Bolos still running"
    ps -f | grep bolos
    pkill -f './bolos'
    #pkill './bolos'
    exit 1
  fi
done
