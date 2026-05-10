#!/bin/bash

echo "===== COMPILAR ====="
make clean
make

echo
echo "===== TESTE COM controller 1 0 ====="
./bin/controller 1 0 &
CONTROLLER_PID=$!
sleep 1

time (
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 2 "sleep 5" &
    ./bin/runner -e 3 "sleep 5" &
    ./bin/runner -e 4 "sleep 5" &
    wait
)

./bin/runner -c
./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE COM controller 2 0 ====="
./bin/controller 2 0 &
CONTROLLER_PID=$!
sleep 1

time (
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 2 "sleep 5" &
    ./bin/runner -e 3 "sleep 5" &
    ./bin/runner -e 4 "sleep 5" &
    wait
)

./bin/runner -c
./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE COM controller 4 0 ====="
./bin/controller 4 0 &
CONTROLLER_PID=$!
sleep 1

time (
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 2 "sleep 5" &
    ./bin/runner -e 3 "sleep 5" &
    ./bin/runner -e 4 "sleep 5" &
    wait
)

./bin/runner -c
./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== FIM DOS TESTES ====="