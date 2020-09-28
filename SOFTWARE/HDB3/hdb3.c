#include "hdb3.h"
/**
 * @description: Encode the source code as HDB3 code.
 * @param: hdb3   is a pointer to HDB3 code.
 *         source is a pointer to source code.
 *         len    is sequence length.
 * @return: none
 */
void HDB3_Encoding(signed char *hdb3, const signed char *source, int len)
{   
    unsigned int i = 0;
    unsigned int cnt_0 = 0; //��¼����0�ĸ���
    signed char last_b = 0;        //��¼��һ�����루��B��B'�����ԣ���ʼ��Ϊ0��֤��һ�����뼫��Ϊ+1
    signed char last_v = -1;       //��¼��һ��V�뼫�ԣ���ʼ��Ϊ-1��֤��һ��V�뼫��Ϊ+1
    for(i = 0; i < len; i++)
    {
        if(!source[i])
        {//Դ��Ϊ0
            cnt_0++;
            if(cnt_0 >= 4)
            {//����0�ĸ�������4
                cnt_0 = 0;
                *(hdb3+i) = (last_v&0x80)?1:-1;//��֤V�뼫�Խ���仯
                last_v = *(hdb3+i);//����last_v
                if((last_v != last_b))
                {//�����ǰV����ǰһ�����뼫�Բ�ͬ��������ͬ���ԵĲ�����
                    *(hdb3+i-3) = last_v;
                    last_b = last_v;//����last_b
                }
            }
            else
            {
                *(hdb3+i) = 0;
            }
        }
        else
        {//Դ��Ϊ1
            cnt_0 = 0;
            if(!last_b)
            {//����ǵ�һ��Ϊ1��Դ�룬���ӦHDB3������뼫��Ϊ+1
                *(hdb3+i) = 1;
                last_b = 1;
            }
            else
            {
                *(hdb3+i) = (last_b&0x80)?1:-1;//��֤���뼫�Խ���仯
                last_b = *(hdb3+i);//����last_b
            }
        }
    }
}

/**
 * @description: Decode HDB3 code.
 * @param: hdb3   is a pointer to HDB3 code.
 *         source is a pointer to source code.
 *         len    is sequence length.
 * @return: none
 */
void HDB3_Decoding(signed char *source, const signed char *hdb3, int len)
{
    unsigned int i = 0;
    unsigned int cnt_0 = 0; //��¼����0�ĸ���
    signed char polarity = 0;      //��¼ǰһ�����루��B��B'������
    for(i = 0; i < len; i++)
    {
        if(!(*(hdb3+i)))
        {//HDB3��Ϊ0��������һ����0
            cnt_0++;
            *(source+i) = 0;
        }
        else
        {//HDB3��Ϊ1��-1
            if((*(hdb3+i) == polarity)&&(cnt_0 >= 2))
            {//�����ǰHDB3�뼫����ǰһ�����뼫����ͬ������0�ĸ������ڵ���2��
             //��ǰHDB3����V�룬������Ϊ0
                *(source+i) = 0;
                if(cnt_0 == 2)
                {//�������0�ĸ�������2������ǰһ�������ǲ����룬������Ϊ0
                    *(source+i-3) = 0;//�޸Ĳ����봦��������
                }
            }
            else
            {//��ǰHDB3�������루��B��B'������������1
                *(source+i) = 1;
                polarity = *(hdb3+i);//��¼���뼫��
            }
            cnt_0 = 0;
        }
    }
}

void Dec2Bin(u16 dec, signed char *binary, u16 len){

	signed char i = 0;
    u16 k = 0, temp;
    temp = dec;
    while (temp)
    {
        i = temp % 2;
        binary[k] = i;
        k++;
        if (k>=len)
            break;
        temp = temp / 2;
    }
}

u16 Bin2Dec(signed char *binary, u16 len)
{
    signed char i = 0;
    u16 dec = 0;
    for (i = 0; i < len; i++)
    {
        dec += pow(2, i) * binary[i];
    }
    return dec;
}

void print_binary_string(signed char *binary, u16 len){
    int i = 0;
    for (i = 0; i < len; i++)
    {
        printf("%d", binary[len-i-1]);
    }
    printf("\r\n");
}
