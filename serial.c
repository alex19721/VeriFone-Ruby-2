#include <stdio.h>
#include "/usr/include/mysql/mysql.h"
#include <termios.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>


int openUart() {
   int fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
   struct termios oldtio = { 0 };
   struct termios newtio = { 0 };
   tcgetattr(fd, &oldtio);
   //Set the baud rate to 115200
   newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = 0; // IGNPAR | ICRNL
   newtio.c_oflag = 0;
   newtio.c_lflag = 0; // ICANON
   newtio.c_cc[VTIME] = 0;
   newtio.c_cc[VMIN] = 16;
//   tcflush(fd, TCIOFLUSH);
   tcsetattr(fd, TCSANOW, &newtio);

   //Set to non-blocking mode, this will be used when reading the serial port
   fcntl(fd, F_SETFL, O_NONBLOCK);
   return fd;
}

void print_buf(char *buf, MYSQL *sql) {
    const char s[] ="INSERT INTO `dvr_db`.`Data` VALUES ('XX.XX.XX','XX:XX:XX','                                            ')";
    char* prn = s;
    int i;
    for (i=0;i<8;i++) prn[i+37]=buf[i];	//date
    for (i=0;i<8;i++) prn[i+48]=buf[i+9];	//time
    for (i=0;i<44;i++) prn[i+59]=buf[i+18];	//transaction
    printf("%s\n",buf);

    mysql_query(sql, "START STARSACTION");
    mysql_query(sql, s);
    mysql_query(sql, "COMMIT");
}


int main(int argc, char **argv) {

//SQL
    MYSQL *con = mysql_init(NULL);
    unsigned char buffer[1024] = {0};
    int bytes_read;
    int i=0;
    char c;
    int fd;

//Uart
  fd=openUart();
  if (fd < 0) {
    //Failed to open the serial port, exit
    printf ("Error to open UART\n");
    return -1;
  } 

//Print ver
    printf("MySQL client version: %s\n", mysql_get_client_info());
//Open connection
  if (mysql_real_connect(con, "localhost", "admin", "123",
          NULL, 0, NULL, 0) == NULL) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
//Open DB
  if (mysql_query(con, "USE dvr_db")) {
//Create dvr_db
    if (mysql_query(con, "CREATE DATABASE dvr_db")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }
//Create Data
  if (mysql_query(con,"CREATE TABLE `dvr_db`.`Data` (`Date` TEXT, `Time` TEXT, `Transaction` TEXT ) ")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
    if (mysql_query(con, "USE dvr_db")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }
  }
    while (1) {
	bytes_read = read(fd, &c, 1);
	if (bytes_read > 0) {
	   if (c==10 && i!=0) { buffer[i]=0; print_buf(buffer+3, con); i=0; }
	    else
	     if (c=='c' && i>48) { buffer[i]=0; print_buf(buffer+3, con); i=1; }
              else 
	       if (c>=' ') { buffer[i]=c; i++; }
	}
    }
}

