/*
* Battery_info.cpp
*
* Created: 2017/07/24
* Author: EDWARDS
* 修正linux不同版本下的异常问题：
* 1.使能同步读写下的超时机制
* 2.去掉特殊字符
* 3.读写前清空缓冲区数据
* 4.异常数据过滤
* 5.修复电量低于10%时反馈数据异常bug
* 6.修复充电过程中装卸电池后，电量反馈数据异常bug
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
	no_data_err_counter = 0;
	percent_err_counter =0;

	power_type = Battery;

	battery_capatity = 0x0000;

	struct termios tio;
	fd = opendev(dev);
	
	if (fd > 0) {
		set_speed(fd, 9600);
	}
	else {
		fprintf(stderr, "Error opening %s: %s\n", dev, strerror(errno));
		return;
	}

    if (set_parity(fd, 8, 1, 'N') == MC_FALSE) {
		fprintf(stderr, "Set Parity Error\n");
		close(fd);
	}
	
	// if (fd > 0) {
		// if (tcgetattr(fd, &tio) < 0) 
		// { 
			// printf("get setting error!\r");
			// //return 1;
		// }
		// tio.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		// tio.c_oflag &= ~OPOST;
		// tio.c_cflag |= CLOCAL | CREAD;
		// tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

		// tio.c_cc[VMIN] = 20;
		// tio.c_cc[VTIME] = 0;
		// cfsetospeed(&tio, B9600);            // 9600 baud
		// cfsetispeed(&tio, B9600);            // 9600 baud
		// tcsetattr(fd, TCSANOW, &tio);
		// tcflush(fd, TCIFLUSH);
		
		// printf("Init okay!\n");
	// }
	// else {
		// fprintf(stderr, "Error opening %s: %s\n", dev, strerror(errno));
		// //return;
	// }
	
	
	
	//init mutex
	pthread_mutex_init(&RW_mutex, NULL);

}

BatteryInterface::~BatteryInterface()
{
	int err = 0;
	if (fd != 0){
		err= close(fd);
		pthread_mutex_destroy(&RW_mutex);
		fprintf(stderr, "close : %d\n", err);
	}
}

int BatteryInterface::opendev(const char *dev)
{
	char *device = (char *)dev;
	int fd = open(device, O_RDWR);         //| O_NOCTTY | O_NDELAY
	if (-1 == fd) { /*\C9\E8\D6\C3\CA\FD\BE\DDλ\CA\FD*/
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
        return(MC_FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*\C9\E8\D6\C3\CA\FD\BE\DDλ\CA\FD*/ {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
        return (MC_FALSE);
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* \C9\E8\D6\C3Ϊ\C6\E6Ч\D1\E9*/
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* ת\BB\BBΪżЧ\D1\E9*/
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S':
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
        return (MC_FALSE);
	}
	/* \C9\E8\D6\C3ֹͣλ*/
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
        return (MC_FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 50; // 5 seconds
	options.c_cc[VMIN] = 0;//2;//读到数据则返回,否则每个字符最多等待2s
	
	//options.c_lflag &= ~(ECHO | ICANON);
	options.c_lflag &= ~(ECHO | ICANON | ECHOE | ISIG);

	options.c_oflag &= ~OPOST;

	options.c_cflag |= CLOCAL | CREAD;

	options.c_iflag &= ~(BRKINT | ISTRIP | ICRNL | IXON);//\BD\E2\B6\FE\BD\F80d\A1\A20x11\A1\A20x13\B5\BF\BB\B6\AA\BF\CE\CC	

	tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
        return (MC_FALSE);
	}
    return (MC_TRUE);


}

bool BatteryInterface::connect_battery()
{
	int nread;			/* Read the counts of data */
    unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	unsigned int sum_counter =0;
	
	tcflush(fd, TCIOFLUSH);// 读写前先清空缓冲区数据，以免串口读取了数据，但是用户没有读取。对同步读写时需要注意每次读写前后是否清空。
	usleep(500000);
	//printf("read is start\n");
	write(fd, connect_arr, sizeof(connect_arr));
	while(sum_counter < 2){
		nread = read(fd, &buff[sum_counter], (10-sum_counter));
		sum_counter=sum_counter+nread;
		if (nread <= 0){
			no_data_err_counter++;
			return false;
		}
	}
	fprintf(stderr, "buff[0]: 0x%02x, buff[1]: 0x%02x\n", buff[0], buff[1]);
	if (buff[0] == 0x01 && buff[1] == 0x01){
		connect_battery_flag = 1;
		return true;
	}
	else
		return false;
	
	// // while(sum_counter < 256){
		// // nread = read(fd, &buff[sum_counter], (256-sum_counter));
		// // sum_counter=sum_counter+nread;
		// // printf("nread : %d\n", nread);
		// // printf("sum_counter: %d\n", sum_counter);
		// // //tcflush(fd, TCIOFLUSH);	
	// // }
	// // printf("read is end\n");
	// // for(unsigned int i =0; i< 256; i++){
		// // printf("read data is buff[%d]: 0x%x\n", i, buff[i]);
	// // }
	// // exit(0);
	
/*
	if (nread <= 0){
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		no_data_err_counter++;
		return false;
	}
	else{
		//fprintf(stderr, "nread number: %d\n", nread);	
		if(nread <2) {
			nread = read(fd, &buff[1], 1);
			if (nread <= 0){
				//perror("err read data:");
				//fprintf(stderr, "read length:%d\n", nread);
				no_data_err_counter++;
				return false;
			}
			
		}
		fprintf(stderr, "buff[0]: 0x%02x, buff[1]: 0x%02x\n", buff[0], buff[1]);
		if (buff[0] == 0x01 && buff[1] == 0x01){
			connect_battery_flag = 1;
			return true;
		}
		else
			return false;
	}
	*/
}

unsigned int BatteryInterface::get_powertype()
{
	if (!connect_battery_flag)return 0xFFFF;
	int nread;			/* Read the counts of data */
    unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	static unsigned int return_value = 0;
	unsigned int sum_counter =0;
	int cmd = 0;
	
	tcflush(fd, TCIOFLUSH);
	usleep(500000);
	cmd = POWERTYPE_CMD;
	write(fd, &cmd, 1);
	while(sum_counter < 2){
		nread = read(fd, &buff[sum_counter], 2);
		sum_counter=sum_counter+nread;
		if (nread <= 0){
			no_data_err_counter++;
			return (return_value);
		}
	}
	//fprintf(stderr, "buff[0]: 0x%02x, buff[1]:0x%02x\n", buff[0], buff[1]);
	return_value = ((buff[1] << 8) | buff[0]);

	power_type = return_value;//get power type

	return (return_value);
	
/*
	cmd = POWERTYPE_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
	nread = read(fd, buff, 2);
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){//\C7л\BB\B5\C4ʱ\BA\F2\BB\E1\CF\ECӦ\BB\BA\C2\FD
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		no_data_err_counter++;
		return (return_value);//\B7\B5\BB\D8\C9\CFһ\B4ε\C4ֵ
	}
	else{
		
		if(nread <2) {
			nread = read(fd, &buff[1], 1);
			if (nread <= 0){
				//perror("err read data:");
				//fprintf(stderr, "read length:%d\n", nread);
				no_data_err_counter++;
				return (return_value);
			}
			
		}
		//fprintf(stderr, "buff[0]: 0x%02x, buff[1]:0x%02x\n", buff[0], buff[1]);
		return_value = ((buff[1] << 8) | buff[0]);
		return (return_value);

	}
*/
}

unsigned int BatteryInterface::get_percentage_of_remaining_power()
{
    if (!connect_battery_flag)return 0xFFFF;
	int nread;			/* Read the counts of data */
    unsigned char buff[10];		/* Recvice data buffer */
	bzero(buff, 10);
	float temp = 0;
    static unsigned int return_value = 0;
    static unsigned int get_value = 0;
	unsigned int full_capatity_value = 0;
	unsigned int remaining_capatity_value = 0;
	int cmd = 0;
    static unsigned int first_read_flag  = 1;
	unsigned int sum_counter =0;

	tcflush(fd, TCIOFLUSH);
	usleep(500000);
	cmd = FULLCAPATIY_CMD;
	write(fd, &cmd, 1);
	while(sum_counter < 2){
		nread = read(fd, &buff[sum_counter], 2);
		sum_counter=sum_counter+nread;
		if (nread <= 0){
			no_data_err_counter++;
			return (return_value);
		}
	}
	full_capatity_value = ((buff[1] << 8) | buff[0]);
	battery_capatity = full_capatity_value;//get battery capatity
	
	tcflush(fd, TCIOFLUSH);
	usleep(500000);
	sum_counter = 0;//reset
	cmd = REMAININGCAPATIY_CMD;
	write(fd, &cmd, 1);
	while(sum_counter < 2){
		nread = read(fd, &buff[sum_counter+2], 2);
		sum_counter=sum_counter+nread;
		if (nread <= 0){
			no_data_err_counter++;
			return (return_value);
		}
	}
	remaining_capatity_value = ((buff[3] << 8) | buff[2]);
	

	fprintf(stderr, "full_capatity_value: 0x%04x, %d mah\n", full_capatity_value, full_capatity_value);
	fprintf(stderr, "remaining_capatity_value: 0x%04x, %d mah\n", remaining_capatity_value, remaining_capatity_value);

	if (full_capatity_value == 0xFFFF && full_capatity_value == remaining_capatity_value){
		first_read_flag = 1;
		if (power_type == Charging)
			return return_value;
		else
			return 100;
	}
	temp = (float)remaining_capatity_value / (float)full_capatity_value;
	fprintf(stderr, "remaining_percentage: %f \n", temp*100);
	get_value = (unsigned int)(temp * 100);
	 if((!first_read_flag) && (battery_capatity == full_capatity_value)){//不是第一次获取电量，且总容值稳定
		 if (abs((int)get_value - (int)return_value) >= 3){
				percent_err_counter++;
				return return_value;
			}
			else{
				return_value = get_value;
				return return_value;
			}
	  }
	 else{
		first_read_flag = 0;
		return_value = get_value;
		return return_value;
	 }
	
	
/*
	cmd = FULLCAPATIY_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
    nread = read(fd, buff, 2);//
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){
        //perror("1-err read data:");
        //fprintf(stderr, "read length:%d\n", nread);
		no_data_err_counter++;
		return (return_value);//\B7\B5\BB\D8\C9\CFһ\B4ε\C4ֵ
	}
	else{
		
		if(nread <2) {
			nread = read(fd, &buff[1], 1);
			if (nread <= 0){
				//perror("err read data:");
				//fprintf(stderr, "read length:%d\n", nread);
				no_data_err_counter++;
				return (return_value);
			}
			
		}
		
		full_capatity_value = ((buff[1] << 8) | buff[0]);
		cmd = REMAININGCAPATIY_CMD;
		pthread_mutex_lock(&RW_mutex);
		write(fd, &cmd, 1);
		nread = read(fd, &buff[2], 2);
		pthread_mutex_unlock(&RW_mutex);
		if (nread <= 0){
            //perror("2-err read data:");
            //fprintf(stderr, "read length:%d\n", nread);
			no_data_err_counter++;
			return (return_value);//\B7\B5\BB\D8\C9\CFһ\B4ε\C4ֵ
		}
		else{
			
			if(nread <2) {
				nread = read(fd, &buff[3], 1);
				if (nread <= 0){
					//perror("err read data:");
					//fprintf(stderr, "read length:%d\n", nread);
					no_data_err_counter++;
					return (return_value);
				}
			
			}
			
			remaining_capatity_value = ((buff[3] << 8) | buff[2]);
   
            fprintf(stderr, "full_capatity_value: 0x%04x, %d mah\n", full_capatity_value, full_capatity_value);
            fprintf(stderr, "remaining_capatity_value: 0x%04x, %d mah\n", remaining_capatity_value, remaining_capatity_value);

			temp = (float)remaining_capatity_value / (float)full_capatity_value;
            fprintf(stderr, "remaining_percentage: %f \n", temp*100);
            get_value = (unsigned int)(temp * 100);
             if(!first_read_flag){
                    if((get_value>(return_value + 10))||(get_value < (return_value - 10))){
                        percent_err_counter++;
						return return_value;
                    }
                    else{
                        return_value = get_value;
                        return return_value;
                    }
              }
             else{
                first_read_flag = 0;
                return_value = get_value;
                return return_value;
             }
			

		}

	}
	*/

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
	unsigned int sum_counter =0;
	
	tcflush(fd, TCIOFLUSH);
	usleep(500000);
	cmd = AVERAGETIMETOEMPTY_CMD;
	write(fd, &cmd, 1);
	while(sum_counter < 2){
		nread = read(fd, &buff[sum_counter], 2);
		sum_counter=sum_counter+nread;
		if (nread <= 0){
			no_data_err_counter++;
			return (return_value);
		}
	}
	average_time_to_empty = ((buff[1] << 8) | buff[0]);
	//fprintf(stderr, "average_time_to_empty : %4d min\n", average_time_to_empty);
	return_value = average_time_to_empty;
	return return_value;
	
/*
	cmd = AVERAGETIMETOEMPTY_CMD;
	pthread_mutex_lock(&RW_mutex);
	write(fd, &cmd, 1);
	nread = read(fd, buff, 2);
	pthread_mutex_unlock(&RW_mutex);
	if (nread <= 0){
		//perror("err read data:");
		//fprintf(stderr, "read length:%d\n", nread);
		//no_data_err_counter++;
		return (return_value);//\B7\B5\BB\D8\C9\CFһ\B4ε\C4ֵ
	}
	else{
		
		if(nread <2) {
			nread = read(fd, &buff[1], 1);
			if (nread <= 0){
				//perror("err read data:");
				//fprintf(stderr, "read length:%d\n", nread);
				//no_data_err_counter++;
				return (return_value);
			}
			
		}
		average_time_to_empty = ((buff[1] << 8) | buff[0]);
		//fprintf(stderr, "average_time_to_empty : %4d min\n", average_time_to_empty);
		return_value = average_time_to_empty;
		return return_value;

	}
	*/

}
