gcc serial.c -o serial `mysql_config --cflags --libs` -Wwrite-strings
./serial
