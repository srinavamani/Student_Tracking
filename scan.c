#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sqlite3.h>

#define CSV_PATH					"/home/root/ID.csv"
#define DB_PATH					"/home/root/ID.db"
#define Version					"1.0"
#define MODEMDEVICE				"/dev/ttyUSB0"

int callback(void *NotUsed, int argc, char **argv, char **azColName);
int check_db(char *Name, char *ID_Number, char *Phone_Number_1, char *Phone_Number_2, char *IN_Time, char *IN_Time_Update, char *OUT_Time, char *OUT_Time_Update, char *IN_SMS_status, char *OUT_SMS_status, char *state);

int ble_status = 0;

typedef struct {
	int iModemFd;
	struct termios options;
	speed_t baudrate ;
	pthread_mutex_t UARTLock;
} UARTDriverInfo_t;

UARTDriverInfo_t UARTDriverInfo;

struct Student_details  
{ 
    int id; 
    char Name[20]; 
    char ID_Number[25];
    char Phone_Number_1[12];
    char Phone_Number_2[12];
    char IN_Time[15];
    char IN_Time_Update[15];
    char OUT_Time[15];
    char OUT_Time_Update[15];
    char IN_SMS_status[2];
    char OUT_SMS_status[2];
}; 

struct Student_details Student[10000];
int student_count=0;

int csv_access()
{
int i=0,j=0,z=0;
char data[100];
FILE *file_state;
char buffer[350000]; 
   
file_state = fopen (CSV_PATH, "r"); 
if (file_state == NULL) 
{ 
fprintf(stderr, "\nError opening file\n"); 
exit (1); 
} 
fread(buffer,350000,1,file_state);

while(buffer[i] != '\0')
{
if(buffer[i] != ',' && buffer[i] != '\n')
data[j++] = buffer[i];
else
{
data[j]='\0';
j=0;
z++;

if(z>=4)
{
if(z==5)
{
strcpy(Student[student_count].Name,data);
}
else if(z==6)
{
strcpy(Student[student_count].ID_Number,data);
}
else if(z==7)
{
strcpy(Student[student_count].Phone_Number_1,data);
}
else if(z==8)
{
strcpy(Student[student_count].Phone_Number_2,data);
strcpy(Student[student_count].IN_SMS_status,"0");
strcpy(Student[student_count].OUT_SMS_status,"0");
strcpy(Student[student_count].IN_Time,"0");
strcpy(Student[student_count].IN_Time_Update,"0");
strcpy(Student[student_count].OUT_Time,"0");
strcpy(Student[student_count].OUT_Time_Update,"0");

z=4;
student_count++;
}

}
}
i++;
}    

fclose (file_state); 
student_count--;
printf("Student Count = %d\n",student_count);

}

int csv_db()
{
int i = 0;
for(i=0;i<=student_count;i++)
{

//printf("Name - %s\nID - %s\nPH_1 - %s\nPH_2 - %s\nIN - %s\nIN_Update - %s\nOUT - %s\nOUT_Update - %s\nIN_SMS - %s\nOUT_SMS - %s\n",Student[i].Name,Student[i].ID_Number,Student[i].Phone_Number_1,Student[i].Phone_Number_2,Student[i].IN_Time,Student[i].IN_Time_Update,Student[i].OUT_Time,Student[i].OUT_Time_Update,Student[i].IN_SMS_status,Student[i].OUT_SMS_status);
//printf("\n Student_count %d ===========================================================================================\n",i);

check_db(Student[i].Name,Student[i].ID_Number,Student[i].Phone_Number_1,Student[i].Phone_Number_2,Student[i].IN_Time,Student[i].IN_Time_Update,Student[i].OUT_Time,Student[i].OUT_Time_Update,Student[i].IN_SMS_status,Student[i].OUT_SMS_status,"write");

}

}

int csv_update(char ID_Number[25], char timestamp[25])
{

int i = 0;
for(i=0;i<=student_count;i++)
{

if(strcmp(Student[i].ID_Number,ID_Number) == 0)
{

	if(strcmp(Student[i].IN_Time,"0") == 0)
	{
	strcpy(Student[i].IN_Time,timestamp);
	}
	else
	{
	strcpy(Student[i].IN_Time_Update,timestamp);
	}

check_db(Student[i].Name,Student[i].ID_Number,Student[i].Phone_Number_1,Student[i].Phone_Number_2,Student[i].IN_Time,Student[i].IN_Time_Update,Student[i].OUT_Time,Student[i].OUT_Time_Update,Student[i].IN_SMS_status,Student[i].OUT_SMS_status,"update");
}
}
}

int openport(char *dev_name)
{
	UARTDriverInfo.iModemFd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (UARTDriverInfo.iModemFd < 0) {
		printf("%s\n", "Open Error");
		return 0;
	}
	fcntl(UARTDriverInfo.iModemFd, F_SETFL, O_RDWR);
	tcgetattr(UARTDriverInfo.iModemFd, &UARTDriverInfo.options);
	UARTDriverInfo.options.c_cflag |= (CLOCAL | CREAD);
	UARTDriverInfo.options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	UARTDriverInfo.options.c_oflag &= ~OPOST;
	cfmakeraw(&UARTDriverInfo.options);
	UARTDriverInfo.options.c_cc[VMIN] = 1;
	UARTDriverInfo.options.c_cc[VTIME] = 0;
	UARTDriverInfo.baudrate = B115200;
	cfsetospeed(&UARTDriverInfo.options, UARTDriverInfo.baudrate);
	cfsetispeed(&UARTDriverInfo.options, UARTDriverInfo.baudrate);
	tcsetattr(UARTDriverInfo.iModemFd, TCSAFLUSH, &UARTDriverInfo.options);
	sleep(2);
	tcflush(UARTDriverInfo.iModemFd, TCIOFLUSH);
	return 1;
}

int check_db(char *Name, char *ID_Number, char *Phone_Number_1, char *Phone_Number_2, char *IN_Time, char *IN_Time_Update, char *OUT_Time, char *OUT_Time_Update, char *IN_SMS_status, char *OUT_SMS_status, char *state)
{
	sqlite3 *db;
	char *err_msg = 0;
	sqlite3_stmt *res;

	int rc = sqlite3_open(DB_PATH, &db);

	if (rc) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	char sql[1000];

	if(state == "init")
	{
		sprintf(sql, "create table if not exists Student_Details(%s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT);", "Name", "ID_Number", "Phone_Number_1", "Phone_Number_2", "IN_Time", "IN_Time_Update", "OUT_Time", "OUT_Time_Update", "IN_SMS_status", "OUT_SMS_status");
	}
	else if(state == "write")
	{
		sprintf(sql, "INSERT INTO Student_Details VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');", Name, ID_Number, Phone_Number_1, Phone_Number_2, IN_Time, IN_Time_Update, OUT_Time, OUT_Time_Update, IN_SMS_status, OUT_SMS_status);
	}
	else if(state == "update")
	{
		sprintf(sql, "UPDATE Student_Details SET IN_Time = '%s', IN_Time_Update = '%s', OUT_Time = '%s', OUT_Time_Update = '%s', IN_SMS_status = '%s', OUT_SMS_status = '%s' WHERE ID_Number = '%s';", IN_Time, IN_Time_Update, OUT_Time, OUT_Time_Update, IN_SMS_status, OUT_SMS_status, ID_Number);
	}

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

	if (rc != SQLITE_OK ) {

		fprintf(stderr, "SQL error: %s\n", err_msg);

		sqlite3_free(err_msg);        
		sqlite3_close(db);

		return 1;
	} 

	sqlite3_close(db);

	return 0;
}

int read_db(int offset)
{
	sqlite3 *db;
	char *err_msg = 0;
	sqlite3_stmt *res;

	int rc = sqlite3_open(DB_PATH, &db);

	if (rc != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 1;
	}

	char sql[100];

	sprintf(sql, "SELECT Packet FROM Student_Details LIMIT 1 OFFSET %d", offset);
	rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		return 1;
	}

	sprintf(sql, "DELETE FROM Student_Details LIMIT 1 OFFSET %d",offset);
	rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

	if (rc != SQLITE_OK ) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
		return 1;
	}

	sqlite3_close(db);
	return 0;
}


int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	NotUsed = 0;
	int i =0;
	printf("COUNT = %d\n",argc);
	for(i = 0; i < argc; i++) {
		printf("%s\n", argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int LSBOF4BIT(unsigned char hex)
{
char value;
unsigned char hex_data = hex & 0x0f;

if(hex_data >= 0x00 && hex_data <= 0x09)
        value = hex_data+48;
else if(hex_data >= 0x0a && hex_data <= 0x0f)
        value = hex_data+97-10;
else if(hex_data >= 0x0A && hex_data <= 0x0F)
        value = hex_data+65-10;

//printf("%c\n",value);

return value;
}


int MSBOF4BIT(unsigned char hex)
{
char value;
unsigned char hex_data = (hex >> 4) & 0x0f;

if(hex_data >= 0x00 && hex_data <= 0x09)
        value = hex_data+48;
else if(hex_data >= 0x0a && hex_data <= 0x0f)
        value = hex_data+97-10;
else if(hex_data >= 0x0A && hex_data <= 0x0F)
        value = hex_data+65-10;

//printf("%c\n",value);

return value;
}

void *uart_reader()
{
	int port_status = openport(MODEMDEVICE);
	int rdlen, byte_count=0;
	unsigned char hex[1], ID_data[21], ID_Number[30], ID_Bat_Value[3], ID_Rssi[3];
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	time_t ltime;
	struct tm *tm;
int i = 0, j = 0, packet = 0,bytes_to_read = 1;


	if(port_status == 1)
	{
		printf("Port Available\n");
bytes_to_read = 1;

	do{
		rdlen = read(UARTDriverInfo.iModemFd, ID_data, bytes_to_read);
		if(rdlen > 0)
		{
/*
if(bytes_to_read == 21)
{
for(i=0;i<=20;i++)
printf("%02x ",ID_data[i]);
}
printf("\n");
*/

			if(ID_data[0] == 0xa0 && ID_data[1] == 0x13 && ID_data[20] != 0x00)
			{
				ID_data[byte_count] = '\0';
				j=0;
				for(i=7;i<19;i++)
				{
					ID_Number[j++] = MSBOF4BIT(ID_data[i]);
					ID_Number[j++] = LSBOF4BIT(ID_data[i]);
				}

				ID_Number[j] = '\0';
				
				ID_Rssi[0] = '6';
				ID_Rssi[1] = '5';

				printf("ID_Number = %s\n\n",ID_Number);
				ltime=time(NULL);
				tm=localtime(&ltime);
				sprintf(timestamp, "%02d%02d%02d%02d%02d%02d",tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900 - 2000, tm->tm_hour, tm->tm_min, tm->tm_sec);

				//check_db(timestamp, ID_Number, ID_Rssi, "update");
				csv_update(ID_Number, timestamp);

				byte_count = 0;
				memset(ID_data, '\0',21);
				memset(ID_Number, '\0', 30);
			}
			else if(ID_data[0] == 0xa0)
			{
				read(UARTDriverInfo.iModemFd, ID_data, bytes_to_read);
				if(ID_data[0] == 0x13)
				{
					bytes_to_read = 19;
					read(UARTDriverInfo.iModemFd, ID_data, bytes_to_read);
					bytes_to_read = 21;
				}
				memset(ID_data, '\0',21);
				memset(ID_Number, '\0', 30);				
			}

		}
		else
		{
				ble_status = '0';
				printf("PORT OPEN ERROR\n");
				sleep(1);
		}
	} while(1);
	}
	else
	{
		ble_status = '0';
		printf("COM_PORT Open Fail\n");
	}	
}

int main()
{
printf("Student Tracking Project - Version - %s\n",Version);

csv_access();

if(check_db("Name", "ID_Number", "Phone_Number_1", "Phone_Number_2", "IN_Time", "IN_Time_Update", "OUT_Time", "OUT_Time_Update", "IN_SMS_status", "OUT_SMS_status","init") == 0)
{
csv_db();
		pthread_t uart_reader_thread;

		if(pthread_create(&uart_reader_thread, NULL, uart_reader, NULL)) {
			printf("Error creating thread\n");
			return 1;
		}

		if(pthread_join(uart_reader_thread, NULL)) {
			fprintf(stderr, "Error joining uart_reader_thread\n");
			return 2;
		}
}

}
