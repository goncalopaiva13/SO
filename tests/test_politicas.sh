#!/bin/bash

echo "===== COMPILAR ====="
make clean
make

echo
echo "===== TESTE 1: FIFO (controller 1 0) ====="
./bin/controller 1 0 &
CONTROLLER_PID=$!
sleep 1

time (
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 1 "sleep 5" &
    sleep 1
    ./bin/runner -e 2 "sleep 1" &
    wait
)

./bin/runner -c
./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 2: ROUND ROBIN (controller 1 1) ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

time (
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 1 "sleep 5" &
    ./bin/runner -e 1 "sleep 5" &
    sleep 1
    ./bin/runner -e 2 "sleep 1" &
    wait
)

./bin/runner -c
./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== FIM DOS TESTES ====="