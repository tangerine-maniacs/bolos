#! /usr/bin/env bash

trap '' SIGINT
tail -f /var/log/syslog >& /dev/null

make
./bolos

for i in {1..1000} ; do
  apid=$(pgrep -f 'A ./bolos')

  kill -TERM "$apid"
  echo "Sent SIGTERM to $apid"

  # wait for "$apid" to die
  while kill -0 "$apid" 2>/dev/null; do
      sleep 0.1
  done
  echo "A dead"

  pgrep -f './bolos'
  if [ $? -eq 0 ]; then
    echo "Bolos still running"
    exit 1
  fi
done
