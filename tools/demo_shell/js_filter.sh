#!/bin/bash

cat $1 | grep  'js' | awk -F '/' '{print $NF}' | sort -u
