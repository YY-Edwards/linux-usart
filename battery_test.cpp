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
	BatteryInterface *my_battery = new BatteryInterface(STTY_DEV.c_str());//����ʵ��Ӳ����ʼ���豸�ӿ�

	int return_err = 0;
	int temp = 0;
	unsigned  int type =0xA000;
	unsigned  int percent =100;
	unsigned  int type_err_counter =0;
	//unsigned int fdata_err_counter =0;
	//unsigned int Rdata_err_counter =0;
	
	int counter = 0;

	//�������ӵ��
	return_err = my_battery->connect_battery();
	//sleep(1);
	if (return_err == true)
	{
		fprintf(stderr, "power connect success\n");
		while (type != External_Power){
			//���½ӿ�ˢ��Ƶ�ʾ�Ϊ1HZ,�����������ӿ����̰߳�ȫ�ӿڣ�����������
			//��ȡ��Դ����/����״̬
			temp = my_battery->get_powertype();
			sleep(2);
			switch (temp)
			{
			case 0x3000:
				fprintf(stderr, "\r\npower type is : External Power\n");//ʹ����ӵ�Դ
				type = 0x3000;
				break;
			case 0xB000:
				fprintf(stderr, "\r\npower type is : Charging\n");//���״̬
				type = 0xB000;
				break;
			case 0xA000:
				fprintf(stderr, "\r\npower type is : Battery\n");//��ص������磬���Ƿŵ�״̬
				type = 0xA000;
				break;
			default:
				fprintf(stderr, "\r\nerr power type is : 0x%04x\n", temp);
				type_err_counter++;
				//type = 0xA000;
				break;

			}

			//��ȡʣ������İٷֱ�(��ӵ�Դ����ʱ��100%)
			//���Է��֣��ڳ����߷ŵ�״̬�£�ʣ������İٷֱ��г���ͻȻΪ0%������С��1%�����󣬸���Ϊ2%
			//������UI��ʾ֮ǰ�����������жϣ�����һ�λ�ȡ��ֵ���Ƚϣ���ֵ�ϴ�����Ա��λ�ȡ�����ݡ�
			//���⽨��״ֵ̬���ٷֱȱ�ֵ�û�����л������ݡ�
			
			if(type == External_Power)percent =100;
			else{
				percent = my_battery->get_percentage_of_remaining_power();
			}
			fprintf(stderr, "Remaining power is : %d \n", percent);
			
			
			//sleep(2);

			//��ȡ�����Թ������ٷ��ӣ������ԣ�4�ڵ�أ�����������Դ󣬷����Ա仯��
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


