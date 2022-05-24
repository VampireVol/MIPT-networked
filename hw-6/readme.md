# Этап подготовки
Создайте папки bin для исполняемых файлов серверов.
```
mkdir lobby/bin
mkdir agario/bin
mkdir cars/bin
```
# Запуск лобби-сервера и агента
Для сборки и запуска лобби-сервера и агента используются следующие командыЖ
```
g++ lobby/lobby-server.cpp lobby/protocol.cpp -o lobby/bin/lobby-server -lenet
g++ lobby/agent.cpp lobby/protocol.cpp -o lobby/bin/agent -lenet

./lobby/bin/lobby-server
./lobby/bin/agent
```
# Запуск северов
Для сборки и запуска серверов agario и cars используется следющие команды:
```
g++ -std=c++17 agario/server.cpp agario/protocol.cpp -o agario/bin/server -lenet
g++ cars/server.cpp cars/protocol.cpp cars/entity.cpp -o cars/bin/server -lenet

./agario/bin/server
./cars/bin/server
```
Для информации сервера можно запустить с набором аргументов
```
./agario/bin/server [port] [aiSize] [minStartRadius] [maxStartRadius] [weightLoss] [speedModif]
./cars/bin/server [port] [forward_accel] [break_accel] [speed_rotation]
```
Пример запуска серверов с аргументами
```
./agario/bin/server 10131 10 0.1 1.5 0.9 1.0
./cars/bin/server 10131 12.0 3.0 0.3
```
Запуск клиентов с набором аргументов из cmd
```
./agario/x64/Debug/hw-6.exe [port] [speedModif]
./cars/x64/Debug/hw-6.exe [port] [forward_accel] [break_accel] [speed_rotation]
```