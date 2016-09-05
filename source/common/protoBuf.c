#include "protoBuf.h"

typedef union   uint64_t_type
{
    uint64_t    data64;
    uint32_t    data32[2];
    uint8_t     data8[8];
} uint64_def;

static uint8_t UnPackDataValue(uint8_t* pInData,uint8_t packlen,uint8_t *pOutdata,uint8_t *datalen,uint8_t *offset)
{
    uint8_t temp,*pdata,i,tempData,tdat;        //  ,len
    uint64_def  unpackdata;
    unpackdata.data64 = 0x0000000000000000;
    pdata = pInData;
    i = 0x00;
    do
    {
        temp = (*pdata++);       //  (*pInData++);        //
        tempData = temp & 0x7f;     // 取低7位数值
        if(i==0x00)
        {
            // 当前字段第一字节，取低7位数值
            unpackdata.data8[i] |= tempData;
        }
        else
        {
            /* 数据解析后存放数值中的位置，此处数值解析只做到64位，超出将不支持
            ，需要重新修改，微信目前处理64位足够用 */
            tdat = (tempData<<(8-i));   /* 移位获取当前字节存放低字节部分数值位 */
            unpackdata.data8[i-1] |= tdat;  /* 保存当前字节解析的低位段 */
            tdat = tempData >> i;       /* 移位获取当前字节存放高字节部分数值位 */
            unpackdata.data8[i] |= tdat;        /* 保存当前字节解析的高位段 */
        }
        i++;
        if(i>packlen)   /* 判断解析的数值是否超出解析的数据长度 */
        {
            /* 解析的数值超出解析的数据长度，返回错误 */
            return  false;
        }
    }
    while((temp&0x80)==0x80);   /* 取当前字节的最高位，用于判断下一字节还是该
                                 字段内，1为下一字节还是本字段内容，0为下一字节不是本字段内容 */

    if(unpackdata.data32[1]>0x00)   /* 判断解析后的数值是否超出32位 */
    {
        *datalen = SIZE_OF_FOUR_WORD_LEN;   /* 数值超出32位，数据长度定为8字节 */
    }
    else if(unpackdata.data32[0]>0xffff)        /* 判断解析后的数值是否超出16位 */
    {
        *datalen = SIZE_OF_TWO_WORD_LEN;    /* 数值超出16位，数据长度定为4字节 */
    }
    else if(unpackdata.data32[0]>0xff)      /* 判断解析后的数值是否超出8位 */
    {
        *datalen = SIZE_OF_WORD_LEN;        /* 数值超出8位，数据长度定为2字节 */
    }
    else
    {
        *datalen = SIZE_OF_BYTE_LEN;        /* 数据长度定为1字节 */
    }
    for(i=0; i<(*datalen); i++)
    {
        *pOutdata++ = unpackdata.data8[i];  /* 解析的数值，copy到输出数据指针中 */
    }
    *offset = pdata-pInData;        /* 数据解析了数据包中的数据长度 */
    return  true;
}

static uint8_t UnPackDataType(uint8_t type,uint8_t* pInData, uint8_t packlen,
                       uint8_t* pOutData, uint8_t *out_len,uint8_t *offset)
{
    uint8_t dataLen;        //  ,offset = 0x00;
    uint8_t i,*pdata,*pOut_data;
    uint64_def  unpackdata;
    pdata = pInData;
    pOut_data = pOutData;


    switch(type)
    {
        case Varint:         /*  int32, int64, uint32, uint64, sint32, sint64, bool, enum    */
        {
            if(UnPackDataValue(pdata, packlen,pOut_data, out_len, offset) == false)
            {
                return false;
            }
            break;
        }
        case Bit64:          /*  fixed64, sfixed64, double                                   */
        {
            /* 取64位数据 */
            *out_len = SIZE_OF_64_BIT_LEN;  /* 数据解析了数据包中的数据长度 */
            /* ?????????????需要增加输出buffer长度限制，防止溢出 */
            for(i=0; i<(*out_len); i++)
            {
                *pOutData++ = *pdata++;
            }
            offset = out_len;
            break;
        }
        case Length_delimit: /*  string, bytes, embedded messages, packed repeated fields    */
        {
            /* 获取字符串长度，解析的数据长度、解析后的指针偏移量 */
            if(UnPackDataValue(pdata, packlen,pOut_data, &dataLen, offset) == false)
            {
                return false;
            }
            /* ?????????????????????????需要增加输入数据包长度判断???????????? */
            /* 缓存字符串长度 */
            for(i=0; i<dataLen; i++)
            {
                unpackdata.data8[i] = *pOutData+i;
            }
            /* ???????????????需要增加输入buffer长度上限限制，防止溢出 */
            /* 获取字符串在未解析的数据包中的偏移地址 */
            pdata += *offset;
            /* 字符串长度 */
            *out_len = unpackdata.data8[0]; //(uint8_t)(tempData);
            /* ?????????????????????????需要增加输出数据包长度判断???????????? */
            for(i=0; i<(*out_len); i++)
            {
                /* copy字符串到输出buffer */
                *pOutData++ = *pdata++;
            }

            /* 该字段解析了“未解析数据包内容”的中长度 */
            *offset = pdata-pInData;        //*out_len;
            break;
        }
        case Start_group:    /*  Groups (deprecated)                                         */
        {
            /* 此字段目前不使用，不做处理 */
            break;
        }
        case End_group:      /*  Groups (deprecated)                                         */
        {
            /* 此字段目前不使用，不做处理 */
            break;
        }
        case Bit32:          /*  fixed32, sfixed32, float                                    */
        {
            /* ???????????????需要增加输入buffer长度上限限制，防止溢出 */

            /* 取64位数据 */
            *out_len = SIZE_OF_32_BIT_LEN;  /* 数据解析了数据包中的数据长度 */

            /* ?????????????需要增加输出buffer长度限制，防止溢出 */

            for(i=0; i<(*out_len); i++)
            {
                /* copy数据到输出buffer */
                *pOutData++ = *pdata++;
            }
            offset = out_len;
            break;
        }
        default:
            break;
    }
    return true;
}

uint8_t response_unpack(uint8_t dest_field,uint8_t *pData, uint8_t Len,
                           uint8_t *pUnData, uint8_t *pdatalen,uint8_t *offset)
{
    uint8_t error, *pdata,templen;//,temp;
    uint8_t type, OutData[128];
    uint8_t i,OutLen, tempoffset;   // HexData[];
    uint64_def  tempdata;
    uint64_t field;
    tempdata.data64=0x0000000000000000;
    OutLen = 0x00;
    tempoffset = 0x00;
    pdata = pData;
    templen=Len;

    do
    {
        /* 获取protocol buffer字段和type数值 */
        error = UnPackDataValue(pdata,templen,OutData,&OutLen,&tempoffset);
        if(error ==false)
        {
            /* 获取protocol buffer解析出错返回错误值 */
            return false;
        }
        for(i=0; i<OutLen; i++)
        {
            tempdata.data8[i]=OutData[i];   /* protocol buffer字段和type值 */
        }
        field= tempdata.data64/8;           /* protocol buffer字段数值（字段数值bit3开始算起，这里除以8之所以没有用移位处理，是因为64位移位会出错  */
        type = tempdata.data8[0] & 0x07;    /* 获取protocol buffer当前字段的type */

        pdata += tempoffset;    /* 未解析数据包指针偏移到下内容地址 */
        templen -= tempoffset;  /* 计算当前未解析数据包长度 */
        /* 解析获取当前字段内容 */
        error = UnPackDataType(type, pdata, templen, OutData, &OutLen,&tempoffset);
        if(error ==false)
        {
            /* 当前字段内容解析出错，返回错误值 */
            return false;
        }
        pdata += tempoffset;    /* 未解析的数据包指针指向下一个字段内容 */
        templen -= tempoffset;  /* 计算当前未解析数据包长度 */
    }
    while(field!=(uint64_t)dest_field);     /* 判断解析的字段是否是所需的字段，不是，继续解析；是，退出解析 */

    *offset = pdata - pData;    /* 获取当前解析了数据包的长度 */
    *pdatalen = OutLen;         /* 当前解析的字段内容长度 */

    for(i=0; i<OutLen; i++)
    {
        *pUnData++ = OutData[i];    /* 输出解析内容 */
    }

    return  true;
}


/**************************************************************
Function:       针对数据字段和类型封装数据包
Description:    protobuffer封包过程，对每种类型的数据进行编码封包
Input:          field 数字包字段，type 数据类型，pInData未封包数据包内容，UnLen未封包数据包长度
Output:         pOutData封包数据包内容,out_len封包数据包长度,
Return:         返回封包后数据包长度
***************************************************************/
uint8_t    PackDataType(uint8_t field, uint8_t type,uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData)
{
    uint64_t    data,tempData;   //
    uint8_t     out_len,temp_len,i,*pdata;
    uint16_t    field_type=0x0000;      /* 用于存放字段和type，需要增加位数，防止字段数值超出13bit，目前不需要 */

    tempData=0x0000000000000000;
    pdata = pInData;
    out_len=0x00;
    field_type = field;     /* 获取字段值 */
    field_type <<= 3;       /* 字段数值右移3位，用于存放type，这里需要注意防止字段值超出13bit */
    field_type += type;     /* 合并字段和type */
    /* 判断字段和type超出8bit？ */
    if(field_type>0x7F) /* ??????????????????这里应该使用for循环封包，防止字段值超出11位 */
    {
        /* 字段和type超出8bit，拆分成protocol buffer形式的编码 */
        *pOutData++ = (uint8_t)((field_type &0x7F) | 0x80); /* 封包最低字节的最高位置1 */
        field_type = (field_type) >>7;      /* 字段和type数值左移7位 */
        out_len++;      /* 封包数据包长度递增 */
    }
    *pOutData = ((uint8_t)field_type) &0x7F;    /* 把字段和type的剩余小于等于7位的数值封包 */
    out_len++;          /* 封包数据包长度递增 */


    switch(type)
    {
        case    Varint:         /*  int32, int64, uint32, uint64, sint32, sint64, bool, enum    */
        {
            /* 获取该字段内容的数值 */
            for(i=0; i<Unlen; i++)
            {
                tempData >>= 8;         /* 缓存先左移8位，最多存放64位数值 */
                data = (*pInData++);    /* 提取一个字节数据 */
                tempData +=  data<< ((Unlen-1)<<3); /* 把提取出的数据放在缓存的最高字节中，这样
                                           就可以保证提取的数据数值是按实际数值输出 */
            }
            /* 判断数值是否大于7位，按照protocol buffer编码规则，数值大于7位
               的，就要把最低7位提取出来，并把提取出的数值最高位补1，逐个比较直到结束 */
            while(tempData > 0x7F)
            {
                (*++pOutData) = (uint8_t)((tempData & 0x7F) | 0x80);    /* 提取低7位的数值，再加上128 */
                out_len++;      /* 封包数据包长度递增 */
                tempData >>=7;  /* 封包的数值右移7位 */
            }
            *++pOutData = (uint8_t)(tempData & 0x7F);   /* 提取封包数值的最高7位数值 */
            out_len++;      /* 封包数据包长度递增 */
            break;
        }
        case    Bit64:          /*  fixed64, sfixed64, double                                   */
        {
            for(i=0; i<8; i++)
            {
                *(++pOutData) = *pdata++;   /* 64位数值封包 */
            }

            out_len += 8;      /* 封包数据包长度加8 */

            break;
        }
        case    Length_delimit: /*  string, bytes, embedded messages, packed repeated fields    */
        {
            /* 计算字符串长度 */
            temp_len = Unlen;
            /* 判断数据包内容长度是否大于127，这里字节数不会超过256，所以只判断最多2字节 */
            if(Unlen > 0x7F)
            {
                /* 数据包长度大于127，需要按protocol buffer编码拆分字节数 */
                *(++pOutData) = (uint8_t)((Unlen &0x7F) | 0x80);    /* 提取低7位的数值，再加上128 */
                Unlen = (Unlen) >>7;        /* 长度数据右移7位，继续封包 */
                out_len++;              /* 封包数据包长度递增 */
            }
            *(++pOutData) = ((uint8_t)Unlen) & 0x7F;    /* 提取数据内容长度最高7位数值 */
            out_len++;                   /* 封包数据包长度递增 */
            for(i=0; i<temp_len; i++)
            {
                *(++pOutData) = *pdata++;   /* 封装数据内容 */
                out_len++;               /* 封包数据包长度递增 */
            }
            break;
        }
        case    Start_group:    /*  Groups (deprecated)                                         */
        {
            break;
        }
        case    End_group:      /*  Groups (deprecated)                                         */
        {
            break;
        }
        case    Bit32:          /*  fixed32, sfixed32, float                                    */
        {
            for(i=0; i<4; i++)
            {
                *(++pOutData) = *pdata++;      /* 32位数值封包 */
            }
            --*pOutData;
            out_len += 4;      /* 封包数据包长度加4 */

            break;
        }
        default:
            break;
    }
    return  out_len;
}







