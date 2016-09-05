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
        tempData = temp & 0x7f;     // ȡ��7λ��ֵ
        if(i==0x00)
        {
            // ��ǰ�ֶε�һ�ֽڣ�ȡ��7λ��ֵ
            unpackdata.data8[i] |= tempData;
        }
        else
        {
            /* ���ݽ���������ֵ�е�λ�ã��˴���ֵ����ֻ����64λ����������֧��
            ����Ҫ�����޸ģ�΢��Ŀǰ����64λ�㹻�� */
            tdat = (tempData<<(8-i));   /* ��λ��ȡ��ǰ�ֽڴ�ŵ��ֽڲ�����ֵλ */
            unpackdata.data8[i-1] |= tdat;  /* ���浱ǰ�ֽڽ����ĵ�λ�� */
            tdat = tempData >> i;       /* ��λ��ȡ��ǰ�ֽڴ�Ÿ��ֽڲ�����ֵλ */
            unpackdata.data8[i] |= tdat;        /* ���浱ǰ�ֽڽ����ĸ�λ�� */
        }
        i++;
        if(i>packlen)   /* �жϽ�������ֵ�Ƿ񳬳����������ݳ��� */
        {
            /* ��������ֵ�������������ݳ��ȣ����ش��� */
            return  false;
        }
    }
    while((temp&0x80)==0x80);   /* ȡ��ǰ�ֽڵ����λ�������ж���һ�ֽڻ��Ǹ�
                                 �ֶ��ڣ�1Ϊ��һ�ֽڻ��Ǳ��ֶ����ݣ�0Ϊ��һ�ֽڲ��Ǳ��ֶ����� */

    if(unpackdata.data32[1]>0x00)   /* �жϽ��������ֵ�Ƿ񳬳�32λ */
    {
        *datalen = SIZE_OF_FOUR_WORD_LEN;   /* ��ֵ����32λ�����ݳ��ȶ�Ϊ8�ֽ� */
    }
    else if(unpackdata.data32[0]>0xffff)        /* �жϽ��������ֵ�Ƿ񳬳�16λ */
    {
        *datalen = SIZE_OF_TWO_WORD_LEN;    /* ��ֵ����16λ�����ݳ��ȶ�Ϊ4�ֽ� */
    }
    else if(unpackdata.data32[0]>0xff)      /* �жϽ��������ֵ�Ƿ񳬳�8λ */
    {
        *datalen = SIZE_OF_WORD_LEN;        /* ��ֵ����8λ�����ݳ��ȶ�Ϊ2�ֽ� */
    }
    else
    {
        *datalen = SIZE_OF_BYTE_LEN;        /* ���ݳ��ȶ�Ϊ1�ֽ� */
    }
    for(i=0; i<(*datalen); i++)
    {
        *pOutdata++ = unpackdata.data8[i];  /* ��������ֵ��copy���������ָ���� */
    }
    *offset = pdata-pInData;        /* ���ݽ��������ݰ��е����ݳ��� */
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
            /* ȡ64λ���� */
            *out_len = SIZE_OF_64_BIT_LEN;  /* ���ݽ��������ݰ��е����ݳ��� */
            /* ?????????????��Ҫ�������buffer�������ƣ���ֹ��� */
            for(i=0; i<(*out_len); i++)
            {
                *pOutData++ = *pdata++;
            }
            offset = out_len;
            break;
        }
        case Length_delimit: /*  string, bytes, embedded messages, packed repeated fields    */
        {
            /* ��ȡ�ַ������ȣ����������ݳ��ȡ��������ָ��ƫ���� */
            if(UnPackDataValue(pdata, packlen,pOut_data, &dataLen, offset) == false)
            {
                return false;
            }
            /* ?????????????????????????��Ҫ�����������ݰ������ж�???????????? */
            /* �����ַ������� */
            for(i=0; i<dataLen; i++)
            {
                unpackdata.data8[i] = *pOutData+i;
            }
            /* ???????????????��Ҫ��������buffer�����������ƣ���ֹ��� */
            /* ��ȡ�ַ�����δ���������ݰ��е�ƫ�Ƶ�ַ */
            pdata += *offset;
            /* �ַ������� */
            *out_len = unpackdata.data8[0]; //(uint8_t)(tempData);
            /* ?????????????????????????��Ҫ����������ݰ������ж�???????????? */
            for(i=0; i<(*out_len); i++)
            {
                /* copy�ַ��������buffer */
                *pOutData++ = *pdata++;
            }

            /* ���ֶν����ˡ�δ�������ݰ����ݡ����г��� */
            *offset = pdata-pInData;        //*out_len;
            break;
        }
        case Start_group:    /*  Groups (deprecated)                                         */
        {
            /* ���ֶ�Ŀǰ��ʹ�ã��������� */
            break;
        }
        case End_group:      /*  Groups (deprecated)                                         */
        {
            /* ���ֶ�Ŀǰ��ʹ�ã��������� */
            break;
        }
        case Bit32:          /*  fixed32, sfixed32, float                                    */
        {
            /* ???????????????��Ҫ��������buffer�����������ƣ���ֹ��� */

            /* ȡ64λ���� */
            *out_len = SIZE_OF_32_BIT_LEN;  /* ���ݽ��������ݰ��е����ݳ��� */

            /* ?????????????��Ҫ�������buffer�������ƣ���ֹ��� */

            for(i=0; i<(*out_len); i++)
            {
                /* copy���ݵ����buffer */
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
        /* ��ȡprotocol buffer�ֶκ�type��ֵ */
        error = UnPackDataValue(pdata,templen,OutData,&OutLen,&tempoffset);
        if(error ==false)
        {
            /* ��ȡprotocol buffer���������ش���ֵ */
            return false;
        }
        for(i=0; i<OutLen; i++)
        {
            tempdata.data8[i]=OutData[i];   /* protocol buffer�ֶκ�typeֵ */
        }
        field= tempdata.data64/8;           /* protocol buffer�ֶ���ֵ���ֶ���ֵbit3��ʼ�����������8֮����û������λ��������Ϊ64λ��λ�����  */
        type = tempdata.data8[0] & 0x07;    /* ��ȡprotocol buffer��ǰ�ֶε�type */

        pdata += tempoffset;    /* δ�������ݰ�ָ��ƫ�Ƶ������ݵ�ַ */
        templen -= tempoffset;  /* ���㵱ǰδ�������ݰ����� */
        /* ������ȡ��ǰ�ֶ����� */
        error = UnPackDataType(type, pdata, templen, OutData, &OutLen,&tempoffset);
        if(error ==false)
        {
            /* ��ǰ�ֶ����ݽ����������ش���ֵ */
            return false;
        }
        pdata += tempoffset;    /* δ���������ݰ�ָ��ָ����һ���ֶ����� */
        templen -= tempoffset;  /* ���㵱ǰδ�������ݰ����� */
    }
    while(field!=(uint64_t)dest_field);     /* �жϽ������ֶ��Ƿ���������ֶΣ����ǣ������������ǣ��˳����� */

    *offset = pdata - pData;    /* ��ȡ��ǰ���������ݰ��ĳ��� */
    *pdatalen = OutLen;         /* ��ǰ�������ֶ����ݳ��� */

    for(i=0; i<OutLen; i++)
    {
        *pUnData++ = OutData[i];    /* ����������� */
    }

    return  true;
}


/**************************************************************
Function:       ��������ֶκ����ͷ�װ���ݰ�
Description:    protobuffer������̣���ÿ�����͵����ݽ��б�����
Input:          field ���ְ��ֶΣ�type �������ͣ�pInDataδ������ݰ����ݣ�UnLenδ������ݰ�����
Output:         pOutData������ݰ�����,�oout_len������ݰ�����,
Return:         ���ط�������ݰ�����
***************************************************************/
uint8_t    PackDataType(uint8_t field, uint8_t type,uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData)
{
    uint64_t    data,tempData;   //
    uint8_t     out_len,temp_len,i,*pdata;
    uint16_t    field_type=0x0000;      /* ���ڴ���ֶκ�type����Ҫ����λ������ֹ�ֶ���ֵ����13bit��Ŀǰ����Ҫ */

    tempData=0x0000000000000000;
    pdata = pInData;
    out_len=0x00;
    field_type = field;     /* ��ȡ�ֶ�ֵ */
    field_type <<= 3;       /* �ֶ���ֵ����3λ�����ڴ��type��������Ҫע���ֹ�ֶ�ֵ����13bit */
    field_type += type;     /* �ϲ��ֶκ�type */
    /* �ж��ֶκ�type����8bit�� */
    if(field_type>0x7F) /* ??????????????????����Ӧ��ʹ��forѭ���������ֹ�ֶ�ֵ����11λ */
    {
        /* �ֶκ�type����8bit����ֳ�protocol buffer��ʽ�ı��� */
        *pOutData++ = (uint8_t)((field_type &0x7F) | 0x80); /* �������ֽڵ����λ��1 */
        field_type = (field_type) >>7;      /* �ֶκ�type��ֵ����7λ */
        out_len++;      /* ������ݰ����ȵ��� */
    }
    *pOutData = ((uint8_t)field_type) &0x7F;    /* ���ֶκ�type��ʣ��С�ڵ���7λ����ֵ��� */
    out_len++;          /* ������ݰ����ȵ��� */


    switch(type)
    {
        case    Varint:         /*  int32, int64, uint32, uint64, sint32, sint64, bool, enum    */
        {
            /* ��ȡ���ֶ����ݵ���ֵ */
            for(i=0; i<Unlen; i++)
            {
                tempData >>= 8;         /* ����������8λ�������64λ��ֵ */
                data = (*pInData++);    /* ��ȡһ���ֽ����� */
                tempData +=  data<< ((Unlen-1)<<3); /* ����ȡ�������ݷ��ڻ��������ֽ��У�����
                                           �Ϳ��Ա�֤��ȡ��������ֵ�ǰ�ʵ����ֵ��� */
            }
            /* �ж���ֵ�Ƿ����7λ������protocol buffer���������ֵ����7λ
               �ģ���Ҫ�����7λ��ȡ������������ȡ������ֵ���λ��1������Ƚ�ֱ������ */
            while(tempData > 0x7F)
            {
                (*++pOutData) = (uint8_t)((tempData & 0x7F) | 0x80);    /* ��ȡ��7λ����ֵ���ټ���128 */
                out_len++;      /* ������ݰ����ȵ��� */
                tempData >>=7;  /* �������ֵ����7λ */
            }
            *++pOutData = (uint8_t)(tempData & 0x7F);   /* ��ȡ�����ֵ�����7λ��ֵ */
            out_len++;      /* ������ݰ����ȵ��� */
            break;
        }
        case    Bit64:          /*  fixed64, sfixed64, double                                   */
        {
            for(i=0; i<8; i++)
            {
                *(++pOutData) = *pdata++;   /* 64λ��ֵ��� */
            }

            out_len += 8;      /* ������ݰ����ȼ�8 */

            break;
        }
        case    Length_delimit: /*  string, bytes, embedded messages, packed repeated fields    */
        {
            /* �����ַ������� */
            temp_len = Unlen;
            /* �ж����ݰ����ݳ����Ƿ����127�������ֽ������ᳬ��256������ֻ�ж����2�ֽ� */
            if(Unlen > 0x7F)
            {
                /* ���ݰ����ȴ���127����Ҫ��protocol buffer�������ֽ��� */
                *(++pOutData) = (uint8_t)((Unlen &0x7F) | 0x80);    /* ��ȡ��7λ����ֵ���ټ���128 */
                Unlen = (Unlen) >>7;        /* ������������7λ��������� */
                out_len++;              /* ������ݰ����ȵ��� */
            }
            *(++pOutData) = ((uint8_t)Unlen) & 0x7F;    /* ��ȡ�������ݳ������7λ��ֵ */
            out_len++;                   /* ������ݰ����ȵ��� */
            for(i=0; i<temp_len; i++)
            {
                *(++pOutData) = *pdata++;   /* ��װ�������� */
                out_len++;               /* ������ݰ����ȵ��� */
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
                *(++pOutData) = *pdata++;      /* 32λ��ֵ��� */
            }
            --*pOutData;
            out_len += 4;      /* ������ݰ����ȼ�4 */

            break;
        }
        default:
            break;
    }
    return  out_len;
}







