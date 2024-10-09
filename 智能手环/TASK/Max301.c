#include "Max301.h"
#include "max30102.h"
#include "ui.h"

uint32_t aun_ir_buffer[500];  // IR LED sensor data
int32_t n_ir_buffer_length;   // data length
uint32_t aun_red_buffer[500]; // Red LED sensor data
int32_t n_sp02;               // SPO2 value
int8_t ch_spo2_valid;         // indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;         // heart rate value
int8_t ch_hr_valid;           // indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;


#define MAX_BRIGHTNESS 255


void HeartSpo2_getvalue_task(void *pvParameters)
{
	// ���㷴ӳ�����İ��� LED ���ȵı���
    uint32_t un_min, un_max, un_prev_data;
    int32_t i;
    int32_t n_brightness;
    float f_temp;

    uint8_t temp[6];

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    un_min = 0x3FFFF;
    un_max = 0;

    n_ir_buffer_length = 500; // ����������Ϊ 500���洢�� 100sps ���е� 5 ������

    // ��ȡǰ 500 ��������ȷ���źŷ�Χ
    for (i = 0; i < n_ir_buffer_length; i++)
    {
        while (MAX30102_INT == 1); // �ȴ��ж����Ŷ���

        max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);
        aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2]; // ���ֵ�Ի��ʵ������
        aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];  // ���ֵ�Ի��ʵ������

        if (un_min > aun_red_buffer[i])
            un_min = aun_red_buffer[i]; // �����ź���Сֵ
        if (un_max < aun_red_buffer[i])
            un_max = aun_red_buffer[i]; // �����ź����ֵ
    }
    un_prev_data = aun_red_buffer[i];
    // ����ǰ 500 ������������ʺ�Ѫ�����Ͷȣ�ǰ 5 ���������
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

    while (1)
    {
        i = 0;
        un_min = 0x3FFFF;
        un_max = 0;
        n_ir_buffer_length = 500;

        // ��ǰ 100 ������ת�����洢���У�������� 400 �������Ƶ�����
        for (i = 100; i < 500; i++)
        {
            aun_red_buffer[i - 100] = aun_red_buffer[i];
            aun_ir_buffer[i - 100] = aun_ir_buffer[i];

            // �����ź���Сֵ�����ֵ
            if (un_min > aun_red_buffer[i])
                un_min = aun_red_buffer[i];

            if (un_max < aun_red_buffer[i])
                un_max = aun_red_buffer[i];
        }

        // �ڼ�������֮ǰ�ɼ� 100 ������
        for (i = 400; i < 500; i++)
        {
            un_prev_data = aun_red_buffer[i - 1];

            while (MAX30102_INT == 1)
                ;

            max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

            // ���ֵ�Ի��ʵ������
            aun_red_buffer[i] = ((temp[0] & 0x03) << 16) | (temp[1] << 8) | temp[2];
            aun_ir_buffer[i] = ((temp[3] & 0x03) << 16) | (temp[4] << 8) | temp[5];

            if (aun_red_buffer[i] > un_prev_data)
            {
                f_temp = aun_red_buffer[i] - un_prev_data;

                f_temp /= (un_max - un_min);

                f_temp *= MAX_BRIGHTNESS;

                n_brightness -= (int32_t)f_temp;

                if (n_brightness < 0)
                    n_brightness = 0;
            }
            else
            {
                f_temp = un_prev_data - aun_red_buffer[i];

                f_temp /= (un_max - un_min);

                f_temp *= MAX_BRIGHTNESS;

                n_brightness += (int32_t)f_temp;

                if (n_brightness > MAX_BRIGHTNESS)
                    n_brightness = MAX_BRIGHTNESS;
            }
        }

        // �������ʺ�Ѫ�����Ͷ�
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

        /* ͨ��UART�������ͼ��������͵��ն˳��� */
        if ((ch_hr_valid == 1) && (n_heart_rate >= 60) && (n_heart_rate <= 100))
        {
						printf("����=%d\r\n", n_heart_rate); 
					
            // �������ʱ�ǩ
            lv_label_set_text_fmt(ui_Label2, "%d", n_heart_rate);
            lv_arc_set_value(ui_Label2, n_heart_rate);
        }
							
        if ((ch_spo2_valid == 1) && (n_sp02 >= 70) && (n_sp02 <= 100))
        {
						printf("Ѫ��Ũ��=%d\r\n", n_sp02); 
						
						// ����Ѫ����ǩ
            lv_label_set_text_fmt(ui_Label13, "%d", n_sp02);
            lv_arc_set_value(ui_Label13, n_sp02);
        }

        vTaskDelay(1000); // �ӳ��Ա�������ֹѭ��
    }
}

