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
	BatteryInterface *my_battery = new BatteryInterface(STTY_DEV.c_str());//����ʵ��Ӳ����ʼ���豸�ӿ�

	int return_err = 0;
	int temp = 0;
	int counter = 0;

	//�������ӵ��
	return_err = my_battery->connect_battery();
	sleep(1);
	if (return_err == true)
	{
		fprintf(stderr, "power connect success\n");
		while (counter < 100){
			//���½ӿ�ˢ��Ƶ�ʾ�Ϊ1HZ,�����������ӿ����̰߳�ȫ�ӿڣ�����������
			//��ȡ��Դ����/����״̬
			temp = my_battery->get_powertype();
			sleep(1);
			switch (temp)
			{
			case 0x3000:
				fprintf(stderr, "power type is : External Power\n");//ʹ����ӵ�Դ
				break;
			case 0xB000:
				fprintf(stderr, "power type is : Charging\n");//���״̬
				break;
			case 0xA000:
				fprintf(stderr, "power type is : Battery\n");//��ص������磬���Ƿŵ�״̬
				break;
			default:
				fprintf(stderr, "err power type is : 0x%04x\n", temp);
				break;

			}

			//��ȡʣ������İٷֱ�(��ӵ�Դ����ʱ��100%)
			//���Է��֣��ڳ����߷ŵ�״̬�£�ʣ������İٷֱ��г���ͻȻΪ0%������С��1%�����󣬸���Ϊ2%
			//������UI��ʾ֮ǰ�����������жϣ�����һ�λ�ȡ��ֵ���Ƚϣ���ֵ�ϴ�����Ա��λ�ȡ�����ݡ�
			//���⽨��״ֵ̬���ٷֱȱ�ֵ�û�����л������ݡ�
			temp = my_battery->get_percentage_of_remaining_power();
			fprintf(stderr, "Remaining power is : %d %\n", temp);
			sleep(1);

			//��ȡ�����Թ������ٷ��ӣ������ԣ�4�ڵ�أ�����������Դ󣬷����Ա仯��
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


