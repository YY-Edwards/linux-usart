/*
* battery_test.cpp
*
* Created: 2017/07/24
* Author: EDWARDS
*/
#include "Battery_info.h"
#include <string>
#define Battery 		0xA000
#define Charging 		0xB000
#define External_Power 	0x3000

std::string STTY_DEV = "/dev/ttyS2";
//std::string STTY_DEV = "/dev/ttySP0";

/*
*@breif  main()
*/

int main(int argc, char *argv[])
{
	BatteryInterface *my_battery = new BatteryInterface(STTY_DEV.c_str());//根据实际硬件初始化设备接口

	int return_err = 0;
	int temp = 0;
	unsigned  int type =0xA000;
	unsigned  int percent =100;
	unsigned  int type_err_counter =0;
	//unsigned int fdata_err_counter =0;
	//unsigned int Rdata_err_counter =0;
	
	int counter = 0;

	//首先连接电池
	return_err = my_battery->connect_battery();
	//sleep(1);
	if (return_err == true)
	{
		fprintf(stderr, "power connect success\n");
		while (type != External_Power){
			//以下接口刷新频率均为1HZ,且以下三个接口是线程安全接口（含互斥锁）
			//获取电源类型/工作状态
			temp = my_battery->get_powertype();
			sleep(2);
			switch (temp)
			{
			case 0x3000:
				fprintf(stderr, "\r\npower type is : External Power\n");//使用外接电源
				type = 0x3000;
				break;
			case 0xB000:
				fprintf(stderr, "\r\npower type is : Charging\n");//充电状态
				type = 0xB000;
				break;
			case 0xA000:
				fprintf(stderr, "\r\npower type is : Battery\n");//电池单独供电，即是放电状态
				type = 0xA000;
				break;
			default:
				fprintf(stderr, "\r\nerr power type is : 0x%04x\n", temp);
				type_err_counter++;
				//type = 0xA000;
				break;

			}

			//获取剩余电量的百分比(外接电源供电时：100%)
			//测试发现，在充电或者放电状态下，剩余电量的百分比有出现突然为0%，或者小于1%的现象，概率为2%
			//建议在UI显示之前，先做数据判断，与上一次获取的值作比较，差值较大则忽略本次获取的数据。
			//另外建议状态值及百分比比值用缓冲队列缓冲数据。
			
			if(type == External_Power)percent =100;
			else{
				percent = my_battery->get_percentage_of_remaining_power();
			}
			fprintf(stderr, "Remaining power is : %d \n", percent);
			
			
			//sleep(2);

			//获取还可以工作多少分钟（经测试（4节电池），数字随机性大，非线性变化）
			//temp = my_battery->get_remaining_time();
			//fprintf(stderr, "Remaining time is : %4d min\r\n", temp);
			sleep(1);
			counter++;
			
			if(type_err_counter!=0)
				fprintf(stderr, "type_err_counter : %4d \r\n", type_err_counter);
			if(my_battery->get_no_data_err_count()!=0)
				fprintf(stderr, "no_data_err_count : %4d \r\n", my_battery->get_no_data_err_count());
			if(my_battery->get_percent_err_count()!=0)
				fprintf(stderr, "percent_err_count : %4d \r\n", my_battery->get_percent_err_count());
			
			
			
		}

	}
	else
		fprintf(stderr, "power connect err\n"); 

	delete my_battery;

}


