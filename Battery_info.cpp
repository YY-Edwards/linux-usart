/*
* Battery_info.cpp
*
* Created: 2017/07/24
* Author: EDWARDS
*/
#include "Battery_info.h"
int speed_arr_temp[] = {
	B921600, B460800, B230400, B115200, B57600, B38400, B19200,
	B9600, B4800, B2400, B1200, B300,
};

int name_arr_temp[] = {
	921600, 460800, 230400, 115200, 57600, 38400, 19200,
	9600, 4800, 2400, 1200, 300,
};



BatteryInterface::BatteryInterface(const char *dev)
{
	connect_battery_flag = 0;
	struct termios tio;
	fd = opendev(dev);

	//if (fd > 0){

	//	if (tcgetattr(fd, &tio) < 0)
	//	{
	//		printf("get setting error!\r");
	//		return;
	//	}


	//	tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	//	tio.c_oflag &= ~OPOST;
	//	tio.c_cflag |= CLOCAL | CREAD;
	//	tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	//	tio.c_cc[VMIN] = 2;
	//	tio.c_cc[VTIME] = 0;
	//	cfsetospeed(&tio, B9600);            // 115200 baud
	//	cfsetispeed(&tio, B9600);            // 115200 baud
	//	tcsetattr(fd, TCSANOW, &tio);
	//	tcflush(fd, TCIFLUSH);

		if (fd > 0) {
			set_speed(fd, 9600);
			}
			else {
			fprintf(stderr, "Error opening %s: %s\n", dev, strerror(errno));
			return;
			}

			if (set_parity(fd, 8, 1, 'N') == FALSE) {
			fprintf(stderr, "Set Parity Error\n");
			close(fd);
			}
		//init mutex
		pthread_mutex_init(&RW_mutex, NULL);
	//}
	//else{

	//	fprintf(stderr, "Error opening %s: %s\n", dev, strerror(errno));
	//	return;

	//}

}

BatteryInterface::~BatteryInterface()
{
	int err = 0;
	if (fd != 0){
		err= close(fd);
		pthread_mutex_destroy(&RW_mutex);
		//fprintf(stderr, "close : %d\n", err);
	}
}

int BatteryInterface::opendev(const char *dev)
{
	char *device = (char *)dev;
	int fd = open(device, O_RDWR);         //| O_NOCTTY | O_NDELAY
	if (-1 == fd) { /*设置数据位数*/
		perror("Can't Open Serial Port");
		return -1;
	}
	else
		return fd;

}

void BatteryInterface::set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);

	for (i = 0; i < sizeof(speed_arr_temp) / sizeof(int); i++) {
		if (speed == name_arr_temp[i])	{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr_temp[i]);
			cfsetospeed(&Opt, speed_arr_temp[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
				perror("tcsetattr fd1");
			return;
		}
		tcflush(fd, TCIOFLUSH);
	}

	if (i == 12){
		printf("\tSorry, please set the correct baud rate!\n\n");
	}
}

int BatteryInterface::set_parity(int fd, int databits, int stopbits, int parity)
{

	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*设置数据位数*/ {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return (FALSE);
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S':
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return (FALSE);
	}
	/* 设置停止位*/
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 20; // 2 seconds
	options.c_cc[VMIN] = 0;

	options.c_lflag &= ~(ECHO | ICANON);

	options.c_oflag &= ~OPOST;

	options.c_cflag |= CLOCAL | CREAD;

	options.c_iflag &= ~(BRKINT | ISTRIP | ICRNL | IXON);//解决二进制0x0d、0x11、0x13等被丢失问题

	tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);


}

bool BatteryInterface::connect_battery()
{
	int nread;			/* Read the counts of data */
	char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);

	write(fd, connect_arr, sizeof(connect_arr));
	nread = read(fd, buff, sizeof(buff));
	if (nread <= 0){
		perror("err read data:");
		fprintf(stderr, "read length:%d\n", nread);
		return false;
	}
	else{
		//fprintf(stderr, "buff[0]: 0x%02x, buff[1]: 0x%02x\n", buff[0], buff[1]);
		if (buff[0] == 0x01 && buff[1] == 0x01){
			connect_battery_flag = 1;
			return true;
		}
		else
			return false;
	}
}

unsigned int BatteryInterface::get_powertype()
{
	if (!connect_battery_flag)return 0xFFFF;
	int nread;			/* Read the counts of data */
	unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	static unsigned int return_value = 0;
	int cmd = 0;

	cmd = POWERTYPE_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
	nread = read(fd, buff, 2);
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){//切换的时候会响应缓慢
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		return (return_value);//返回上一次的值
	}
	else{

		//fprintf(stderr, "buff[0]: 0x%02x, buff[1]:0x%02x\n", buff[0], buff[1]);
		return_value = ((buff[1] << 8) | buff[0]);
		return (return_value);

	}

}

unsigned int BatteryInterface::get_percentage_of_remaining_power()
{	
	if (!connect_battery_flag)return 0xFFFF;
	int nread;			/* Read the counts of data */
	unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	float temp = 0;
	static int return_value = 0;
	unsigned int full_capatity_value = 0;
	unsigned int remaining_capatity_value = 0;
	int cmd = 0;

	cmd = FULLCAPATIY_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
	nread = read(fd, buff, 2);
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		return (return_value);//返回上一次的值
	}
	else{
		full_capatity_value = ((buff[1] << 8) | buff[0]);
		cmd = REMAININGCAPATIY_CMD;
		pthread_mutex_lock(&RW_mutex);
		write(fd, &cmd, 1);
		nread = read(fd, &buff[2], 2);
		pthread_mutex_unlock(&RW_mutex);
		if (nread <= 0){
			//perror("err read data:");
			//fprintf(stderr, "read length:%d\n", nread);
			return (return_value);//返回上一次的值
		}
		else{
			remaining_capatity_value = ((buff[3] << 8) | buff[2]);
			//fprintf(stderr, "full_capatity_value: 0x%04x, %d mah\n", full_capatity_value, full_capatity_value);
			//fprintf(stderr, "remaining_capatity_value: 0x%04x, %d mah\n", remaining_capatity_value, remaining_capatity_value);

			temp = (float)remaining_capatity_value / (float)full_capatity_value;
			//fprintf(stderr, "remaining_percentage: %f %\n", temp*100);
			return_value = (int)(temp * 100);
			return return_value;

		}

	}

}

unsigned int BatteryInterface::get_remaining_time()
{
	if (!connect_battery_flag)return 0xFFFF;
	int nread;			/* Read the counts of data */
	unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	static unsigned int return_value = 0;
	unsigned int average_time_to_empty = 0;
	int cmd = 0;

	cmd = AVERAGETIMETOEMPTY_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
	nread = read(fd, buff, 2);
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		return (return_value);//返回上一次的值
	}
	else{
		average_time_to_empty = ((buff[1] << 8) | buff[0]);
		//fprintf(stderr, "average_time_to_empty : %4d min\n", average_time_to_empty);
		return_value = average_time_to_empty;
		return return_value;

	}

}
