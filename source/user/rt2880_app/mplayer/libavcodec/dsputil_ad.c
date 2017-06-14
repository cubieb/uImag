/*
 * DSP utils
 * Copyright (c) 2000, 2001 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * gmc & q-pel & 32/64 bit based MC by Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file dsputil.c
 * DSP utils
 */

#include "avcodec.h"
#include "dsputil.h"

/* vorbis.c */
void vorbis_inverse_coupling(float *mag, float *ang, int blocksize);


uint8_t ff_cropTbl[256 + 2 * MAX_NEG_CROP] = {0, };
uint32_t ff_squareTbl[512] = {0, };


const uint8_t ff_alternate_horizontal_scan[64] = {
    0,  1,   2,  3,  8,  9, 16, 17,
    10, 11,  4,  5,  6,  7, 15, 14,
    13, 12, 19, 18, 24, 25, 32, 33,
    26, 27, 20, 21, 22, 23, 28, 29,
    30, 31, 34, 35, 40, 41, 48, 49,
    42, 43, 36, 37, 38, 39, 44, 45,
    46, 47, 50, 51, 56, 57, 58, 59,
    52, 53, 54, 55, 60, 61, 62, 63,
};

const uint8_t ff_alternate_vertical_scan[64] = {
    0,  8,  16, 24,  1,  9,  2, 10,
    17, 25, 32, 40, 48, 56, 57, 49,
    41, 33, 26, 18,  3, 11,  4, 12,
    19, 27, 34, 42, 50, 58, 35, 43,
    51, 59, 20, 28,  5, 13,  6, 14,
    21, 29, 36, 44, 52, 60, 37, 45,
    53, 61, 22, 30,  7, 15, 23, 31,
    38, 46, 54, 62, 39, 47, 55, 63,
};

/* a*inverse[b]>>32 == a/b for all 0<=a<=65536 && 2<=b<=255 */
const uint32_t ff_inverse[256]={
         0, 4294967295U,2147483648U,1431655766, 1073741824,  858993460,  715827883,  613566757,
 536870912,  477218589,  429496730,  390451573,  357913942,  330382100,  306783379,  286331154,
 268435456,  252645136,  238609295,  226050911,  214748365,  204522253,  195225787,  186737709,
 178956971,  171798692,  165191050,  159072863,  153391690,  148102321,  143165577,  138547333,
 134217728,  130150525,  126322568,  122713352,  119304648,  116080198,  113025456,  110127367,
 107374183,  104755300,  102261127,   99882961,   97612894,   95443718,   93368855,   91382283,
  89478486,   87652394,   85899346,   84215046,   82595525,   81037119,   79536432,   78090315,
  76695845,   75350304,   74051161,   72796056,   71582789,   70409300,   69273667,   68174085,
  67108864,   66076420,   65075263,   64103990,   63161284,   62245903,   61356676,   60492498,
  59652324,   58835169,   58040099,   57266231,   56512728,   55778797,   55063684,   54366675,
  53687092,   53024288,   52377650,   51746594,   51130564,   50529028,   49941481,   49367441,
  48806447,   48258060,   47721859,   47197443,   46684428,   46182445,   45691142,   45210183,
  44739243,   44278014,   43826197,   43383509,   42949673,   42524429,   42107523,   41698712,
  41297763,   40904451,   40518560,   40139882,   39768216,   39403370,   39045158,   38693400,
  38347923,   38008561,   37675152,   37347542,   37025581,   36709123,   36398028,   36092163,
  35791395,   35495598,   35204650,   34918434,   34636834,   34359739,   34087043,   33818641,
  33554432,   33294321,   33038210,   32786010,   32537632,   32292988,   32051995,   31814573,
  31580642,   31350127,   31122952,   30899046,   30678338,   30460761,   30246249,   30034737,
  29826162,   29620465,   29417585,   29217465,   29020050,   28825284,   28633116,   28443493,
  28256364,   28071682,   27889399,   27709467,   27531842,   27356480,   27183338,   27012373,
  26843546,   26676816,   26512144,   26349493,   26188825,   26030105,   25873297,   25718368,
  25565282,   25414008,   25264514,   25116768,   24970741,   24826401,   24683721,   24542671,
  24403224,   24265352,   24129030,   23994231,   23860930,   23729102,   23598722,   23469767,
  23342214,   23216040,   23091223,   22967740,   22845571,   22724695,   22605092,   22486740,
  22369622,   22253717,   22139007,   22025474,   21913099,   21801865,   21691755,   21582751,
  21474837,   21367997,   21262215,   21157475,   21053762,   20951060,   20849356,   20748635,
  20648882,   20550083,   20452226,   20355296,   20259280,   20164166,   20069941,   19976593,
  19884108,   19792477,   19701685,   19611723,   19522579,   19434242,   19346700,   19259944,
  19173962,   19088744,   19004281,   18920561,   18837576,   18755316,   18673771,   18592933,
  18512791,   18433337,   18354562,   18276457,   18199014,   18122225,   18046082,   17970575,
  17895698,   17821442,   17747799,   17674763,   17602325,   17530479,   17459217,   17388532,
  17318417,   17248865,   17179870,   17111424,   17043522,   16976156,   16909321,   16843010,
};

/* Input permutation for the simple_idct_mmx */
static const uint8_t simple_mmx_permutation[64]={
        0x00, 0x08, 0x04, 0x09, 0x01, 0x0C, 0x05, 0x0D,
        0x10, 0x18, 0x14, 0x19, 0x11, 0x1C, 0x15, 0x1D,
        0x20, 0x28, 0x24, 0x29, 0x21, 0x2C, 0x25, 0x2D,
        0x12, 0x1A, 0x16, 0x1B, 0x13, 0x1E, 0x17, 0x1F,
        0x02, 0x0A, 0x06, 0x0B, 0x03, 0x0E, 0x07, 0x0F,
        0x30, 0x38, 0x34, 0x39, 0x31, 0x3C, 0x35, 0x3D,
        0x22, 0x2A, 0x26, 0x2B, 0x23, 0x2E, 0x27, 0x2F,
        0x32, 0x3A, 0x36, 0x3B, 0x33, 0x3E, 0x37, 0x3F,
};



static void bswap_buf(uint32_t *dst, uint32_t *src, int w){
    int i;

    for(i=0; i+8<=w; i+=8){
        dst[i+0]= bswap_32(src[i+0]);
        dst[i+1]= bswap_32(src[i+1]);
        dst[i+2]= bswap_32(src[i+2]);
        dst[i+3]= bswap_32(src[i+3]);
        dst[i+4]= bswap_32(src[i+4]);
        dst[i+5]= bswap_32(src[i+5]);
        dst[i+6]= bswap_32(src[i+6]);
        dst[i+7]= bswap_32(src[i+7]);
    }
    for(;i<w; i++){
        dst[i+0]= bswap_32(src[i+0]);
    }
}

static void add_bytes_c(uint8_t *dst, uint8_t *src, int w){
    int i;
    for(i=0; i+7<w; i+=8){
        dst[i+0] += src[i+0];
        dst[i+1] += src[i+1];
        dst[i+2] += src[i+2];
        dst[i+3] += src[i+3];
        dst[i+4] += src[i+4];
        dst[i+5] += src[i+5];
        dst[i+6] += src[i+6];
        dst[i+7] += src[i+7];
    }
    for(; i<w; i++)
        dst[i+0] += src[i+0];
}

static void diff_bytes_c(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w){
    int i;
    for(i=0; i+7<w; i+=8){
        dst[i+0] = src1[i+0]-src2[i+0];
        dst[i+1] = src1[i+1]-src2[i+1];
        dst[i+2] = src1[i+2]-src2[i+2];
        dst[i+3] = src1[i+3]-src2[i+3];
        dst[i+4] = src1[i+4]-src2[i+4];
        dst[i+5] = src1[i+5]-src2[i+5];
        dst[i+6] = src1[i+6]-src2[i+6];
        dst[i+7] = src1[i+7]-src2[i+7];
    }
    for(; i<w; i++)
        dst[i+0] = src1[i+0]-src2[i+0];
}

static void just_return(void *mem av_unused, int stride av_unused, int h av_unused) { return; }

static void vector_fmul_c(float *dst, const float *src, int len){
    int i;
    for(i=0; i<len; i++)
        dst[i] *= src[i];
}

static void vector_fmul_reverse_c(float *dst, const float *src0, const float *src1, int len){
    int i;
    src1 += len-1;
    for(i=0; i<len; i++)
        dst[i] = src0[i] * src1[-i];
}

void ff_vector_fmul_add_add_c(float *dst, const float *src0, const float *src1, const float *src2, int src3, int len, int step){
    int i;
    for(i=0; i<len; i++)
        dst[i*step] = src0[i] * src1[i] + src2[i] + src3;
}

void ff_float_to_int16_c(int16_t *dst, const float *src, int len){
    int i;
    for(i=0; i<len; i++) {
        int_fast32_t tmp = ((int32_t*)src)[i];
        if(tmp & 0xf0000){
            tmp = (0x43c0ffff - tmp)>>31;
            // is this faster on some gcc/cpu combinations?
//          if(tmp > 0x43c0ffff) tmp = 0xFFFF;
//          else                 tmp = 0;
        }
        dst[i] = tmp - 0x8000;
    }
}


int ff_check_alignment(void){
    static int did_fail=0;
    DECLARE_ALIGNED_16(int, aligned);

    if((long)&aligned & 15){
        if(!did_fail){
#if defined(HAVE_MMX) || defined(HAVE_ALTIVEC)
            av_log(NULL, AV_LOG_ERROR,
                "Compiler did not align stack variables. Libavcodec has been miscompiled\n"
                "and may be very slow or crash. This is not a bug in libavcodec,\n"
                "but in the compiler. You may try recompiling using gcc >= 4.2.\n"
                "Do not report crashes to FFmpeg developers.\n");
#endif
            did_fail=1;
        }
        return -1;
    }
    return 0;
}

void dsputil_init(DSPContext* c, AVCodecContext *avctx)
{
    int i;

    ff_check_alignment();

  
    c->add_bytes= add_bytes_c;
    c->diff_bytes= diff_bytes_c;
    c->bswap_buf= bswap_buf;

#if CONFIG_VORBIS_DECODER
    c->vorbis_inverse_coupling = vorbis_inverse_coupling;
#endif

    c->vector_fmul = vector_fmul_c;
    c->vector_fmul_reverse = vector_fmul_reverse_c;
    c->vector_fmul_add_add = ff_vector_fmul_add_add_c;
    c->float_to_int16 = ff_float_to_int16_c;

    c->prefetch= just_return;

    
   
  
}

