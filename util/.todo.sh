#!/usr/bin/env bash

cd "$(dirname "$0")/.."

rm -f info/TODO # Delete todo file (if have :>)

grep -RIn \
  --exclude=tags \
  --exclude=.todo.sh \
  --exclude=Makefile \
  --exclude-dir={res,bin,cross,info,root,tag} \
  --binary-files=without-match "TODO" . | awk -F: '
BEGIN {
    sep = "+-----------------------------------------+--------+-------------------------------------------------+"
    print sep
    printf "| %-39s | %-6s | %-47s |\n", "File", "Line", "TODO in File"
    print sep
}
{
    file = $1
    line = $2

    text = $0
    sub(/^[^:]+:[^:]+:/, "", text)
    sub(/^[ \t]+/, "", text)

    printf "| %-39s | %-6s | %-s\n", file, line, text
}
END {
    print sep
}' > info/TODO

# All grep for make todo List
