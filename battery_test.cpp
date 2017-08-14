/*
* battery_test.cpp
*
* Created: 2017/07/24
* Author: EDWARDS
*/
#include "Battery_info.h"
#include <string>
std::string STTY_DEV = "/dev/ttyS1";
//std::string STTY_DEV = "/dev/ttySP0";

/*
*@breif  main()
*/

int main(int argc, char *argv[])
{
	BatteryInterface *my_battery = new BatteryInterface(STTY_DEV.c_str());//根据实际硬件初始化设备接口

	int return_err = 0;
	int temp = 0;
	int counter = 0;

	//首先连接电池
	return_err = my_battery->connect_battery();
	sleep(1);
	if (return_err == true)
	{
		fprintf(stderr, "power connect success\n");
		while (counter < 100){
			//以下接口刷新频率均为1HZ,且以下三个接口是线程安全接口（含互斥锁）
			//获取电源类型/工作状态
			temp = my_battery->get_powertype();
			sleep(1);
			switch (temp)
			{
			case 0x3000:
				fprintf(stderr, "power type is : External Power\n");//使用外接电源
				break;
			case 0xB000:
				fprintf(stderr, "power type is : Charging\n");//充电状态
				break;
			case 0xA000:
				fprintf(stderr, "power type is : Battery\n");//电池单独供电，即是放电状态
				break;
			default:
				fprintf(stderr, "err power type is : 0x%04x\n", temp);
				break;

			}

			//获取剩余电量的百分比(外接电源供电时：100%)
			//测试发现，在充电或者放电状态下，剩余电量的百分比有出现突然为0%，或者小于1%的现象，概率为2%
			//建议在UI显示之前，先做数据判断，与上一次获取的值作比较，差值较大则忽略本次获取的数据。
			//另外建议状态值及百分比比值用缓冲队列缓冲数据。
			temp = my_battery->get_percentage_of_remaining_power();
			fprintf(stderr, "Remaining power is : %d %\n", temp);
			sleep(1);

			//获取还可以工作多少分钟（经测试（4节电池），数字随机性大，非线性变化）
			temp = my_battery->get_remaining_time();
			fprintf(stderr, "Remaining time is : %4d min\r\n", temp);
			sleep(1);
			counter++;
		}

	}
	else
		fprintf(stderr, "power connect err\n"); 

	delete my_battery;

}


