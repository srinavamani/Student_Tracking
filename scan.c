#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h> 

#define Version		"1.0"
#define MODEMDEVICE				"/dev/ttyUSB0"

int ble_status = 0;

typedef struct {
	int iModemFd;
	struct termios options;
	speed_t baudrate ;
	pthread_mutex_t UARTLock;
} UARTDriverInfo_t;

UARTDriverInfo_t UARTDriverInfo;

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

void *uart_reader()
{
	int port_status = openport(MODEMDEVICE);
	int rdlen, byte_count=0;
	char buf[1], ID_data[21], ID_Number[13], ID_Bat_Value[3], ID_Rssi[3];
	if(port_status == 1)
	{
		printf("Port Available\n");

	do{
		rdlen = read(UARTDriverInfo.iModemFd, buf, 1);
		if(rdlen > 0)
		{
			if(buf[0] == ':' && byte_count == 21)
			{
				ID_data[21] = '\0';
				printf("%s\n",ID_data);

				ID_Number[0] = ID_data[0];
				ID_Number[1] = ID_data[1];
				ID_Number[2] = ID_data[2];
				ID_Number[3] = ID_data[3];
				ID_Number[4] = ID_data[4];
				ID_Number[5] = ID_data[5];
				ID_Number[6] = ID_data[6];
				ID_Number[7] = ID_data[7];
				ID_Number[8] = ID_data[8];
				ID_Number[9] = ID_data[9];
				ID_Number[10] = ID_data[10];
				ID_Number[11] = ID_data[11];
				ID_Number[12] = '\0';

				ID_Bat_Value[0] = ID_data[12];
				ID_Bat_Value[1] = ID_data[13];
				ID_Bat_Value[2] = ID_data[14];
				ID_Bat_Value[3] = '\0';

				ID_Rssi[0] = ID_data[18];
				ID_Rssi[1] = ID_data[19];
				ID_Rssi[2] = ID_data[20];
				ID_Rssi[3] = '\0';
				
				printf("ID_Number = %s\nID_Bat_Value = %s\nID_Rssi = %s\n\n",ID_Number, ID_Bat_Value, ID_Rssi);
				byte_count = 0;
			}
			else if(buf[0] == ':')
			{
				byte_count = 0;
			}
			else
			{
				ID_data[byte_count++] = buf[0];
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
