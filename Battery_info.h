#ifndef BATTERY_INFO_H

#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define FALSE 0
#define TRUE 1

#define POWERTYPE_CMD			0xA2
#define FULLCAPATIY_CMD			0x10
#define REMAININGCAPATIY_CMD	0x0F
#define AVERAGETIMETOEMPTY_CMD	0x12


const char connect_arr[] = { 0xAA, 0x2A, 0x98, 0x01, 0x01, 0xA1 };


class BatteryInterface
{
public:
	BatteryInterface(const char *dev);
	~BatteryInterface();

	/*
	连接电池设备接口
	*/
	bool connect_battery();

	/*
	获取电池类型接口
	*/
	unsigned int get_powertype();//return value:
								//			0x3000:External Power
								//			0xB000:Charging
								//			0xA000:Battery

	/*
	获取剩余电量百分比数据接口
	*/
	unsigned int get_percentage_of_remaining_power();

	/*
	获取电池可使用时间接口(minute)
	*/
	unsigned int get_remaining_time();


private:

	int  fd;
	int connect_battery_flag;

	void set_speed(int fd, int speed);
	/*
	*@brief   设置串口数据位，停止位和效验位
	*@param  fd     类型  int  打开的串口文件句柄*
	*@param  databits 类型  int 数据位   取值 为 7 或者8*
	*@param  stopbits 类型  int 停止位   取值为 1 或者2*
	*@param  parity  类型  int  效验类型 取值为N,E,O,,S
	*/
	int set_parity(int fd, int databits, int stopbits, int parity);

	/**
	*@breif 打开串口
	*/
	int opendev(const char *dev);

	//unsigned int get_fullcapatity();

	//unsigned int get_remainingcapatity();
	pthread_mutex_t RW_mutex;


};

#endif 
