#!/bin/sh
chmod +x loadModule.py unloadModule.py producer consumer 
sudo ./unloadModule.py
sudo ./loadModule.py scull_size=5

./producer 1 0 &
./producer 2 0 &
./producer 3 0 &
./producer 4 0 &
./consumer 1 &
./consumer 2 &
./producer 5 0 &
./producer 6 0 &
./producer 7 0 &
./producer 8 0 &
./producer 9 0 &
./producer 10 0 &
./consumer 3 &
./producer 11 0 &
./producer 12 0 &
./producer 13 0 &
./producer 14 0 &
./producer 15 0 &
./producer 16 0 &
./producer 17 0 &
./producer 18 0 &
./producer 19 0 &
./producer 20 0 &
./producer 21 0 &
./producer 22 0 &
./consumer 4 &
./consumer 5 &
./consumer 6 &
./consumer 7 &
./consumer 8 &
./consumer 9 &
./consumer 10 &
./consumer 11 &
./consumer 12 &
./consumer 13 &
./consumer 14 &
./consumer 15 &

