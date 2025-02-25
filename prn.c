#include <stdio.h>
#include "/usr/include/mysql/mysql.h"
#include <termios.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>
int prn_flag=0;


int openUart() {
   int fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
   struct termios oldtio = { 0 };
   struct termios newtio = { 0 };
   tcgetattr(fd, &oldtio);
   //Set the baud rate to 115200
   newtio.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
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

void save_check(char *s, int j, MYSQL *sql) {

    const char str[] ="INSERT INTO `prn_db`.`Data` VALUES ('XX.XX.XX','XX:XX:XX XX','                                            ')";
    char* prn = str;
    unsigned char d[9] = {0};
    unsigned char t[12] = {0};
    int i,k,p=0;
//time
    for (i=0;i<11;i++) t[i]=s[(j-1)*50+29+i]; //Time
    if (s[(j-1)*50+29]==' ') { t[0]='0'; p=1; } // s[(j-1)*50+29]='0';

//date
    if (s[(j-1)*50+20+p]==' ') s[(j-1)*50+20+p]='0';
    for (i=0;i<8;i++) d[i]=s[(j-1)*50+20+i+p]; //Data
    if (d[0]==' ') d[0]='0';



//total correct line
    for (i=0;i<j;i++)
         if (strstr(&s[i*50],"E!a2TOTAL")) {
	    int l=strlen(&s[i*50]);
	    s[i*50+0]=' ';s[i*50+1]=' ';s[i*50+2]=' ';s[i*50+3]=' ';
	    for (k=0;k<=l;k++) {
		s[i*50+41-k]=s[i*50+l-k];
		s[i*50+l-k]=' ';
	    }
    }

    mysql_query(sql, "START STARSACTION");
    for (k=0;k<j;k++) {
	for (i=0;i<8;i++) prn[i+37]=d[i];		//date
	for (i=0;i<11;i++) prn[i+48]=t[i];		//time
	for (i=0;i<40;i++) prn[i+62]=s[k*50+i];	//transaction
	mysql_query(sql, str);
    }
    mysql_query(sql, "COMMIT");
    for (i=0; i<j; i++)  printf ("%s %s %s\n",d,t,&s[i*50]);
}
//-------------------------------------------------------------------------

int main(int argc, char **argv) {

    unsigned char check_str[50*50] = {0};
    int cur_str=0;
    int max_str=0;
    int flag_str=0;

//SQL
    MYSQL *con = mysql_init(NULL);
    unsigned char buffer[1024] = {0};
    int bytes_read;
    int i,j=0;
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
  if (mysql_query(con, "USE prn_db")) {
//Create dvr_db
    if (mysql_query(con, "CREATE DATABASE prn_db")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }
//Create Data
  if (mysql_query(con,"CREATE TABLE `prn_db`.`Data` (`Date` TEXT, `Time` TEXT, `Transaction` TEXT ) ")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
    if (mysql_query(con, "USE prn_db")) {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
    }
  }
    cur_str=0;
    max_str=0;
    flag_str=0;
    i=0;




    while (1) {
        bytes_read = read(fd, &c, 1);
        if (bytes_read > 0) {
           if (c==10 && i!=0) {
		buffer[i]=0;
		if (flag_str==0) {
//		    if (buffer[11]=='T') { 
		    if (strstr(buffer," --- ")) {
//		       for (j=0;j<=i;j++) check_str[j]=buffer[j+11];
//		       printf("%s\n",check_str);
		       flag_str=1;// cur_str=1; max_str=1;
		    }
		i=0;
		} else {
		    for (j=0;j<=i;j++) check_str[cur_str*50+j]=buffer[j+11];
//		    printf("%s\n",check_str+cur_str*50);
		    cur_str++; max_str++;
		    i=0;
		    if (strstr(buffer,"CSH:")) { //Сохраняем чек
			save_check(check_str,max_str, con);
			cur_str=0; max_str=0; i=0; flag_str=0;
		    }
		  }
	   }
            else if (c>=' ') { buffer[i]=c; i++; }
        }
    }

}



/*
MySQL client version: 8.0.41
02/22/25 03:08:05 PM T  Coffee Hot 12oz         1        1.59
02/22/25 03:08:05 PM                               ----------
02/22/25 03:08:05 PM                    Subtotal         1.59
02/22/25 03:08:05 PM                         Tax         0.11
02/22/25 03:08:05 PM                          TOTAL      1.70 
02/22/25 03:08:05 PM                        CASH  $      1.70
02/22/25 03:08:05 PM 
02/22/25 03:08:05 PM 
02/22/25 03:08:05 PM ST#AB123               DR#1 TRAN#1010141
CSH: 1                2/22/25 3:08:05 PM

*/