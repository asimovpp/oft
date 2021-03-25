#!/usr/bin/env bash

[[ -z ${1} ]] && echo "Executable not provided" && exit 1

nm -P "${1}" | awk '$2 == "T" {print $1}'
