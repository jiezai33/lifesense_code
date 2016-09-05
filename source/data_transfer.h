#ifndef _TRANSFER_H_
#define _TRANSFER_H_
#include <stdint.h>

#define USR_DATA_TABLE_SIZE			(32)	//���ݴ���״̬����������

typedef struct
{
	uint32_t addr;	//flash�ĵ�ַ
	uint8_t group;	//flash��ַ�����������
}flash_addr_struct;


typedef struct
{
	uint8_t data_type;	//��������
	uint8_t data_id_max;//����ID�������
}current_data_statue_struct;

typedef struct
{
	uint8_t data_id;		//����ID
	uint8_t earse_flag;		//������־
	uint8_t time_out_count;	//��ʱ�Ĵ���
	uint32_t time;			//���͵�ǰ����ʱ��
	flash_addr_struct *addr;//����ID��Ӧ��flash��ַ������
}data_struct;

typedef struct
{
	current_data_statue_struct statue;
	data_struct data[USR_DATA_TABLE_SIZE];
}data_transfer_table_struct;


typedef enum
{
	STEP_DATA = 1,	//�ǲ�����
	SLEEP_DATA,		//˯������
	HOUR_DATA,		//Сʱ����
	FLASH_DATA_MAX
}flash_data_enum;




/*****************************************************************************
 * �� �� �� : get_data_falsh_addr
 * �������� : 
 * ������� : uint8_t data_type  ��������
               int8_t data_id     ����ID
 * ������� : ��
 * �� �� ֵ : �ο� flash_addr_struct �ṹ��
 * �޸���ʷ : ��
 * ˵    �� : ���ݴ�����������ͺ�ID��ȡ��Ӧ��flash��ַ��������
*****************************************************************************/
flash_addr_struct *get_data_falsh_addr(uint8_t data_type,int8_t data_id);




/*****************************************************************************
 * �� �� �� : data_transfer_statue_init
 * �������� : 
 * ������� : flash_data_enum data_type  ��������
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : �����������ÿ�η�����ǰ���Ѵ�������״̬��ĸ���ʼ��һ�Σ���ΪĿǰ����Ļ�����ÿ��ֻ�ܴ���һ�����͵�����
 				���Կ��Ը��ݴ��������������ͽ��г�ʼ������״̬��������Ӧ��flash�ĵ�ַ��ֵ��״̬��
*****************************************************************************/
void data_transfer_statue_init(flash_data_enum data_type);




/*****************************************************************************
 * �� �� �� : data_retransmission
 * �������� : 
 * ������� : 	uint8_t data_type        �������� 
 				uint8_t data_id          ����ID
                flash_addr_struct *addr  ��Ӧflash �ĵ�ַ�������� 
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : �����ظ����ƣ�������û�з��ͳɹ���������������������·�����Ӧ��������
*****************************************************************************/
uint8_t data_retransmission(uint8_t data_type,int8_t data_id,flash_addr_struct *addr);




/*****************************************************************************
 * �� �� �� : data_transmit
 * �������� : 
 * ������� :  uint8_t data_type  ��������
               uint8_t data_id    ����ID
               uint8_t *data      ����ָ��
               uint8_t length     ���ݳ���
 * ������� : ��
 * �� �� ֵ :   0:���ݴ�����ɻ����Ѿ��ѵ�ǰ�������ݴ��䵽����bufferȥ��
 				5:���������뵱ǰ��������״̬���� �������Ͳ�һ��
 				6:����ID������ǰ�������͵�ID�����ֵ
 				7:�ڳ�ʱ��ʱ�䷶Χ�ڣ��������ǰ��
 * �޸���ʷ : ��
 * ˵    �� : ���������flash��ѯ�Ƿ������ݷ��͸�APP������еĻ���������ĺ�����������Ӧ������
 *****************************************************************************/
uint32_t data_transmit(uint8_t data_type,uint8_t data_id,uint8_t *data,uint8_t length);




/*****************************************************************************
 * �� �� �� : data_transmit_ack
 * �������� : 
 * ������� : uint8_t data_type  ��������
               uint8_t data_id    ����ID
 * ������� : ��
 * �� �� ֵ :   0:��ǰ���ݰ�������
 				5:��ǰ���͵��������Ͳ���
 				6:��ǰ���͵�����IDԽ��
 * �޸���ʷ : ��
 * ˵    �� : ͬ�������첽�������ݺ󣬵õ���������Ӧ���������Ӧ�İ�����λ��1
*****************************************************************************/
uint8_t data_transmit_ack(uint8_t data_type,uint8_t data_id);




/*****************************************************************************
 * �� �� �� : data_transfer_statue_loop
 * �������� : 
 * ������� : void  ��
 * ������� : ��
 * �� �� ֵ :   0:˵����ǰ�������͵����а����Ѿ�������ɣ����Բ�����
 				1:�ظ��������ظ����������������������Ͽ�����
 				5:״̬�������ʹ���
 * �޸���ʷ : ��
 * ˵    �� : �������mainloop����ѭ������
*****************************************************************************/
uint8_t data_transfer_statue_mainloop(void);

#endif

