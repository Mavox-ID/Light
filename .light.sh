set -e

cd "$(dirname "$0")"

mkdir -p bin

gcc -o bin/script util/script.c -g -Wall -Wextra -O2 -pthread -ldl -rdynamic

taskset -c 0-$(($(nproc)-1)) \
nice -n 10 \
ionice -c 2 -n 6 \
bin/script util/start.script options="$*" \
|| \
taskset -c 0-$(($(nproc)-1)) \
nice -n 10 \
ionice -c 2 -n 6 \
bin/script util/start.script options="build-fix"
