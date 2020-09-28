#ifndef __HDB3_
#define __HDB3_
#include "stm32f4xx.h"
#include <stdio.h>
#include <math.h>
void HDB3_Encoding(signed char *hdb3, const signed char *source, int len);
void HDB3_Decoding(signed char *source, const signed char *hdb3, int len);
void Dec2Bin(u16 dec, signed char * binary, u16 len);
u16 Bin2Dec(signed char *binary, u16 len);
void print_binary_string(signed char *binary, u16 len);
#endif
