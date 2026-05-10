#!/bin/bash

rm -f log.txt out.txt erros.txt

echo "=== Teste 1: comando simples ==="
./bin/runner -e 1 "echo ola"

echo
echo "=== Teste 2: redirecionamento stdout ==="
rm -f out.txt
./bin/runner -e 1 "echo ola > out.txt"
echo "Conteudo de out.txt:"
cat out.txt

echo
echo "=== Teste 3: redirecionamento stdin ==="
./bin/runner -e 1 "wc -l < /etc/passwd"

echo
echo "=== Teste 4: redirecionamento stderr ==="
rm -f erros.txt
./bin/runner -e 1 "ls naoexiste 2> erros.txt"
echo "Conteudo de erros.txt:"
cat erros.txt

echo
echo "=== Teste 5: pipe simples ==="
./bin/runner -e 1 "grep root /etc/passwd | wc -l"

echo
echo "=== Teste 6: pipe + redirecionamento ==="
rm -f out.txt
./bin/runner -e 1 "grep root /etc/passwd | wc -l > out.txt"
echo "Conteudo de out.txt:"
cat out.txt

echo
echo "=== Teste 7: stdin + pipe ==="
./bin/runner -e 1 "cat < /etc/passwd | wc -l"