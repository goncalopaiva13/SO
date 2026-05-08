#!/bin/bash

echo "===== COMPILAR ====="
make clean
make

echo
echo "===== TESTE 1: consulta com comandos em execução e em espera ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "sleep 10" &
RUN1=$!

sleep 1

./bin/runner -e 2 "sleep 10" &
RUN2=$!

sleep 1

echo
echo "=== Resultado de ./bin/runner -c ==="
./bin/runner -c

wait $RUN1
wait $RUN2

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 2: shutdown com comando ainda em execução ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "sleep 5" &
RUN1=$!

sleep 1

./bin/runner -s &
SHUTDOWN_RUNNER=$!

wait $RUN1
wait $SHUTDOWN_RUNNER
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 3: varios utilizadores ====="
./bin/controller 2 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "sleep 3" &
RUN1=$!

./bin/runner -e 2 "echo ola" &
RUN2=$!

./bin/runner -e 3 "sleep 2" &
RUN3=$!

wait $RUN1
wait $RUN2
wait $RUN3

echo
echo "=== Consulta final ==="
./bin/runner -c

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null
echo

echo "===== TESTE 4: verificar log ====="
echo "Conteudo de log.txt:"
cat log.txt

echo
echo "===== TESTE 5: comando invalido ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "comando_que_nao_existe"

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 6: ficheiro de input inexistente ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "wc -l < ficheiro_inexistente"

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 7: pipe com comando invalido ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "comando_que_nao_existe | wc -l"

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== TESTE 8: pipeline com mais de dois comandos ====="
./bin/controller 1 1 &
CONTROLLER_PID=$!
sleep 1

./bin/runner -e 1 "cat < /etc/passwd | grep root | wc -l"

./bin/runner -s
wait $CONTROLLER_PID 2>/dev/null

echo
echo "===== FIM DOS TESTES EXTRA ====="