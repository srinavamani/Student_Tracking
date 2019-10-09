#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "Msg_Queue.h"
#include <fcntl.h> 
#include <termios.h>
#include <libxml/parser.h>
#include <sqlite3.h>

#define UART_READER 

#define MODEMDEVICE				"/dev/ttyUSB0"
#define DB_PATH					"/tmp/Student_tag.db"
#define VERSION					"1.1"

#define Beacon_Queue_Key 1992
#define Beacon_Data_Length 12
#define RSSI_CHECK 1

#define BAUDRATE B115200
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

char ble_status = '0';
int Device_conf_status = 0;
int COM_OK_COUNT = 0;
const char* message;

int openport(char *dev_name);
int fd=0;

//struct termios oldtp, newtp;
typedef struct {
	int iModemFd;
	struct termios options;
	speed_t baudrate ;
	pthread_mutex_t UARTLock;
} UARTDriverInfo_t;

UARTDriverInfo_t UARTDriverInfo;

int callback(void *, int, char **, char **);

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam)
{
	struct hci_request rq;
	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = ocf;
	rq.cparam = cparam;
	rq.clen = clen;
	rq.rparam = status;
	rq.rlen = 1;
	return rq;
}

int set_interface_attribs(int fd, int speed)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
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

void *uart_reader()
{
	printf("Entered uart_reader\n");
	int port_status = openport(MODEMDEVICE);
	if(port_status == 1)
	{
	printf("Entered uart_reader\n");
		ble_status = '1';
		char rough[21];
		char rough_1[12], gpio_access[100];
		int i = 0, rssi_value = 0, tag_false_count = 0;
		unsigned char buf[1];
		int rdlen;
		char beacons_log[100];
		char rssi[2];
		do {
			rdlen = read(UARTDriverInfo.iModemFd, buf, 1);

			if (rdlen > 0) {
//				printf("%c",buf[0]);

			}
			else
			{
				ble_status = '0';
				printf("PORT OPEN ERROR\n");
				sleep(1);
			}
		} while (1);
	}
	else
	{
		ble_status = '0';
		printf("COM_PORT Open Fail\n");
		while(1)
		{
			sleep(1);
		}
	}
}
/*
int wifi_mac_assigning()
{
	if( access( "/sys/class/net/wlan0/carrier", F_OK ) != -1 ) {
		printf("WIFI Hardware detected.. Yes\n");
		system("cat /sys/class/net/wlan0/address > /home/pi/MAC_ID");
		int i = 0, j = 0;
		char mac_rough[20];
		FILE *fp_mac_id = fopen("/home/pi/MAC_ID", "r");
		if(fp_mac_id != NULL)
		{
			fread(mac_rough,17,1,fp_mac_id);
			fclose(fp_mac_id);

			for(i = 0; i <= 16; i++)
			{
				if(mac_rough[i] != ':')
//					mac[j++] = mac_rough[i];
					mac[j++] = 'A';
			}
			printf("%s\n",mac);
		}
		else
		{
			printf("MAC_ID file not found\n");
		}
		return 0;
	}
	else
	{
		printf("WIFI Hardware detected.. NO\n");
		sleep(30);
		system("sudo reboot");
		return 1;
	}
}
*/
int main()
{
	printf("Welcome to Student tracker.....!\n");
	printf("VERSION - %s\n",VERSION);

		sys_mq_reset(Beacon_Queue_Key);

		pthread_t uart_reader_thread;

#ifdef UART_READER 
		if(pthread_create(&uart_reader_thread, NULL, uart_reader, NULL)) {
			printf("Error creating thread\n");
			return 1;
		}
#endif

		if(pthread_join(uart_reader_thread, NULL)) {
			fprintf(stderr, "Error joining uart_reader_thread\n");
			return 2;
		}
	return 0;
}

/*
 * speedtest-cli
 *
 * sudo apt-get install python-pip
 *
 * sudo pip install speedtest-cli
 *
 */

// http://admin.tturk.in/admin/uploads/firmware/current_wifi/current_firmware.bin
// http://admin.tturk.in/admin/api/request/assignsetting/receiverid/AAAAAAAAAAAA?format=xml

