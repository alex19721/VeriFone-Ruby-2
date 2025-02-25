gcc prn.c -o prn `mysql_config --cflags --libs` -Wwrite-strings
./prn
