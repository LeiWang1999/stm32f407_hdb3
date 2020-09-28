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
    unsigned int cnt_0 = 0; //记录连续0的个数
    signed char last_b = 0;        //记录上一个信码（含B及B'）极性，初始化为0保证第一个信码极性为+1
    signed char last_v = -1;       //记录上一个V码极性，初始化为-1保证第一个V码极性为+1
    for(i = 0; i < len; i++)
    {
        if(!source[i])
        {//源码为0
            cnt_0++;
            if(cnt_0 >= 4)
            {//连续0的个数大于4
                cnt_0 = 0;
                *(hdb3+i) = (last_v&0x80)?1:-1;//保证V码极性交替变化
                last_v = *(hdb3+i);//更新last_v
                if((last_v != last_b))
                {//如果当前V码与前一个信码极性不同，则增加同极性的补信码
                    *(hdb3+i-3) = last_v;
                    last_b = last_v;//更新last_b
                }
            }
            else
            {
                *(hdb3+i) = 0;
            }
        }
        else
        {//源码为1
            cnt_0 = 0;
            if(!last_b)
            {//如果是第一个为1的源码，则对应HDB3码的信码极性为+1
                *(hdb3+i) = 1;
                last_b = 1;
            }
            else
            {
                *(hdb3+i) = (last_b&0x80)?1:-1;//保证信码极性交替变化
                last_b = *(hdb3+i);//更新last_b
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
    unsigned int cnt_0 = 0; //记录连续0的个数
    signed char polarity = 0;      //记录前一个信码（含B及B'）极性
    for(i = 0; i < len; i++)
    {
        if(!(*(hdb3+i)))
        {//HDB3码为0，译码结果一定是0
            cnt_0++;
            *(source+i) = 0;
        }
        else
        {//HDB3码为1或-1
            if((*(hdb3+i) == polarity)&&(cnt_0 >= 2))
            {//如果当前HDB3码极性与前一个信码极性相同且连续0的个数大于等于2个
             //则当前HDB3码是V码，译码结果为0
                *(source+i) = 0;
                if(cnt_0 == 2)
                {//如果连续0的个数等于2个，则前一个信码是补信码，译码结果为0
                    *(source+i-3) = 0;//修改补信码处的译码结果
                }
            }
            else
            {//当前HDB3码是信码（含B及B'），译码结果是1
                *(source+i) = 1;
                polarity = *(hdb3+i);//记录信码极性
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
