#!/usr/bin/env bash

pgrep -u "$USER" bolos | xargs -n1 kill
