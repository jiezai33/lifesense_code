#ifndef _MD5_H_
#define _MD5_H_
/***********************************************************************************
 * �� �� ��   : Md5.h
 * �� �� ��   : zhangfei
 * ��������   : 2016��3��24��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : Md5.c ͷ�ļ�
 * �޸���ʷ   : 
                
***********************************************************************************/

/*
 * This code has some adaptations for the Ghostscript environment, but it
 * will compile and run correctly in any environment with 8-bit chars and
 * 32-bit ints.  Specifically, it assumes that if the following are
 * defined, they have the same meaning as in Ghostscript: P1, P2, P3,
 * ARCH_IS_BIG_ENDIAN.
 */
typedef unsigned char        u8_t;
typedef signed char          s8_t;

typedef unsigned int         u16_t;
typedef signed int           s16_t;

typedef unsigned long int    u32_t;
typedef signed long int      s32_t;

typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_int_t; /* 16-bit word */
typedef unsigned long md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;

#ifdef __cplusplus
extern "C" 
{
#endif
void Md5_test( void );
/* Initialize the algorithm. */
void md5_init(md5_state_t *pms);

/* Append a string to the message. */
void md5_append(md5_state_t *pms, const md5_byte_t *data, long int nbytes);           //2014030301


/* Finish the message and return the digest. */
void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);

void md5_Code(md5_byte_t *pDataIn, md5_byte_t ucDataInLen, md5_byte_t ucDataOut[16]);

#ifdef __cplusplus
}  /* end extern "C" */
#endif


#endif
