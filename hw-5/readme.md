# Этап подготовки
Создайте папку bin для исполняемых файлов.
```
mkdir bin
cd bin
```
# Запуск севера
Для сборки и запуска сервера используется следющая команда:
```
g++ ../server.cpp ../protocol.cpp ../entity.cpp -o server -lenet && ./server
```