#define CSV_PATH				"/home/root/ID.csv"
#define DB_PATH					"/home/root/ID.db"
#define Version					"1.0"
#define MODEMDEVICE				"/dev/ttyUSB0"

int check_db(char *Roll_No, char *Name, char *ID_Number, char *Phone_Number_1, char *Phone_Number_2, char *IN_Time, char *IN_Time_Update, char *OUT_Time, char *OUT_Time_Update, char *IN_SMS_status, char *OUT_SMS_status, char *state);

typedef struct {
	int iModemFd;
	struct termios options;
	speed_t baudrate ;
	pthread_mutex_t UARTLock;
} UARTDriverInfo_t;

struct Student_details  
{ 
    int id; 
    char Roll_No[10];
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
UARTDriverInfo_t UARTDriverInfo;

int student_count=0;
int reader_state = 0;

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

if(z>=5)
{
if(z==6)
strcpy(Student[student_count].Roll_No,data);
else if(z==7)
strcpy(Student[student_count].Name,data);
else if(z==8)
strcpy(Student[student_count].ID_Number,data);
else if(z==9)
strcpy(Student[student_count].Phone_Number_1,data);
else if(z==10)
{
strcpy(Student[student_count].Phone_Number_2,data);
strcpy(Student[student_count].IN_SMS_status,"0");
strcpy(Student[student_count].OUT_SMS_status,"0");
strcpy(Student[student_count].IN_Time,"0");
strcpy(Student[student_count].IN_Time_Update,"0");
strcpy(Student[student_count].OUT_Time,"0");
strcpy(Student[student_count].OUT_Time_Update,"0");
z=5;
student_count++;
}
}
}
i++;
}    

fclose (file_state);
//student_count--;
printf("Student Count = %d\n",student_count);
}

int csv_db()
{
int i = 0;
for(i=0;i<student_count;i++)
check_db(Student[i].Roll_No,Student[i].Name,Student[i].ID_Number,Student[i].Phone_Number_1,Student[i].Phone_Number_2,Student[i].IN_Time,Student[i].IN_Time_Update,Student[i].OUT_Time,Student[i].OUT_Time_Update,Student[i].IN_SMS_status,Student[i].OUT_SMS_status,"write");
}

int csv_update(char ID_Number[25], char timestamp[25])
{

int i = 0;
for(i=0;i<=student_count;i++)
{

if(strcmp(Student[i].ID_Number,ID_Number) == 0)
{

	if(reader_state == 1)
	{
		if(strcmp(Student[i].IN_Time,"0") == 0)
		{
			strcpy(Student[i].IN_Time,timestamp);
		}
		else
		{
			strcpy(Student[i].IN_Time_Update,timestamp);
		}
	}
	else if(reader_state == 2)
	{
		if(strcmp(Student[i].OUT_Time,"0") == 0)
		{
			strcpy(Student[i].OUT_Time,timestamp);
		}
		else
		{
			strcpy(Student[i].OUT_Time_Update,timestamp);
		}

	}

check_db(Student[i].Roll_No, Student[i].Name,Student[i].ID_Number,Student[i].Phone_Number_1,Student[i].Phone_Number_2,Student[i].IN_Time,Student[i].IN_Time_Update,Student[i].OUT_Time,Student[i].OUT_Time_Update,Student[i].IN_SMS_status,Student[i].OUT_SMS_status,"update");
}
}
}

int check_db(char *Roll_No,char *Name, char *ID_Number, char *Phone_Number_1, char *Phone_Number_2, char *IN_Time, char *IN_Time_Update, char *OUT_Time, char *OUT_Time_Update, char *IN_SMS_status, char *OUT_SMS_status, char *state)
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
		sprintf(sql, "create table if not exists Student_Details(%s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT, %s TEXT);", "Roll_No", "Name", "ID_Number", "Phone_Number_1", "Phone_Number_2", "IN_Time", "IN_Time_Update", "OUT_Time", "OUT_Time_Update", "IN_SMS_status", "OUT_SMS_status");
	}
	else if(state == "write")
	{
		sprintf(sql, "INSERT INTO Student_Details VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s');", Roll_No, Name, ID_Number, Phone_Number_1, Phone_Number_2, IN_Time, IN_Time_Update, OUT_Time, OUT_Time_Update, IN_SMS_status, OUT_SMS_status);
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

int db_init()
{
	return check_db("Roll_No", "Name", "ID_Number", "Phone_Number_1", "Phone_Number_2", "IN_Time", "IN_Time_Update", "OUT_Time", "OUT_Time_Update", "IN_SMS_status", "OUT_SMS_status","init");
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
