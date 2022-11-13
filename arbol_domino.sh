#!/usr/bin/env bash

ps -f | grep "bolos" | grep -v grep | sort | ./Arbol_dominO
