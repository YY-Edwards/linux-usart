#ifndef BATTERY_INFO_H

#include <stdio.h>      /*��׼�����������*/
#include <stdlib.h>
#include <unistd.h>     /*Unix��׼��������*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*�ļ����ƶ���*/
#include <termios.h>    /*PPSIX�ն˿��ƶ���*/
#include <errno.h>      /*����Ŷ���*/
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
	���ӵ���豸�ӿ�
	*/
	bool connect_battery();

	/*
	��ȡ������ͽӿ�
	*/
	unsigned int get_powertype();//return value:
								//			0x3000:External Power
								//			0xB000:Charging
								//			0xA000:Battery

	/*
	��ȡʣ������ٷֱ����ݽӿ�
	*/
	unsigned int get_percentage_of_remaining_power();

	/*
	��ȡ��ؿ�ʹ��ʱ��ӿ�(minute)
	*/
	unsigned int get_remaining_time();


private:

	int  fd;
	int connect_battery_flag;

	void set_speed(int fd, int speed);
	/*
	*@brief   ���ô�������λ��ֹͣλ��Ч��λ
	*@param  fd     ����  int  �򿪵Ĵ����ļ����*
	*@param  databits ����  int ����λ   ȡֵ Ϊ 7 ����8*
	*@param  stopbits ����  int ֹͣλ   ȡֵΪ 1 ����2*
	*@param  parity  ����  int  Ч������ ȡֵΪN,E,O,,S
	*/
	int set_parity(int fd, int databits, int stopbits, int parity);

	/**
	*@breif �򿪴���
	*/
	int opendev(const char *dev);

	//unsigned int get_fullcapatity();

	//unsigned int get_remainingcapatity();
	pthread_mutex_t RW_mutex;


};

#endif 
