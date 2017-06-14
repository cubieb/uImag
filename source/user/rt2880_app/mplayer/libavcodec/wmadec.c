/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file wmadec.c
 * WMA compatible decoder.
 * This decoder handles Microsoft Windows Media Audio data, versions 1 & 2.
 * WMA v1 is identified by audio format 0x160 in Microsoft media files
 * (ASF/AVI/WAV). WMA v2 is identified by audio format 0x161.
 *
 * To use this decoder, a calling application must supply the extra data
 * bytes provided with the WMA data. These are the extra, codec-specific
 * bytes at the end of a WAVEFORMATEX data structure. Transmit these bytes
 * to the decoder using the extradata[_size] fields in AVCodecContext. There
 * should be 4 extra bytes for v1 data and 6 extra bytes for v2 data.
 */
 /*
  * Fixed point modifications by Derek Taubert (taubert@geeks.org) Feb 2005
  * Extra fixed point modifications by Peter McQuillan Jan 2006
 */

#define DEBUG
#include <assert.h>
#include "avcodec.h"
#include "dsputil.h"
//#include <math.h>
#include "wma.h"


#include "wmadata.h"

#define hgain_huffbits	ff_wma_hgain_huffbits
#define hgain_huffcodes	ff_wma_hgain_huffcodes
#define scale_huffbits ff_wma_scale_huffbits
#define scale_huffcodes ff_wma_scale_huffcodes
#define lsp_codebook ff_wma_lsp_codebook

// deprecated, dont use get_vlc for new code, use get_vlc2 instead or use GET_VLC directly
static inline int get_vlc(GetBitContext *s, VLC *vlc)
{
    int code;
    VLC_TYPE (*table)[2]= vlc->table;
    
    OPEN_READER(re, s)
    UPDATE_CACHE(re, s)

    GET_VLC(code, re, s, table, vlc->bits, 3)    

    CLOSE_READER(re, s)
    return code;
}

#ifdef TRACE
static void dump_shorts(const char *name, const short *tab, int n)
{
    int i;

    tprintf("%s[%d]:\n", name, n);
    for(i=0;i<n;i++) {
        if ((i & 7) == 0)
            tprintf("%4d: ", i);
        tprintf(" %5d.0", tab[i]);
        if ((i & 7) == 7)
            tprintf("\n");
    }
}

static void dump_words(const char *name, const int32_t *tab, int n)
{
    int i;

    tprintf("%s[%d]:\n", name, n);
    for(i=0;i<n;i++) {
        if ((i & 7) == 0)
            tprintf("%4d: ", i);
        tprintf(" %10d.0", tab[i]);
        if ((i & 7) == 7)
            tprintf("\n");
    }
}

static void dump_floats(const char *name, int prec, const float *tab, int n)
{
    int i;

    tprintf("%s[%d]:\n", name, n);
    for(i=0;i<n;i++) {
        if ((i & 7) == 0)
            tprintf("%4d: ", i);
        tprintf(" %8.*f", prec, tab[i]);
        if ((i & 7) == 7)
            tprintf("\n");
    }
    if ((i & 7) != 0)
        tprintf("\n");
}
#endif

/* XXX: use same run/length optimization as mpeg decoders */
static void init_coef_vlc(VLC *vlc,
                          uint16_t **prun_table, uint16_t **plevel_table,
                          const CoefVLCTable *vlc_table)
{
    int n = vlc_table->n;
    const uint8_t *table_bits = vlc_table->huffbits;
    const uint32_t *table_codes = vlc_table->huffcodes;
    const uint16_t *levels_table = vlc_table->levels;
    uint16_t *run_table, *level_table;
    const uint16_t *p;
    int i, l, j, level;

    init_vlc(vlc, 9, n, table_bits, 1, 1, table_codes, 4, 4, 0);

    run_table = (uint16_t *)malloc(n * sizeof(uint16_t));
    level_table = (uint16_t *)malloc(n * sizeof(uint16_t));
    p = levels_table;
    i = 2;
    level = 1;
    while (i < n) {
        l = *p++;
        for(j=0;j<l;j++) {
            run_table[i] = j;
            level_table[i] = level;
            i++;
        }
        level++;
    }
    *prun_table = run_table;
    *plevel_table = level_table;
}

static int wma_decode_init(AVCodecContext * avctx)
{
    WMADecodeContext *s = (WMADecodeContext *)avctx->priv_data;
    int i, flags1, flags2;
    int32_t *window;
    uint8_t *extradata;
    float bps1, high_freq;
    volatile float bps;
    int sample_rate1;
    int coef_vlc_table;

    s->sample_rate = avctx->sample_rate;
    s->nb_channels = avctx->channels;
    s->bit_rate = avctx->bit_rate;
    s->block_align = avctx->block_align;

    if (avctx->codec->id == CODEC_ID_WMAV1) {
        s->version = 1;
    } else {
        s->version = 2;
    }

    /* extract flag infos */
    flags1 = 0;
    flags2 = 0;
    extradata = (uint8_t *)avctx->extradata;
    if (s->version == 1 && avctx->extradata_size >= 4) {
        flags1 = extradata[0] | (extradata[1] << 8);
        flags2 = extradata[2] | (extradata[3] << 8);
    } else if (s->version == 2 && avctx->extradata_size >= 6) {
        flags1 = extradata[0] | (extradata[1] << 8) |
            (extradata[2] << 16) | (extradata[3] << 24);
        flags2 = extradata[4] | (extradata[5] << 8);
    }
    s->use_exp_vlc = flags2 & 0x0001;
    s->use_bit_reservoir = flags2 & 0x0002;
    s->use_variable_block_len = flags2 & 0x0004;

    /* compute MDCT block size */
    if (s->sample_rate <= 16000) {
        s->frame_len_bits = 9;
    } else if (s->sample_rate <= 22050 ||
               (s->sample_rate <= 32000 && s->version == 1)) {
        s->frame_len_bits = 10;
    } else {
        s->frame_len_bits = 11;
    }


    s->frame_len = 1 << s->frame_len_bits;
    if (s->use_variable_block_len) {
        int nb_max, nb;
        nb = ((flags2 >> 3) & 3) + 1;
        if ((s->bit_rate / s->nb_channels) >= 32000)
            nb += 2;
        nb_max = s->frame_len_bits - BLOCK_MIN_BITS;
        if (nb > nb_max)
            nb = nb_max;
        s->nb_block_sizes = nb + 1;
    } else {
        s->nb_block_sizes = 1;
    }


    /* init rate dependant parameters */
    s->use_noise_coding = 1;
    high_freq = s->sample_rate * 0.5;

    /* if version 2, then the rates are normalized */
    sample_rate1 = s->sample_rate;
    if (s->version == 2) {
        if (sample_rate1 >= 44100)
            sample_rate1 = 44100;
        else if (sample_rate1 >= 22050)
            sample_rate1 = 22050;
        else if (sample_rate1 >= 16000)
            sample_rate1 = 16000;
        else if (sample_rate1 >= 11025)
            sample_rate1 = 11025;
        else if (sample_rate1 >= 8000)
            sample_rate1 = 8000;
    }

    bps = (float)s->bit_rate / (float)(s->nb_channels * s->sample_rate);
    s->byte_offset_bits = av_log2((int)(bps * s->frame_len / 8.0)) + 2;

    /* compute high frequency value and choose if noise coding should
       be activated */
    bps1 = bps;
    if (s->nb_channels == 2)
        bps1 = bps * 1.6;
    if (sample_rate1 == 44100) {
        if (bps1 >= 0.61)
            s->use_noise_coding = 0;
        else
            high_freq = high_freq * 0.4;
    } else if (sample_rate1 == 22050) {
        if (bps1 >= 1.16)
            s->use_noise_coding = 0;
        else if (bps1 >= 0.72)
            high_freq = high_freq * 0.7;
        else
            high_freq = high_freq * 0.6;
    } else if (sample_rate1 == 16000) {
        if (bps > 0.5)
            high_freq = high_freq * 0.5;
        else
            high_freq = high_freq * 0.3;
    } else if (sample_rate1 == 11025) {
        high_freq = high_freq * 0.7;
    } else if (sample_rate1 == 8000) {
        if (bps <= 0.625) {
            high_freq = high_freq * 0.5;
        } else if (bps > 0.75) {
            s->use_noise_coding = 0;
        } else {
            high_freq = high_freq * 0.65;
        }
    } else {
        if (bps >= 0.8) {
            high_freq = high_freq * 0.75;
        } else if (bps >= 0.6) {
            high_freq = high_freq * 0.6;
        } else {
            high_freq = high_freq * 0.5;
        }
    }
/*
    dprintf("flags1=0x%x flags2=0x%x\n", flags1, flags2);
    dprintf("version=%d channels=%d sample_rate=%d bitrate=%d block_align=%d\n",
           s->version, s->nb_channels, s->sample_rate, s->bit_rate,
           s->block_align);
    dprintf("bps=%f bps1=%f high_freq=%f bitoffset=%d\n",
           bps, bps1, high_freq, s->byte_offset_bits);
    dprintf("use_noise_coding=%d use_exp_vlc=%d nb_block_sizes=%d\n",
           s->use_noise_coding, s->use_exp_vlc, s->nb_block_sizes);
*/
    /* compute the scale factor band sizes for each MDCT block size */
    {
        int a, b, pos, lpos, k, block_len, i, j, n;
        const uint8_t *table;

        if (s->version == 1) {
            s->coefs_start = 3;
        } else {
            s->coefs_start = 0;
        }
        for(k = 0; k < s->nb_block_sizes; k++) {
            block_len = s->frame_len >> k;

            if (s->version == 1) {
                lpos = 0;
                for(i=0;i<25;i++) {
                    a = wma_critical_freqs[i];
                    b = s->sample_rate;
                    pos = ((block_len * 2 * a)  + (b >> 1)) / b;
                    if (pos > block_len)
                        pos = block_len;
                    s->exponent_bands[0][i] = pos - lpos;
                    if (pos >= block_len) {
                        i++;
                        break;
                    }
                    lpos = pos;
                }
                s->exponent_sizes[0] = i;
            } else {
                /* hardcoded tables */
                table = NULL;
                a = s->frame_len_bits - BLOCK_MIN_BITS - k;
                if (a < 3) {
                    if (s->sample_rate >= 44100)
                        table = exponent_band_44100[a];
                    else if (s->sample_rate >= 32000)
                        table = exponent_band_32000[a];
                    else if (s->sample_rate >= 22050)
                        table = exponent_band_22050[a];
                }
                if (table) {
                    n = *table++;
                    for(i=0;i<n;i++)
                        s->exponent_bands[k][i] = table[i];
                    s->exponent_sizes[k] = n;
                } else {
                    j = 0;
                    lpos = 0;
                    for(i=0;i<25;i++) {
                        a = wma_critical_freqs[i];
                        b = s->sample_rate;
                        pos = ((block_len * 2 * a)  + (b << 1)) / (4 * b);
                        pos <<= 2;
                        if (pos > block_len)
                            pos = block_len;
                        if (pos > lpos)
                            s->exponent_bands[k][j++] = pos - lpos;
                        if (pos >= block_len)
                            break;
                        lpos = pos;
                    }
                    s->exponent_sizes[k] = j;
                }
            }

            /* max number of coefs */
            s->coefs_end[k] = (s->frame_len - ((s->frame_len * 9) / 100)) >> k;
            /* high freq computation */
            s->high_band_start[k] = (int)((block_len * 2 * high_freq) /
                                          s->sample_rate + 0.5);
            n = s->exponent_sizes[k];
            j = 0;
            pos = 0;
            for(i=0;i<n;i++) {
                int start, end;
                start = pos;
                pos += s->exponent_bands[k][i];
                end = pos;
                if (start < s->high_band_start[k])
                    start = s->high_band_start[k];
                if (end > s->coefs_end[k])
                    end = s->coefs_end[k];
                if (end > start)
                    s->exponent_high_bands[k][j++] = end - start;
            }
            s->exponent_high_sizes[k] = j;
#if 0
            tprintf("%5d: coefs_end=%d high_band_start=%d nb_high_bands=%d: ",
                  s->frame_len >> k,
                  s->coefs_end[k],
                  s->high_band_start[k],
                  s->exponent_high_sizes[k]);
            for(j=0;j<s->exponent_high_sizes[k];j++)
                tprintf(" %d", s->exponent_high_bands[k][j]);
            tprintf("\n");
#endif
        }
    }

#ifdef TRACE
    {
        int i, j;
        for(i = 0; i < s->nb_block_sizes; i++) {
            tprintf("%5d: n=%2d:",
                   s->frame_len >> i,
                   s->exponent_sizes[i]);
            for(j=0;j<s->exponent_sizes[i];j++)
                tprintf(" %d", s->exponent_bands[i][j]);
            tprintf("\n");
        }
    }
#endif

    /* init MDCT windows : simple sinus window */
    for(i = 0; i < s->nb_block_sizes; i++) {
        int n, j;
        float alpha;
        n = 1 << (s->frame_len_bits - i);
        window = (int32_t *)malloc(sizeof(int32_t) * n);
        alpha = M_PI / (2.0 * n);
        for(j=0;j<n;j++) {
	    window[n - j - 1] = NFPTOFXS(sin((j + 0.5) * alpha), 31);
        }
        s->windows[i] = window;
    }

    s->reset_block_lengths = 1;

    if (s->use_noise_coding) {

        /* init the noise generator */
        if (s->use_exp_vlc)
            s->noise_mult = 0.02;
        else
            s->noise_mult = 0.04;

#ifdef TRACE
        for(i=0;i<NOISE_TAB_SIZE;i++) {
            s->noise_table[i] = 512.0 * s->noise_mult;
	    s->inoise_table[i] = FPTOFXS(s->noise_table[i], 16);
	}
#else
        {
            unsigned int seed;
            float norm;
            seed = 1;
            norm = (512.0 / (float)(1LL << 31)) * sqrt(3) * s->noise_mult;
            for(i=0;i<NOISE_TAB_SIZE;i++) {
                seed = seed * 314159 + 1;
                s->noise_table[i] = (float)((int)seed) * norm;
		s->inoise_table[i] = FPTOFXS(s->noise_table[i], 16);
            }
        }
#endif
        init_vlc(&s->hgain_vlc, 9, sizeof(hgain_huffbits),
                 hgain_huffbits, 1, 1,
                 hgain_huffcodes, 2, 2, 0);
    }

    if (s->use_exp_vlc) {
	wma_exp_vlc_init(s);
        init_vlc(&s->exp_vlc, 9, sizeof(scale_huffbits),
                 scale_huffbits, 1, 1,
                 scale_huffcodes, 4, 4, 0);
    } else {
        wma_lsp_to_curve_init(s, s->frame_len);
    }

    /* choose the VLC tables for the coefficients */
    coef_vlc_table = 2;
    if (s->sample_rate >= 32000) {
        if (bps1 < 0.72)
            coef_vlc_table = 0;
        else if (bps1 < 1.16)
            coef_vlc_table = 1;
    }

    init_coef_vlc(&s->coef_vlc[0], &s->run_table[0], &s->level_table[0],
                  &coef_vlcs[coef_vlc_table * 2]);
    init_coef_vlc(&s->coef_vlc[1], &s->run_table[1], &s->level_table[1],
                  &coef_vlcs[coef_vlc_table * 2 + 1]);
             
    return 0;
}

/* interpolate values for a bigger or smaller block. The block must
   have multiple sizes */
static void interpolate_array(float *scale, int old_size, int new_size)
{
    int i, j, jincr, k;
    float v;

    if (new_size > old_size) {
        jincr = new_size / old_size;
        j = new_size;
        for(i = old_size - 1; i >=0; i--) {
            v = scale[i];
            k = jincr;
            do {
                scale[--j] = v;
            } while (--k);
        }
    } else if (new_size < old_size) {
        j = 0;
        jincr = old_size / new_size;
        for(i = 0; i < new_size; i++) {
            scale[i] = scale[j];
            j += jincr;
        }
    }
}

static void iinterpolate_array(uint32_t *scale, int old_size, int new_size)
{
    int i, j, jincr, k;
    uint32_t v;

    if (new_size > old_size) {
        jincr = new_size / old_size;
        j = new_size;
        for(i = old_size - 1; i >=0; i--) {
            v = scale[i];
            k = jincr;
            do {
                scale[--j] = v;
            } while (--k);
        }
    } else if (new_size < old_size) {
        j = 0;
        jincr = old_size / new_size;
        for(i = 0; i < new_size; i++) {
            scale[i] = scale[j];
            j += jincr;
        }
    }
}

/* compute x^-0.25 with an exponent and mantissa table. We use linear
   interpolation to reduce the mantissa table size at a small speed
   expense (linear interpolation approximately doubles the number of
   bits of precision). */
static inline float pow_m1_4(WMADecodeContext *s, float x)
{
    union {
        float f;
        unsigned int v;
    } u, t;
    unsigned int e, m;
    float a, b;

    u.f = x;
    e = u.v >> 23;
    m = (u.v >> (23 - LSP_POW_BITS)) & ((1 << LSP_POW_BITS) - 1);
    /* build interpolation scale: 1 <= t < 2. */
    t.v = ((u.v << LSP_POW_BITS) & ((1 << 23) - 1)) | (127 << 23);
    a = s->lsp_pow_m_table1[m];
    b = s->lsp_pow_m_table2[m];
    return s->lsp_pow_e_table[e] * (a + b * t.f);
}

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len)
{
    float wdel;
    int i;
    
    wdel = M_PI / frame_len;
    for(i=0;i<frame_len;i++)
        s->lsp_cos_table[i] = 2.0f * cos(wdel * i);

    /* tables for x^-0.25 computation */
	/* Replaced by pre-calculated values */

        s->lsp_pow_e_table[0] = 3037000499.976050;
        s->lsp_pow_e_table[1] = 2553802833.553599;
        s->lsp_pow_e_table[2] = 2147483648.000000;
        s->lsp_pow_e_table[3] = 1805811301.419170;
        s->lsp_pow_e_table[4] = 1518500249.988025;
        s->lsp_pow_e_table[5] = 1276901416.776799;
        s->lsp_pow_e_table[6] = 1073741824.000000;
        s->lsp_pow_e_table[7] = 902905650.709585;
        s->lsp_pow_e_table[8] = 759250124.994012;
        s->lsp_pow_e_table[9] = 638450708.388400;
        s->lsp_pow_e_table[10] = 536870912.000000;
        s->lsp_pow_e_table[11] = 451452825.354792;
        s->lsp_pow_e_table[12] = 379625062.497006;
        s->lsp_pow_e_table[13] = 319225354.194200;
        s->lsp_pow_e_table[14] = 268435456.000000;
        s->lsp_pow_e_table[15] = 225726412.677396;
        s->lsp_pow_e_table[16] = 189812531.248503;
        s->lsp_pow_e_table[17] = 159612677.097100;
        s->lsp_pow_e_table[18] = 134217728.000000;
        s->lsp_pow_e_table[19] = 112863206.338698;
        s->lsp_pow_e_table[20] = 94906265.624252;
        s->lsp_pow_e_table[21] = 79806338.548550;
        s->lsp_pow_e_table[22] = 67108864.000000;
        s->lsp_pow_e_table[23] = 56431603.169349;
        s->lsp_pow_e_table[24] = 47453132.812126;
        s->lsp_pow_e_table[25] = 39903169.274275;
        s->lsp_pow_e_table[26] = 33554432.000000;
        s->lsp_pow_e_table[27] = 28215801.584675;
        s->lsp_pow_e_table[28] = 23726566.406063;
        s->lsp_pow_e_table[29] = 19951584.637137;
        s->lsp_pow_e_table[30] = 16777216.000000;
        s->lsp_pow_e_table[31] = 14107900.792337;
        s->lsp_pow_e_table[32] = 11863283.203031;
        s->lsp_pow_e_table[33] = 9975792.318569;
        s->lsp_pow_e_table[34] = 8388608.000000;
        s->lsp_pow_e_table[35] = 7053950.396169;
        s->lsp_pow_e_table[36] = 5931641.601516;
        s->lsp_pow_e_table[37] = 4987896.159284;
        s->lsp_pow_e_table[38] = 4194304.000000;
        s->lsp_pow_e_table[39] = 3526975.198084;
        s->lsp_pow_e_table[40] = 2965820.800758;
        s->lsp_pow_e_table[41] = 2493948.079642;
        s->lsp_pow_e_table[42] = 2097152.000000;
        s->lsp_pow_e_table[43] = 1763487.599042;
        s->lsp_pow_e_table[44] = 1482910.400379;
        s->lsp_pow_e_table[45] = 1246974.039821;
        s->lsp_pow_e_table[46] = 1048576.000000;
        s->lsp_pow_e_table[47] = 881743.799521;
        s->lsp_pow_e_table[48] = 741455.200189;
        s->lsp_pow_e_table[49] = 623487.019911;
        s->lsp_pow_e_table[50] = 524288.000000;
        s->lsp_pow_e_table[51] = 440871.899761;
        s->lsp_pow_e_table[52] = 370727.600095;
        s->lsp_pow_e_table[53] = 311743.509955;
        s->lsp_pow_e_table[54] = 262144.000000;
        s->lsp_pow_e_table[55] = 220435.949880;
        s->lsp_pow_e_table[56] = 185363.800047;
        s->lsp_pow_e_table[57] = 155871.754978;
        s->lsp_pow_e_table[58] = 131072.000000;
        s->lsp_pow_e_table[59] = 110217.974940;
        s->lsp_pow_e_table[60] = 92681.900024;
        s->lsp_pow_e_table[61] = 77935.877489;
        s->lsp_pow_e_table[62] = 65536.000000;
        s->lsp_pow_e_table[63] = 55108.987470;
        s->lsp_pow_e_table[64] = 46340.950012;
        s->lsp_pow_e_table[65] = 38967.938744;
        s->lsp_pow_e_table[66] = 32768.000000;
        s->lsp_pow_e_table[67] = 27554.493735;
        s->lsp_pow_e_table[68] = 23170.475006;
        s->lsp_pow_e_table[69] = 19483.969372;
        s->lsp_pow_e_table[70] = 16384.000000;
        s->lsp_pow_e_table[71] = 13777.246868;
        s->lsp_pow_e_table[72] = 11585.237503;
        s->lsp_pow_e_table[73] = 9741.984686;
        s->lsp_pow_e_table[74] = 8192.000000;
        s->lsp_pow_e_table[75] = 6888.623434;
        s->lsp_pow_e_table[76] = 5792.618751;
        s->lsp_pow_e_table[77] = 4870.992343;
        s->lsp_pow_e_table[78] = 4096.000000;
        s->lsp_pow_e_table[79] = 3444.311717;
        s->lsp_pow_e_table[80] = 2896.309376;
        s->lsp_pow_e_table[81] = 2435.496172;
        s->lsp_pow_e_table[82] = 2048.000000;
        s->lsp_pow_e_table[83] = 1722.155858;
        s->lsp_pow_e_table[84] = 1448.154688;
        s->lsp_pow_e_table[85] = 1217.748086;
        s->lsp_pow_e_table[86] = 1024.000000;
        s->lsp_pow_e_table[87] = 861.077929;
        s->lsp_pow_e_table[88] = 724.077344;
        s->lsp_pow_e_table[89] = 608.874043;
        s->lsp_pow_e_table[90] = 512.000000;
        s->lsp_pow_e_table[91] = 430.538965;
        s->lsp_pow_e_table[92] = 362.038672;
        s->lsp_pow_e_table[93] = 304.437021;
        s->lsp_pow_e_table[94] = 256.000000;
        s->lsp_pow_e_table[95] = 215.269482;
        s->lsp_pow_e_table[96] = 181.019336;
        s->lsp_pow_e_table[97] = 152.218511;
        s->lsp_pow_e_table[98] = 128.000000;
        s->lsp_pow_e_table[99] = 107.634741;
        s->lsp_pow_e_table[100] = 90.509668;
        s->lsp_pow_e_table[101] = 76.109255;
        s->lsp_pow_e_table[102] = 64.000000;
        s->lsp_pow_e_table[103] = 53.817371;
        s->lsp_pow_e_table[104] = 45.254834;
        s->lsp_pow_e_table[105] = 38.054628;
        s->lsp_pow_e_table[106] = 32.000000;
        s->lsp_pow_e_table[107] = 26.908685;
        s->lsp_pow_e_table[108] = 22.627417;
        s->lsp_pow_e_table[109] = 19.027314;
        s->lsp_pow_e_table[110] = 16.000000;
        s->lsp_pow_e_table[111] = 13.454343;
        s->lsp_pow_e_table[112] = 11.313708;
        s->lsp_pow_e_table[113] = 9.513657;
        s->lsp_pow_e_table[114] = 8.000000;
        s->lsp_pow_e_table[115] = 6.727171;
        s->lsp_pow_e_table[116] = 5.656854;
        s->lsp_pow_e_table[117] = 4.756828;
        s->lsp_pow_e_table[118] = 4.000000;
        s->lsp_pow_e_table[119] = 3.363586;
        s->lsp_pow_e_table[120] = 2.828427;
        s->lsp_pow_e_table[121] = 2.378414;
        s->lsp_pow_e_table[122] = 2.000000;
        s->lsp_pow_e_table[123] = 1.681793;
        s->lsp_pow_e_table[124] = 1.414214;
        s->lsp_pow_e_table[125] = 1.189207;
        s->lsp_pow_e_table[126] = 1.000000;
        s->lsp_pow_e_table[127] = 0.840896;
        s->lsp_pow_e_table[128] = 0.707107;
        s->lsp_pow_e_table[129] = 0.594604;
        s->lsp_pow_e_table[130] = 0.500000;
        s->lsp_pow_e_table[131] = 0.420448;
        s->lsp_pow_e_table[132] = 0.353553;
        s->lsp_pow_e_table[133] = 0.297302;
        s->lsp_pow_e_table[134] = 0.250000;
        s->lsp_pow_e_table[135] = 0.210224;
        s->lsp_pow_e_table[136] = 0.176777;
        s->lsp_pow_e_table[137] = 0.148651;
        s->lsp_pow_e_table[138] = 0.125000;
        s->lsp_pow_e_table[139] = 0.105112;
        s->lsp_pow_e_table[140] = 0.088388;
        s->lsp_pow_e_table[141] = 0.074325;
        s->lsp_pow_e_table[142] = 0.062500;
        s->lsp_pow_e_table[143] = 0.052556;
        s->lsp_pow_e_table[144] = 0.044194;
        s->lsp_pow_e_table[145] = 0.037163;
        s->lsp_pow_e_table[146] = 0.031250;
        s->lsp_pow_e_table[147] = 0.026278;
        s->lsp_pow_e_table[148] = 0.022097;
        s->lsp_pow_e_table[149] = 0.018581;
        s->lsp_pow_e_table[150] = 0.015625;
        s->lsp_pow_e_table[151] = 0.013139;
        s->lsp_pow_e_table[152] = 0.011049;
        s->lsp_pow_e_table[153] = 0.009291;
        s->lsp_pow_e_table[154] = 0.007812;
        s->lsp_pow_e_table[155] = 0.006570;
        s->lsp_pow_e_table[156] = 0.005524;
        s->lsp_pow_e_table[157] = 0.004645;
        s->lsp_pow_e_table[158] = 0.003906;
        s->lsp_pow_e_table[159] = 0.003285;
        s->lsp_pow_e_table[160] = 0.002762;
        s->lsp_pow_e_table[161] = 0.002323;
        s->lsp_pow_e_table[162] = 0.001953;
        s->lsp_pow_e_table[163] = 0.001642;
        s->lsp_pow_e_table[164] = 0.001381;
        s->lsp_pow_e_table[165] = 0.001161;
        s->lsp_pow_e_table[166] = 0.000977;
        s->lsp_pow_e_table[167] = 0.000821;
        s->lsp_pow_e_table[168] = 0.000691;
        s->lsp_pow_e_table[169] = 0.000581;
        s->lsp_pow_e_table[170] = 0.000488;
        s->lsp_pow_e_table[171] = 0.000411;
        s->lsp_pow_e_table[172] = 0.000345;
        s->lsp_pow_e_table[173] = 0.000290;
        s->lsp_pow_e_table[174] = 0.000244;
        s->lsp_pow_e_table[175] = 0.000205;
        s->lsp_pow_e_table[176] = 0.000173;
        s->lsp_pow_e_table[177] = 0.000145;
        s->lsp_pow_e_table[178] = 0.000122;
        s->lsp_pow_e_table[179] = 0.000103;
        s->lsp_pow_e_table[180] = 0.000086;
        s->lsp_pow_e_table[181] = 0.000073;
        s->lsp_pow_e_table[182] = 0.000061;
        s->lsp_pow_e_table[183] = 0.000051;
        s->lsp_pow_e_table[184] = 0.000043;
        s->lsp_pow_e_table[185] = 0.000036;
        s->lsp_pow_e_table[186] = 0.000031;
        s->lsp_pow_e_table[187] = 0.000026;
        s->lsp_pow_e_table[188] = 0.000022;
        s->lsp_pow_e_table[189] = 0.000018;
        s->lsp_pow_e_table[190] = 0.000015;
        s->lsp_pow_e_table[191] = 0.000013;
        s->lsp_pow_e_table[192] = 0.000011;
        s->lsp_pow_e_table[193] = 0.000009;
        s->lsp_pow_e_table[194] = 0.000008;
        s->lsp_pow_e_table[195] = 0.000006;
        s->lsp_pow_e_table[196] = 0.000005;
        s->lsp_pow_e_table[197] = 0.000005;
        s->lsp_pow_e_table[198] = 0.000004;
        s->lsp_pow_e_table[199] = 0.000003;
        s->lsp_pow_e_table[200] = 0.000003;
        s->lsp_pow_e_table[201] = 0.000002;
        s->lsp_pow_e_table[202] = 0.000002;
        s->lsp_pow_e_table[203] = 0.000002;
        s->lsp_pow_e_table[204] = 0.000001;
        s->lsp_pow_e_table[205] = 0.000001;
        s->lsp_pow_e_table[206] = 0.000001;
        s->lsp_pow_e_table[207] = 0.000001;
        s->lsp_pow_e_table[208] = 0.000001;
        s->lsp_pow_e_table[209] = 0.000001;
        s->lsp_pow_e_table[210] = 0.000000;
        s->lsp_pow_e_table[211] = 0.000000;
        s->lsp_pow_e_table[212] = 0.000000;
        s->lsp_pow_e_table[213] = 0.000000;
        s->lsp_pow_e_table[214] = 0.000000;
        s->lsp_pow_e_table[215] = 0.000000;
        s->lsp_pow_e_table[216] = 0.000000;
        s->lsp_pow_e_table[217] = 0.000000;
        s->lsp_pow_e_table[218] = 0.000000;
        s->lsp_pow_e_table[219] = 0.000000;
        s->lsp_pow_e_table[220] = 0.000000;
        s->lsp_pow_e_table[221] = 0.000000;
        s->lsp_pow_e_table[222] = 0.000000;
        s->lsp_pow_e_table[223] = 0.000000;
        s->lsp_pow_e_table[224] = 0.000000;
        s->lsp_pow_e_table[225] = 0.000000;
        s->lsp_pow_e_table[226] = 0.000000;
        s->lsp_pow_e_table[227] = 0.000000;
        s->lsp_pow_e_table[228] = 0.000000;
        s->lsp_pow_e_table[229] = 0.000000;
        s->lsp_pow_e_table[230] = 0.000000;
        s->lsp_pow_e_table[231] = 0.000000;
        s->lsp_pow_e_table[232] = 0.000000;
        s->lsp_pow_e_table[233] = 0.000000;
        s->lsp_pow_e_table[234] = 0.000000;
        s->lsp_pow_e_table[235] = 0.000000;
        s->lsp_pow_e_table[236] = 0.000000;
        s->lsp_pow_e_table[237] = 0.000000;
        s->lsp_pow_e_table[238] = 0.000000;
        s->lsp_pow_e_table[239] = 0.000000;
        s->lsp_pow_e_table[240] = 0.000000;
        s->lsp_pow_e_table[241] = 0.000000;
        s->lsp_pow_e_table[242] = 0.000000;
        s->lsp_pow_e_table[243] = 0.000000;
        s->lsp_pow_e_table[244] = 0.000000;
        s->lsp_pow_e_table[245] = 0.000000;
        s->lsp_pow_e_table[246] = 0.000000;
        s->lsp_pow_e_table[247] = 0.000000;
        s->lsp_pow_e_table[248] = 0.000000;
        s->lsp_pow_e_table[249] = 0.000000;
        s->lsp_pow_e_table[250] = 0.000000;
        s->lsp_pow_e_table[251] = 0.000000;
        s->lsp_pow_e_table[252] = 0.000000;
        s->lsp_pow_e_table[253] = 0.000000;
        s->lsp_pow_e_table[254] = 0.000000;
        s->lsp_pow_e_table[255] = 0.000000;


    /* NOTE: these two tables are needed to avoid two operations in
       pow_m1_4 */
    /* Replaced by pre-calculated values */

        s->lsp_pow_m_table1[127] = 1.001958;
        s->lsp_pow_m_table2[127] = -0.000979;
        s->lsp_pow_m_table1[126] = 1.002946;
        s->lsp_pow_m_table2[126] = -0.000984;
        s->lsp_pow_m_table1[125] = 1.003940;
        s->lsp_pow_m_table2[125] = -0.000989;
        s->lsp_pow_m_table1[124] = 1.004938;
        s->lsp_pow_m_table2[124] = -0.000993;
        s->lsp_pow_m_table1[123] = 1.005942;
        s->lsp_pow_m_table2[123] = -0.000998;
        s->lsp_pow_m_table1[122] = 1.006950;
        s->lsp_pow_m_table2[122] = -0.001004;
        s->lsp_pow_m_table1[121] = 1.007964;
        s->lsp_pow_m_table2[121] = -0.001009;
        s->lsp_pow_m_table1[120] = 1.008982;
        s->lsp_pow_m_table2[120] = -0.001014;
        s->lsp_pow_m_table1[119] = 1.010006;
        s->lsp_pow_m_table2[119] = -0.001019;
        s->lsp_pow_m_table1[118] = 1.011035;
        s->lsp_pow_m_table2[118] = -0.001024;
        s->lsp_pow_m_table1[117] = 1.012069;
        s->lsp_pow_m_table2[117] = -0.001029;
        s->lsp_pow_m_table1[116] = 1.013109;
        s->lsp_pow_m_table2[116] = -0.001034;
        s->lsp_pow_m_table1[115] = 1.014154;
        s->lsp_pow_m_table2[115] = -0.001040;
        s->lsp_pow_m_table1[114] = 1.015204;
        s->lsp_pow_m_table2[114] = -0.001045;
        s->lsp_pow_m_table1[113] = 1.016260;
        s->lsp_pow_m_table2[113] = -0.001050;
        s->lsp_pow_m_table1[112] = 1.017321;
        s->lsp_pow_m_table2[112] = -0.001056;
        s->lsp_pow_m_table1[111] = 1.018388;
        s->lsp_pow_m_table2[111] = -0.001061;
        s->lsp_pow_m_table1[110] = 1.019461;
        s->lsp_pow_m_table2[110] = -0.001067;
        s->lsp_pow_m_table1[109] = 1.020539;
        s->lsp_pow_m_table2[109] = -0.001073;
        s->lsp_pow_m_table1[108] = 1.021623;
        s->lsp_pow_m_table2[108] = -0.001078;
        s->lsp_pow_m_table1[107] = 1.022713;
        s->lsp_pow_m_table2[107] = -0.001084;
        s->lsp_pow_m_table1[106] = 1.023808;
        s->lsp_pow_m_table2[106] = -0.001090;
        s->lsp_pow_m_table1[105] = 1.024909;
        s->lsp_pow_m_table2[105] = -0.001096;
        s->lsp_pow_m_table1[104] = 1.026017;
        s->lsp_pow_m_table2[104] = -0.001101;
        s->lsp_pow_m_table1[103] = 1.027130;
        s->lsp_pow_m_table2[103] = -0.001107;
        s->lsp_pow_m_table1[102] = 1.028250;
        s->lsp_pow_m_table2[102] = -0.001113;
        s->lsp_pow_m_table1[101] = 1.029375;
        s->lsp_pow_m_table2[101] = -0.001119;
        s->lsp_pow_m_table1[100] = 1.030507;
        s->lsp_pow_m_table2[100] = -0.001126;
        s->lsp_pow_m_table1[99] = 1.031645;
        s->lsp_pow_m_table2[99] = -0.001132;
        s->lsp_pow_m_table1[98] = 1.032789;
        s->lsp_pow_m_table2[98] = -0.001138;
        s->lsp_pow_m_table1[97] = 1.033940;
        s->lsp_pow_m_table2[97] = -0.001144;
        s->lsp_pow_m_table1[96] = 1.035097;
        s->lsp_pow_m_table2[96] = -0.001151;
        s->lsp_pow_m_table1[95] = 1.036261;
        s->lsp_pow_m_table2[95] = -0.001157;
        s->lsp_pow_m_table1[94] = 1.037431;
        s->lsp_pow_m_table2[94] = -0.001164;
        s->lsp_pow_m_table1[93] = 1.038608;
        s->lsp_pow_m_table2[93] = -0.001170;
        s->lsp_pow_m_table1[92] = 1.039791;
        s->lsp_pow_m_table2[92] = -0.001177;
        s->lsp_pow_m_table1[91] = 1.040982;
        s->lsp_pow_m_table2[91] = -0.001184;
        s->lsp_pow_m_table1[90] = 1.042179;
        s->lsp_pow_m_table2[90] = -0.001190;
        s->lsp_pow_m_table1[89] = 1.043383;
        s->lsp_pow_m_table2[89] = -0.001197;
        s->lsp_pow_m_table1[88] = 1.044594;
        s->lsp_pow_m_table2[88] = -0.001204;
        s->lsp_pow_m_table1[87] = 1.045812;
        s->lsp_pow_m_table2[87] = -0.001211;
        s->lsp_pow_m_table1[86] = 1.047037;
        s->lsp_pow_m_table2[86] = -0.001218;
        s->lsp_pow_m_table1[85] = 1.048270;
        s->lsp_pow_m_table2[85] = -0.001225;
        s->lsp_pow_m_table1[84] = 1.049509;
        s->lsp_pow_m_table2[84] = -0.001233;
        s->lsp_pow_m_table1[83] = 1.050757;
        s->lsp_pow_m_table2[83] = -0.001240;
        s->lsp_pow_m_table1[82] = 1.052011;
        s->lsp_pow_m_table2[82] = -0.001247;
        s->lsp_pow_m_table1[81] = 1.053273;
        s->lsp_pow_m_table2[81] = -0.001255;
        s->lsp_pow_m_table1[80] = 1.054543;
        s->lsp_pow_m_table2[80] = -0.001262;
        s->lsp_pow_m_table1[79] = 1.055820;
        s->lsp_pow_m_table2[79] = -0.001270;
        s->lsp_pow_m_table1[78] = 1.057105;
        s->lsp_pow_m_table2[78] = -0.001277;
        s->lsp_pow_m_table1[77] = 1.058399;
        s->lsp_pow_m_table2[77] = -0.001285;
        s->lsp_pow_m_table1[76] = 1.059699;
        s->lsp_pow_m_table2[76] = -0.001293;
        s->lsp_pow_m_table1[75] = 1.061008;
        s->lsp_pow_m_table2[75] = -0.001301;
        s->lsp_pow_m_table1[74] = 1.062326;
        s->lsp_pow_m_table2[74] = -0.001309;
        s->lsp_pow_m_table1[73] = 1.063651;
        s->lsp_pow_m_table2[73] = -0.001317;
        s->lsp_pow_m_table1[72] = 1.064985;
        s->lsp_pow_m_table2[72] = -0.001325;
        s->lsp_pow_m_table1[71] = 1.066327;
        s->lsp_pow_m_table2[71] = -0.001334;
        s->lsp_pow_m_table1[70] = 1.067677;
        s->lsp_pow_m_table2[70] = -0.001342;
        s->lsp_pow_m_table1[69] = 1.069036;
        s->lsp_pow_m_table2[69] = -0.001351;
        s->lsp_pow_m_table1[68] = 1.070404;
        s->lsp_pow_m_table2[68] = -0.001359;
        s->lsp_pow_m_table1[67] = 1.071781;
        s->lsp_pow_m_table2[67] = -0.001368;
        s->lsp_pow_m_table1[66] = 1.073166;
        s->lsp_pow_m_table2[66] = -0.001377;
        s->lsp_pow_m_table1[65] = 1.074561;
        s->lsp_pow_m_table2[65] = -0.001386;
        s->lsp_pow_m_table1[64] = 1.075965;
        s->lsp_pow_m_table2[64] = -0.001395;
        s->lsp_pow_m_table1[63] = 1.077377;
        s->lsp_pow_m_table2[63] = -0.001404;
        s->lsp_pow_m_table1[62] = 1.078800;
        s->lsp_pow_m_table2[62] = -0.001413;
        s->lsp_pow_m_table1[61] = 1.080231;
        s->lsp_pow_m_table2[61] = -0.001422;
        s->lsp_pow_m_table1[60] = 1.081673;
        s->lsp_pow_m_table2[60] = -0.001432;
        s->lsp_pow_m_table1[59] = 1.083123;
        s->lsp_pow_m_table2[59] = -0.001441;
        s->lsp_pow_m_table1[58] = 1.084584;
        s->lsp_pow_m_table2[58] = -0.001451;
        s->lsp_pow_m_table1[57] = 1.086054;
        s->lsp_pow_m_table2[57] = -0.001461;
        s->lsp_pow_m_table1[56] = 1.087535;
        s->lsp_pow_m_table2[56] = -0.001471;
        s->lsp_pow_m_table1[55] = 1.089025;
        s->lsp_pow_m_table2[55] = -0.001481;
        s->lsp_pow_m_table1[54] = 1.090527;
        s->lsp_pow_m_table2[54] = -0.001491;
        s->lsp_pow_m_table1[53] = 1.092038;
        s->lsp_pow_m_table2[53] = -0.001501;
        s->lsp_pow_m_table1[52] = 1.093560;
        s->lsp_pow_m_table2[52] = -0.001511;
        s->lsp_pow_m_table1[51] = 1.095092;
        s->lsp_pow_m_table2[51] = -0.001522;
        s->lsp_pow_m_table1[50] = 1.096636;
        s->lsp_pow_m_table2[50] = -0.001533;
        s->lsp_pow_m_table1[49] = 1.098190;
        s->lsp_pow_m_table2[49] = -0.001544;
        s->lsp_pow_m_table1[48] = 1.099755;
        s->lsp_pow_m_table2[48] = -0.001554;
        s->lsp_pow_m_table1[47] = 1.101332;
        s->lsp_pow_m_table2[47] = -0.001566;
        s->lsp_pow_m_table1[46] = 1.102920;
        s->lsp_pow_m_table2[46] = -0.001577;
        s->lsp_pow_m_table1[45] = 1.104519;
        s->lsp_pow_m_table2[45] = -0.001588;
        s->lsp_pow_m_table1[44] = 1.106131;
        s->lsp_pow_m_table2[44] = -0.001600;
        s->lsp_pow_m_table1[43] = 1.107753;
        s->lsp_pow_m_table2[43] = -0.001611;
        s->lsp_pow_m_table1[42] = 1.109388;
        s->lsp_pow_m_table2[42] = -0.001623;
        s->lsp_pow_m_table1[41] = 1.111035;
        s->lsp_pow_m_table2[41] = -0.001635;
        s->lsp_pow_m_table1[40] = 1.112695;
        s->lsp_pow_m_table2[40] = -0.001647;
        s->lsp_pow_m_table1[39] = 1.114367;
        s->lsp_pow_m_table2[39] = -0.001660;
        s->lsp_pow_m_table1[38] = 1.116051;
        s->lsp_pow_m_table2[38] = -0.001672;
        s->lsp_pow_m_table1[37] = 1.117748;
        s->lsp_pow_m_table2[37] = -0.001685;
        s->lsp_pow_m_table1[36] = 1.119459;
        s->lsp_pow_m_table2[36] = -0.001698;
        s->lsp_pow_m_table1[35] = 1.121182;
        s->lsp_pow_m_table2[35] = -0.001710;
        s->lsp_pow_m_table1[34] = 1.122919;
        s->lsp_pow_m_table2[34] = -0.001724;
        s->lsp_pow_m_table1[33] = 1.124669;
        s->lsp_pow_m_table2[33] = -0.001737;
        s->lsp_pow_m_table1[32] = 1.126433;
        s->lsp_pow_m_table2[32] = -0.001750;
        s->lsp_pow_m_table1[31] = 1.128211;
        s->lsp_pow_m_table2[31] = -0.001764;
        s->lsp_pow_m_table1[30] = 1.130003;
        s->lsp_pow_m_table2[30] = -0.001778;
        s->lsp_pow_m_table1[29] = 1.131810;
        s->lsp_pow_m_table2[29] = -0.001792;
        s->lsp_pow_m_table1[28] = 1.133631;
        s->lsp_pow_m_table2[28] = -0.001807;
        s->lsp_pow_m_table1[27] = 1.135466;
        s->lsp_pow_m_table2[27] = -0.001821;
        s->lsp_pow_m_table1[26] = 1.137317;
        s->lsp_pow_m_table2[26] = -0.001836;
        s->lsp_pow_m_table1[25] = 1.139183;
        s->lsp_pow_m_table2[25] = -0.001851;
        s->lsp_pow_m_table1[24] = 1.141064;
        s->lsp_pow_m_table2[24] = -0.001866;
        s->lsp_pow_m_table1[23] = 1.142960;
        s->lsp_pow_m_table2[23] = -0.001881;
        s->lsp_pow_m_table1[22] = 1.144873;
        s->lsp_pow_m_table2[22] = -0.001897;
        s->lsp_pow_m_table1[21] = 1.146802;
        s->lsp_pow_m_table2[21] = -0.001913;
        s->lsp_pow_m_table1[20] = 1.148747;
        s->lsp_pow_m_table2[20] = -0.001929;
        s->lsp_pow_m_table1[19] = 1.150709;
        s->lsp_pow_m_table2[19] = -0.001945;
        s->lsp_pow_m_table1[18] = 1.152688;
        s->lsp_pow_m_table2[18] = -0.001962;
        s->lsp_pow_m_table1[17] = 1.154683;
        s->lsp_pow_m_table2[17] = -0.001979;
        s->lsp_pow_m_table1[16] = 1.156697;
        s->lsp_pow_m_table2[16] = -0.001996;
        s->lsp_pow_m_table1[15] = 1.158727;
        s->lsp_pow_m_table2[15] = -0.002013;
        s->lsp_pow_m_table1[14] = 1.160776;
        s->lsp_pow_m_table2[14] = -0.002031;
        s->lsp_pow_m_table1[13] = 1.162843;
        s->lsp_pow_m_table2[13] = -0.002049;
        s->lsp_pow_m_table1[12] = 1.164929;
        s->lsp_pow_m_table2[12] = -0.002067;
        s->lsp_pow_m_table1[11] = 1.167033;
        s->lsp_pow_m_table2[11] = -0.002086;
        s->lsp_pow_m_table1[10] = 1.169157;
        s->lsp_pow_m_table2[10] = -0.002105;
        s->lsp_pow_m_table1[9] = 1.171300;
        s->lsp_pow_m_table2[9] = -0.002124;
        s->lsp_pow_m_table1[8] = 1.173463;
        s->lsp_pow_m_table2[8] = -0.002143;
        s->lsp_pow_m_table1[7] = 1.175645;
        s->lsp_pow_m_table2[7] = -0.002163;
        s->lsp_pow_m_table1[6] = 1.177849;
        s->lsp_pow_m_table2[6] = -0.002183;
        s->lsp_pow_m_table1[5] = 1.180073;
        s->lsp_pow_m_table2[5] = -0.002204;
        s->lsp_pow_m_table1[4] = 1.182318;
        s->lsp_pow_m_table2[4] = -0.002225;
        s->lsp_pow_m_table1[3] = 1.184585;
        s->lsp_pow_m_table2[3] = -0.002246;
        s->lsp_pow_m_table1[2] = 1.186874;
        s->lsp_pow_m_table2[2] = -0.002267;
        s->lsp_pow_m_table1[1] = 1.189185;
        s->lsp_pow_m_table2[1] = -0.002289;
        s->lsp_pow_m_table1[0] = 1.191518;
        s->lsp_pow_m_table2[0] = -0.002311;

#if 0
    for(i=1;i<20;i++) {
        float v, r1, r2;
        v = 5.0 / i;
        r1 = pow_m1_4(s, v);
        r2 = pow(v,-0.25);
        printf("%f^-0.25=%f e=%f\n", v, r1, r2 - r1);
    }
#endif
}

/* NOTE: We use the same code as Vorbis here */
/* XXX: optimize it further with SSE/3Dnow */
static void wma_lsp_to_curve(WMADecodeContext *s,
                             int32_t *iout, float *out, float *val_max_ptr,
                             int n, float *lsp)
{
    int i, j;
    float p, q, w, v, val_max;

    val_max = 0;
    for(i=0;i<n;i++) {
        p = 0.5f;
        q = 0.5f;
        w = s->lsp_cos_table[i];
        for(j=1;j<NB_LSP_COEFS;j+=2){
            q *= w - lsp[j - 1];
            p *= w - lsp[j];
        }
        p *= p * (2.0f - w);
        q *= q * (2.0f + w);
        v = p + q;
        v = pow_m1_4(s, v);
        if (v > val_max)
            val_max = v;
        out[i] = v;
	iout[i] = FPTOFXU(v, 19);
    }
    *val_max_ptr = val_max;
}

/* decode exponents coded with LSP coefficients (same idea as Vorbis) */
static void decode_exp_lsp(WMADecodeContext *s, int ch)
{
    float lsp_coefs[NB_LSP_COEFS];
    int val, i;

    for(i = 0; i < NB_LSP_COEFS; i++) {
        if (i == 0 || i >= 8)
            val = get_bits(&s->gb, 3);
        else
            val = get_bits(&s->gb, 4);
        lsp_coefs[i] = lsp_codebook[i][val];
    }

    wma_lsp_to_curve(s, (int32_t *)s->iexponents[ch],
		     s->exponents[ch], &s->max_exponent[ch],
                     s->block_len, lsp_coefs);
    CONDTRIG("decode_exp_lsp");
}

static void wma_exp_vlc_init(WMADecodeContext *s)
{
    /* replaced code with pre calculated values */

    s->pow_10_to_yover16[0] = 1.000000;
    s->pow_10_to_yover16[1] = 1.154782;
    s->pow_10_to_yover16[2] = 1.333521;
    s->pow_10_to_yover16[3] = 1.539927;
    s->pow_10_to_yover16[4] = 1.778279;
    s->pow_10_to_yover16[5] = 2.053525;
    s->pow_10_to_yover16[6] = 2.371374;
    s->pow_10_to_yover16[7] = 2.738420;
    s->pow_10_to_yover16[8] = 3.162278;
    s->pow_10_to_yover16[9] = 3.651741;
    s->pow_10_to_yover16[10] = 4.216965;
    s->pow_10_to_yover16[11] = 4.869675;
    s->pow_10_to_yover16[12] = 5.623413;
    s->pow_10_to_yover16[13] = 6.493816;
    s->pow_10_to_yover16[14] = 7.498942;
    s->pow_10_to_yover16[15] = 8.659643;
    s->pow_10_to_yover16[16] = 10.000000;
    s->pow_10_to_yover16[17] = 11.547820;
    s->pow_10_to_yover16[18] = 13.335214;
    s->pow_10_to_yover16[19] = 15.399265;
    s->pow_10_to_yover16[20] = 17.782794;
    s->pow_10_to_yover16[21] = 20.535250;
    s->pow_10_to_yover16[22] = 23.713737;
    s->pow_10_to_yover16[23] = 27.384196;
    s->pow_10_to_yover16[24] = 31.622777;
    s->pow_10_to_yover16[25] = 36.517413;
    s->pow_10_to_yover16[26] = 42.169650;
    s->pow_10_to_yover16[27] = 48.696753;
    s->pow_10_to_yover16[28] = 56.234133;
    s->pow_10_to_yover16[29] = 64.938163;
    s->pow_10_to_yover16[30] = 74.989421;
    s->pow_10_to_yover16[31] = 86.596432;
    s->pow_10_to_yover16[32] = 100.000000;
    s->pow_10_to_yover16[33] = 115.478198;
    s->pow_10_to_yover16[34] = 133.352143;
    s->pow_10_to_yover16[35] = 153.992653;
    s->pow_10_to_yover16[36] = 177.827941;
    s->pow_10_to_yover16[37] = 205.352503;
    s->pow_10_to_yover16[38] = 237.137371;
    s->pow_10_to_yover16[39] = 273.841963;
    s->pow_10_to_yover16[40] = 316.227766;
    s->pow_10_to_yover16[41] = 365.174127;
    s->pow_10_to_yover16[42] = 421.696503;
    s->pow_10_to_yover16[43] = 486.967525;
    s->pow_10_to_yover16[44] = 562.341325;
    s->pow_10_to_yover16[45] = 649.381632;
    s->pow_10_to_yover16[46] = 749.894209;
    s->pow_10_to_yover16[47] = 865.964323;
    s->pow_10_to_yover16[48] = 1000.000000;
    s->pow_10_to_yover16[49] = 1154.781985;
    s->pow_10_to_yover16[50] = 1333.521432;
    s->pow_10_to_yover16[51] = 1539.926526;
    s->pow_10_to_yover16[52] = 1778.279410;
    s->pow_10_to_yover16[53] = 2053.525026;
    s->pow_10_to_yover16[54] = 2371.373706;
    s->pow_10_to_yover16[55] = 2738.419634;
    s->pow_10_to_yover16[56] = 3162.277660;
    s->pow_10_to_yover16[57] = 3651.741273;
    s->pow_10_to_yover16[58] = 4216.965034;
    s->pow_10_to_yover16[59] = 4869.675252;
    s->pow_10_to_yover16[60] = 5623.413252;
    s->pow_10_to_yover16[61] = 6493.816316;
    s->pow_10_to_yover16[62] = 7498.942093;

    s->ipow_10_to_yover16[0] = 524287;
    s->ipow_10_to_yover16[1] = 605437;
    s->ipow_10_to_yover16[2] = 699148;
    s->ipow_10_to_yover16[3] = 807364;
    s->ipow_10_to_yover16[4] = 932329;
    s->ipow_10_to_yover16[5] = 1076636;
    s->ipow_10_to_yover16[6] = 1243280;
    s->ipow_10_to_yover16[7] = 1435718;
    s->ipow_10_to_yover16[8] = 1657941;
    s->ipow_10_to_yover16[9] = 1914560;
    s->ipow_10_to_yover16[10] = 2210900;
    s->ipow_10_to_yover16[11] = 2553108;
    s->ipow_10_to_yover16[12] = 2948282;
    s->ipow_10_to_yover16[13] = 3404623;
    s->ipow_10_to_yover16[14] = 3931598;
    s->ipow_10_to_yover16[15] = 4540138;
    s->ipow_10_to_yover16[16] = 5242870;
    s->ipow_10_to_yover16[17] = 6054372;
    s->ipow_10_to_yover16[18] = 6991479;
    s->ipow_10_to_yover16[19] = 8073634;
    s->ipow_10_to_yover16[20] = 9323288;
    s->ipow_10_to_yover16[21] = 10766364;
    s->ipow_10_to_yover16[22] = 12432804;
    s->ipow_10_to_yover16[23] = 14357178;
    s->ipow_10_to_yover16[24] = 16579411;
    s->ipow_10_to_yover16[25] = 19145606;
    s->ipow_10_to_yover16[26] = 22109000;
    s->ipow_10_to_yover16[27] = 25531076;
    s->ipow_10_to_yover16[28] = 29482826;
    s->ipow_10_to_yover16[29] = 34046236;
    s->ipow_10_to_yover16[30] = 39315976;
    s->ipow_10_to_yover16[31] = 45401384;
    s->ipow_10_to_yover16[32] = 52428700;
    s->ipow_10_to_yover16[33] = 60543716;
    s->ipow_10_to_yover16[34] = 69914792;
    s->ipow_10_to_yover16[35] = 80736352;
    s->ipow_10_to_yover16[36] = 93232880;
    s->ipow_10_to_yover16[37] = 107663648;
    s->ipow_10_to_yover16[38] = 124328040;
    s->ipow_10_to_yover16[39] = 143571776;
    s->ipow_10_to_yover16[40] = 165794096;
    s->ipow_10_to_yover16[41] = 191456048;
    s->ipow_10_to_yover16[42] = 221090000;
    s->ipow_10_to_yover16[43] = 255310752;
    s->ipow_10_to_yover16[44] = 294828224;
    s->ipow_10_to_yover16[45] = 340462368;
    s->ipow_10_to_yover16[46] = 393159808;
    s->ipow_10_to_yover16[47] = 454013824;
    s->ipow_10_to_yover16[48] = 524287008;
    s->ipow_10_to_yover16[49] = 605437184;
    s->ipow_10_to_yover16[50] = 699147968;
    s->ipow_10_to_yover16[51] = 807363456;
    s->ipow_10_to_yover16[52] = 932328768;
    s->ipow_10_to_yover16[53] = 1076636544;
    s->ipow_10_to_yover16[54] = 1243280384;
    s->ipow_10_to_yover16[55] = 1435717888;
    s->ipow_10_to_yover16[56] = 1657940992;
    s->ipow_10_to_yover16[57] = 1914560384;
    s->ipow_10_to_yover16[58] = 2210899968;
    s->ipow_10_to_yover16[59] = 2553107456;
    s->ipow_10_to_yover16[60] = 2948282368;
    s->ipow_10_to_yover16[61] = 3404623616;
    s->ipow_10_to_yover16[62] = 3931597824;
}

/* decode exponents coded with VLC codes */
static int decode_exp_vlc(WMADecodeContext *s, int ch)
{
    int last_exp, n, code;
    const uint16_t *ptr, *band_ptr;
    float v, *q, max_scale, *q_end;
    uint32_t iv, *iq;

    band_ptr = s->exponent_bands[s->frame_len_bits - s->block_len_bits];
    ptr = band_ptr;
    q = s->exponents[ch];
    iq = s->iexponents[ch];
    q_end = q + s->block_len;
    max_scale = 0;
    if (s->version == 1) {
        last_exp = get_bits(&s->gb, 5) + 10;
	assert((last_exp >= 0) && (last_exp < EXP_VLC_TAB_SIZE));
	v = s->pow_10_to_yover16[last_exp];
	iv = s->ipow_10_to_yover16[last_exp];
        max_scale = v;
        n = *ptr++;
        do {
            *q++ = v;
            *iq++ = iv;
        } while (--n);
    }
    last_exp = 36;
    while (q < q_end) {
        code = get_vlc(&s->gb, &s->exp_vlc);
        if (code < 0)
            return -1;
        /* NOTE: this offset is the same as MPEG4 AAC ! */
        last_exp += code - 60;

	if(last_exp>=EXP_VLC_TAB_SIZE)
	{
		last_exp = EXP_VLC_TAB_SIZE-1;
	}
	if(last_exp<0)
	{
		last_exp = 0;
	}

	assert((last_exp >= 0) && (last_exp < EXP_VLC_TAB_SIZE));
	v = s->pow_10_to_yover16[last_exp];
	iv = s->ipow_10_to_yover16[last_exp];
        if (v > max_scale)
            max_scale = v;
        n = *ptr++;
        do {
            *q++ = v;
            *iq++ = iv;
        } while (--n);
    }
    s->max_exponent[ch] = max_scale;
    return 0;
}


/* The following is a set of precalculated results for the calculation
** of ten to the power of (int multiplied by 0.05)
** This needs to be tidied up
*/

float internal_05_power_ten(int gain)
{
        if(gain==0)
        {
                return(1.000000);
        }
        else if(gain==1)
        {
                return(1.122018);
        }
        else if(gain==2)
        {
                return(1.258925);
        }
        else if(gain==3)
        {
                return(1.412538);
        }
        else if(gain==4)
        {
                return(1.584893);
        }
        else if(gain==5)
        {
                return(1.778279);
        }
        else if(gain==6)
        {
                return(1.995262);
        }
        else if(gain==7)
        {
                return(2.238721);
        }
        else if(gain==8)
        {
                return(2.511886);
        }
        else if(gain==9)
        {
                return(2.818383);
        }
        else if(gain==10)
        {
                return(3.162278);
        }
        else if(gain==11)
        {
                return(3.548134);
        }
        else if(gain==12)
        {
                return(3.981072);
        }
        else if(gain==13)
        {
                return(4.466836);
        }
        else if(gain==14)
        {
                return(5.011872);
        }
        else if(gain==15)
        {
                return(5.623413);
        }
        else if(gain==16)
        {
                return(6.309574);
        }
        else if(gain==17)
        {
                return(7.079458);
        }
        else if(gain==18)
        {
                return(7.943282);
        }
        else if(gain==19)
        {
                return(8.912509);
        }
        else if(gain==20)
        {
                return(10.000000);
        }
        else if(gain==21)
        {
                return(11.220184);
        }
        else if(gain==22)
        {
                return(12.589254);
        }
        else if(gain==23)
        {
                return(14.125376);
        }
        else if(gain==24)
        {
                return(15.848932);
        }
        else if(gain==25)
        {
                return(17.782795);
        }
        else if(gain==26)
        {
                return(19.952623);
        }
        else if(gain==27)
        {
                return(22.387211);
        }
        else if(gain==28)
        {
                return(25.118864);
        }
        else if(gain==29)
        {
                return(28.183830);
        }
        else if(gain==30)
        {
                return(31.622776);
        }
        else if(gain==31)
        {
                return(35.481339);
        }
        else if(gain==32)
        {
                return(39.810719);
        }
        else if(gain==33)
        {
                return(44.668358);
        }
        else if(gain==34)
        {
                return(50.118725);
        }
        else if(gain==35)
        {
                return(56.234131);
        }
        else if(gain==36)
        {
                return(63.095734);
        }
        else if(gain==37)
        {
                return(70.794579);
        }
        else if(gain==38)
        {
                return(79.432823);
        }
        else if(gain==39)
        {
                return(89.125092);
        }
        else if(gain==40)
        {
                return(100.000000);
        }
        else if(gain==41)
        {
                return(112.201843);
        }
        else if(gain==42)
        {
                return(125.892540);
        }
        else if(gain==43)
        {
                return(141.253754);
        }
        else if(gain==44)
        {
                return(158.489319);
        }
        else if(gain==45)
        {
                return(177.827942);
        }
        else if(gain==46)
        {
                return(199.526230);
        }
        else if(gain==47)
        {
                return(223.872116);
        }
        else if(gain==48)
        {
                return(251.188644);
        }
        else if(gain==49)
        {
                return(281.838287);
        }
        else if(gain==50)
        {
                return(316.227753);
        }
        else if(gain==51)
        {
                return(354.813385);
        }
        else if(gain==52)
        {
                return(398.107178);
        }
        else if(gain==53)
        {
                return(446.683594);
        }
        else if(gain==54)
        {
                return(501.187225);
        }
        else if(gain==55)
        {
                return(562.341309);
        }
        else if(gain==56)
        {
                return(630.957336);
        }
        else if(gain==57)
        {
                return(707.945801);
        }
        else if(gain==58)
        {
                return(794.328247);
        }
        else if(gain==59)
        {
                return(891.250916);
        }
        else if(gain==60)
        {
                return(1000.000000);
        }
        else if(gain==61)
        {
                return(1122.018433);
        }
        else if(gain==62)
        {
                return(1258.925415);
        }
        else if(gain==63)
        {
                return(1412.537598);
        }
        else if(gain==64)
        {
                return(1584.893188);
        }
        else if(gain==65)
        {
                return(1778.279419);
        }
        else if(gain==66)
        {
                return(1995.262329);
        }
        else if(gain==67)
        {
                return(2238.721191);
        }
        else if(gain==68)
        {
                return(2511.886475);
        }
        else if(gain==69)
        {
                return(2818.382812);
        }
        else if(gain==70)
        {
                return(3162.277588);
        }
        else if(gain==71)
        {
                return(3548.133789);
        }
        else if(gain==72)
        {
                return(3981.071777);
        }
        else if(gain==73)
        {
                return(4466.835938);
        }
        else if(gain==74)
        {
                return(5011.872559);
        }
        else if(gain==75)
        {
                return(5623.413086);
        }
        else if(gain==76)
        {
                return(6309.573242);
        }
        else if(gain==77)
        {
                return(7079.458008);
        }
        else if(gain==78)
        {
                return(7943.282227);
        }
        else if(gain==79)
        {
                return(8912.509766);
        }
        else if(gain==80)
        {
                return(10000.000000);
        }
        else if(gain==81)
        {
                return(11220.184570);
        }
        else if(gain==82)
        {
                return(12589.253906);
        }
        else if(gain==83)
        {
                return(14125.375000);
        }
        else if(gain==84)
        {
                return(15848.931641);
        }
        else if(gain==85)
        {
                return(17782.794922);
        }
        else if(gain==86)
        {
                return(19952.623047);
        }
        else if(gain==87)
        {
                return(22387.210938);
        }
        else if(gain==88)
        {
                return(25118.865234);
        }
        else if(gain==89)
        {
                return(28183.830078);
        }
        else if(gain==90)
        {
                return(31622.777344);
        }
        else if(gain==91)
        {
                return(35481.339844);
        }
        else if(gain==92)
        {
                return(39810.718750);
        }
        else if(gain==93)
        {
                return(44668.359375);
        }
        else if(gain==94)
        {
                return(50118.722656);
        }
        else if(gain==95)
        {
                return(56234.132812);
        }
        else if(gain==96)
        {
                return(63095.734375);
        }
        else if(gain==97)
        {
                return(70794.578125);
        }
        else if(gain==98)
        {
                return(79432.820312);
        }
        else if(gain==99)
        {
                return(89125.093750);
        }
        else if(gain==100)
        {
                return(100000.000000);
        }
        else if(gain==101)
        {
                return(112201.843750);
        }
        else if(gain==102)
        {
                return(125892.539062);
        }
        else if(gain==103)
        {
                return(141253.750000);
        }
        else if(gain==104)
        {
                return(158489.312500);
        }
        else if(gain==105)
        {
                return(177827.937500);
        }
        else if(gain==106)
        {
                return(199526.234375);
        }
        else if(gain==107)
        {
                return(223872.109375);
        }
        else if(gain==108)
        {
                return(251188.640625);
        }
        else if(gain==109)
        {
                return(281838.281250);
        }
        else if(gain==110)
        {
                return(316227.781250);
        }
        else if(gain==111)
        {
                return(354813.375000);
        }
        else if(gain==112)
        {
                return(398107.156250);
        }
        else if(gain==113)
        {
                return(446683.593750);
        }
        else if(gain==114)
        {
                return(501187.218750);
        }
        else if(gain==115)
        {
                return(562341.312500);
        }
        else if(gain==116)
        {
                return(630957.375000);
        }
        else if(gain==117)
        {
                return(707945.812500);
        }
        else if(gain==118)
        {
                return(794328.250000);
        }
        else if(gain==119)
        {
                return(891250.937500);
        }
        else if(gain==120)
        {
                return(1000000.000000);
        }
        else if(gain==121)
        {
                return(1122018.500000);
        }
        else if(gain==122)
        {
                return(1258925.375000);
        }
        else if(gain==123)
        {
                return(1412537.500000);
        }
        else if(gain==124)
        {
                return(1584893.250000);
        }
        else if(gain==125)
        {
                return(1778279.375000);
        }
        else if(gain==126)
        {
                return(1995262.375000);
        }
        else if(gain==127)
        {
                return(2238721.250000);
        }
        else if(gain==128)
        {
                return(2511886.500000);
        }
        else if(gain==129)
        {
                return(2818383.000000);
        }
        else if(gain==130)
        {
                return(3162277.750000);
        }
        else if(gain==131)
        {
                return(3548134.000000);
        }
        else if(gain==132)
        {
                return(3981071.750000);
        }
        else if(gain==133)
        {
                return(4466836.000000);
        }
        else if(gain==134)
        {
                return(5011872.500000);
        }
        else if(gain==135)
        {
                return(5623413.500000);
        }
        else if(gain==136)
        {
                return(6309573.500000);
        }
        else if(gain==137)
        {
                return(7079458.000000);
        }
        else if(gain==138)
        {
                return(7943282.500000);
        }
        else if(gain==139)
        {
                return(8912509.000000);
        }
        else if(gain==140)
        {
                return(10000000.000000);
        }
        else if(gain==141)
        {
                return(11220185.000000);
        }
        else if(gain==142)
        {
                return(12589254.000000);
        }
        else if(gain==143)
        {
                return(14125375.000000);
        }
        else if(gain==144)
        {
                return(15848932.000000);
        }
        else if(gain==145)
        {
                return(17782794.000000);
        }
        else if(gain==146)
        {
                return(19952624.000000);
        }
        else if(gain==147)
        {
                return(22387212.000000);
        }
        else if(gain==148)
        {
                return(25118864.000000);
        }
        else if(gain==149)
        {
                return(28183830.000000);
        }
        else if(gain==150)
        {
                return(31622776.000000);
        }
        else if(gain==151)
        {
                return(35481340.000000);
        }
        else if(gain==152)
        {
                return(39810716.000000);
        }
        else if(gain==153)
        {
                return(44668360.000000);
        }
        else if(gain==154)
        {
                return(50118724.000000);
        }
        else if(gain==155)
        {
                return(56234132.000000);
        }
        else if(gain==156)
        {
                return(63095736.000000);
        }
        else if(gain==157)
        {
                return(70794576.000000);
        }
        else if(gain==158)
        {
                return(79432824.000000);
        }
        else if(gain==159)
        {
                return(89125096.000000);
        }
        else if(gain==160)
        {
                return(100000000.000000);
        }
        else if(gain==161)
        {
                return(112201848.000000);
        }
        else if(gain==162)
        {
                return(125892544.000000);
        }
        else if(gain==163)
        {
                return(141253760.000000);
        }
        else if(gain==164)
        {
                return(158489312.000000);
        }
        else if(gain==165)
        {
                return(177827936.000000);
        }
        else if(gain==166)
        {
                return(199526224.000000);
        }
        else if(gain==167)
        {
                return(223872112.000000);
        }
        else if(gain==168)
        {
                return(251188640.000000);
        }
        else if(gain==169)
        {
                return(281838304.000000);
        }
        else if(gain==170)
        {
                return(316227776.000000);
        }
        else if(gain==171)
        {
                return(354813376.000000);
        }
        else if(gain==172)
        {
                return(398107168.000000);
        }
        else if(gain==173)
        {
                return(446683584.000000);
        }
        else if(gain==174)
        {
                return(501187232.000000);
        }
        else if(gain==175)
        {
                return(562341312.000000);
        }
        else if(gain==176)
        {
                return(630957376.000000);
        }
        else if(gain==177)
        {
                return(707945792.000000);
        }
        else if(gain==178)
        {
                return(794328256.000000);
        }
        else if(gain==179)
        {
                return(891250944.000000);
        }
        else if(gain==180)
        {
                return(1000000000.000000);
        }
        else if(gain==181)
        {
                return(1122018432.000000);
        }
        else if(gain==182)
        {
                return(1258925440.000000);
        }
        else if(gain==183)
        {
                return(1412537600.000000);
        }
        else if(gain==184)
        {
                return(1584893184.000000);
        }
        else if(gain==185)
        {
                return(1778279424.000000);
        }
        else if(gain==186)
        {
                return(1995262336.000000);
        }
        else if(gain==187)
        {
                return(2238721024.000000);
        }
        else if(gain==188)
        {
                return(2511886336.000000);
        }
        else if(gain==189)
        {
                return(2818382848.000000);
        }
        else if(gain==190)
        {
                return(3162277632.000000);
        }
        else if(gain==191)
        {
                return(3548133888.000000);
        }
        else if(gain==192)
        {
                return(3981071616.000000);
        }
        else if(gain==193)
        {
                return(4466835968.000000);
        }
        else if(gain==194)
        {
                return(5011872256.000000);
        }
        else if(gain==195)
        {
                return(5623413248.000000);
        }
        else if(gain==196)
        {
                return(6309573632.000000);
        }
        else if(gain==197)
        {
                return(7079457792.000000);
        }
        else if(gain==198)
        {
                return(7943282176.000000);
        }
        else if(gain==199)
        {
                return(8912508928.000000);
        }
	else
	{
fprintf(stderr,"unhandled gain value passed %d\n", gain);
		return(100000.000000);
	}


}




/* return 0 if OK. return 1 if last block of frame. return -1 if
   unrecorrable error. */
static int wma_decode_block(WMADecodeContext *s)
{
    int n, v, a, ch, code, bsize;
    int coef_nb_bits, total_gain, parse_exponents;
    int32_t window[BLOCK_MAX_SIZE * 2];

// XXX: FIXME!! there's a bug somewhere which makes this mandatory under altivec
#ifdef HAVE_ALTIVEC
    volatile int nb_coefs[MAX_CHANNELS] __attribute__((aligned(16)));
#else
    int nb_coefs[MAX_CHANNELS];
#endif
    float mdct_norm;

#ifdef TRACE
    tprintf("***decode_block: %d:%d\n", s->frame_count - 1, s->block_num);
#endif


    /* compute current block length */
    if (s->use_variable_block_len) {
        n = av_log2(s->nb_block_sizes - 1) + 1;

        if (s->reset_block_lengths) {
            s->reset_block_lengths = 0;
            v = get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
                return -1;
            s->prev_block_len_bits = s->frame_len_bits - v;
            v = get_bits(&s->gb, n);
            if (v >= s->nb_block_sizes)
                return -1;
            s->block_len_bits = s->frame_len_bits - v;
        } else {
            /* update block lengths */
            s->prev_block_len_bits = s->block_len_bits;
            s->block_len_bits = s->next_block_len_bits;
        }

        v = get_bits(&s->gb, n);
        if (v >= s->nb_block_sizes)
            return -1;
        s->next_block_len_bits = s->frame_len_bits - v;
    } else {

        /* fixed block len */
        s->next_block_len_bits = s->frame_len_bits;
        s->prev_block_len_bits = s->frame_len_bits;
        s->block_len_bits = s->frame_len_bits;
    }

    /* now check if the block length is coherent with the frame length */
    s->block_len = 1 << s->block_len_bits;
    if ((s->block_pos + s->block_len) > s->frame_len)
        return -1;

    if (s->nb_channels == 2) {
        s->ms_stereo = get_bits(&s->gb, 1);
    }
    v = 0;
    for(ch = 0; ch < s->nb_channels; ch++) {
        a = get_bits(&s->gb, 1);
        s->channel_coded[ch] = a;
        v |= a;
    }
    /* if no channel coded, no need to go further */
    /* XXX: fix potential framing problems */
    if (!v)
        goto next;

    bsize = s->frame_len_bits - s->block_len_bits;

    /* read total gain and extract corresponding number of bits for
       coef escape coding */
    total_gain = 1;
    for(;;) {
        a = get_bits(&s->gb, 7);
        total_gain += a;
        if (a != 127)
            break;
    }

    if (total_gain < 15)
        coef_nb_bits = 13;
    else if (total_gain < 32)
        coef_nb_bits = 12;
    else if (total_gain < 40)
        coef_nb_bits = 11;
    else if (total_gain < 45)
        coef_nb_bits = 10;
    else
        coef_nb_bits = 9;

    /* compute number of coefficients */
    n = s->coefs_end[bsize] - s->coefs_start;
    for(ch = 0; ch < s->nb_channels; ch++)
        nb_coefs[ch] = n;

    /* complex coding */
    if (s->use_noise_coding) {

        for(ch = 0; ch < s->nb_channels; ch++) {
            if (s->channel_coded[ch]) {
                int i, n, a;
                n = s->exponent_high_sizes[bsize];
                for(i=0;i<n;i++) {
                    a = get_bits(&s->gb, 1);
                    s->high_band_coded[ch][i] = a;
                    /* if noise coding, the coefficients are not transmitted */
                    if (a)
                        nb_coefs[ch] -= s->exponent_high_bands[bsize][i];
                }
            }
        }
        for(ch = 0; ch < s->nb_channels; ch++) {
            if (s->channel_coded[ch]) {
                int i, n, val, code;

                n = s->exponent_high_sizes[bsize];
                val = (int)0x80000000;
                for(i=0;i<n;i++) {
                    if (s->high_band_coded[ch][i]) {
                        if (val == (int)0x80000000) {
                            val = get_bits(&s->gb, 7) - 19;
                        } else {
                            code = get_vlc(&s->gb, &s->hgain_vlc);
                            if (code < 0)
                                return -1;
                            val += code - 18;
                        }
                        s->high_band_values[ch][i] = val;
                    }
                }
            }
        }
    }

    /* exposant can be interpolated in short blocks. */
    parse_exponents = 1;
    if (s->block_len_bits != s->frame_len_bits) {
        parse_exponents = get_bits(&s->gb, 1);
    }

    if (parse_exponents) {
        for(ch = 0; ch < s->nb_channels; ch++) {
            if (s->channel_coded[ch]) {
                if (s->use_exp_vlc) {
                    if (decode_exp_vlc(s, ch) < 0)
                        return -1;
                } else {
                    decode_exp_lsp(s, ch);
                }
            }
        }
    } else {
        for(ch = 0; ch < s->nb_channels; ch++) {
            if (s->channel_coded[ch]) {
                interpolate_array(s->exponents[ch], 1 << s->prev_block_len_bits,
                                  s->block_len);
                iinterpolate_array(s->iexponents[ch], 1 << s->prev_block_len_bits,
                                  s->block_len);
            }
        }
    }

    /* parse spectral coefficients : just RLE encoding */
    for(ch = 0; ch < s->nb_channels; ch++) {
        if (s->channel_coded[ch]) {
            VLC *coef_vlc;
            int level, run, sign, tindex;
            int16_t *ptr, *eptr;
            const int16_t *level_table, *run_table;

            /* special VLC tables are used for ms stereo because
               there is potentially less energy there */
            tindex = (ch == 1 && s->ms_stereo);
            coef_vlc = &s->coef_vlc[tindex];
            run_table = (const int16_t *)s->run_table[tindex];
            level_table = (const int16_t *)s->level_table[tindex];
            /* XXX: optimize */
            ptr = &s->coefs1[ch][0];
            eptr = ptr + nb_coefs[ch];
            memset(ptr, 0, s->block_len * sizeof(int16_t));
            for(;;) {
                code = get_vlc(&s->gb, coef_vlc);
                if (code < 0)
                    return -1;
                if (code == 1) {
                    /* EOB */
                    break;
                } else if (code == 0) {
                    /* escape */
                    level = get_bits(&s->gb, coef_nb_bits);
                    /* NOTE: this is rather suboptimal. reading
                       block_len_bits would be better */
                    run = get_bits(&s->gb, s->frame_len_bits);
                } else {
                    /* normal code */
                    run = run_table[code];
                    level = level_table[code];
                }
                sign = get_bits(&s->gb, 1);
                if (!sign)
                    level = -level;
                ptr += run;
                if (ptr >= eptr)
                    return -1;
                *ptr++ = level;
                /* NOTE: EOB can be omitted */
                if (ptr >= eptr)
                    break;
            }
        }
        if (s->version == 1 && s->nb_channels >= 2) {
            align_get_bits(&s->gb);
        }
    }

    /* normalize */
    {
        int n4 = s->block_len / 2;
        mdct_norm = 1.0 / (float)n4;
        if (s->version == 1) {
            mdct_norm *= sqrt(n4);
        }
    }

    /* finally compute the MDCT coefficients */
    for(ch = 0; ch < s->nb_channels; ch++) {
        if (s->channel_coded[ch]) {
            int16_t *coefs1;
	    int32_t *coefs;
            float *exponents, mult, mult1, noise, *exp_ptr;
	    uint32_t *iexponents, imult, imult1;
	    int32_t inoise;
            int i, j, n, n1, last_high_band;
            float exp_power[HIGH_BAND_MAX_SIZE];

            coefs1 = s->coefs1[ch];
            exponents = s->exponents[ch];
            iexponents = s->iexponents[ch];
            //mult = pow(10, total_gain * 0.05) / s->max_exponent[ch];

	    mult = internal_05_power_ten(total_gain) /  s->max_exponent[ch];

            mult *= mdct_norm;
	    imult = FPTOFXU(mult, 20);
            coefs = s->coefs[ch];
            if (s->use_noise_coding) {
                /* very low freqs : noise */
                for(i = 0;i < s->coefs_start; i++) {
#if 0
                    *coefs++ = lrintf(s->noise_table[s->noise_index] * (*exponents++) * mult);
#else
		    *coefs++ = IRINT(IRINT((int64_t)s->inoise_table[s->noise_index] * *iexponents, 23) * imult, 32);
		    iexponents++;
		    exponents++;
#endif
		    CONDTRIG("very low freqs : noise");
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }

                n1 = s->exponent_high_sizes[bsize];

                /* compute power of high bands */
                exp_ptr = exponents +
                    s->high_band_start[bsize] -
                    s->coefs_start;
                last_high_band = 0; /* avoid warning */
                for(j=0;j<n1;j++) {
                    n = s->exponent_high_bands[s->frame_len_bits -
                                              s->block_len_bits][j];
                    if (s->high_band_coded[ch][j]) {
                        float e2, v;
                        e2 = 0;
                        for(i = 0;i < n; i++) {
                            v = exp_ptr[i];
                            e2 += v * v;
                        }
                        exp_power[j] = e2 / n;
                        last_high_band = j;
                        tprintf("%d: power=%f (%d)\n", j, exp_power[j], n);
			CONDTRIG("high_band_coded");
                    }
                    exp_ptr += n;
                }

                /* main freqs and high freqs */
                for(j=-1;j<n1;j++) {
                    if (j < 0) {
                        n = s->high_band_start[bsize] -
                            s->coefs_start;
                    } else {
                        n = s->exponent_high_bands[s->frame_len_bits -
                                                  s->block_len_bits][j];
                    }
                    if (j >= 0 && s->high_band_coded[ch][j]) {
                        /* use noise with specified power */
                        mult1 = sqrt(exp_power[j] / exp_power[last_high_band]);
                        /* replaced mult1 pow() calculation with pre calculated values */

			mult1 = mult1 * internal_05_power_ten(s->high_band_values[ch][j]);
                        //mult1 = mult1 * pow(10, s->high_band_values[ch][j] * 0.05);
                        mult1 = mult1 / (s->max_exponent[ch] * s->noise_mult);
                        mult1 *= mdct_norm;
			imult1 = FPTOFXU(mult1, 16);
                        for(i = 0;i < n; i++) {
#if 0
                            noise = s->noise_table[s->noise_index];
                            *coefs++ = lrintf((*exponents++) * noise * mult1);
#else
                            inoise = s->inoise_table[s->noise_index];
			    *coefs++ = IRINT(IRINT((int64_t)inoise * *iexponents, 19) * imult1, 32);
			    iexponents++;
			    exponents++;
#endif
			    CONDTRIG("noise with specified power");
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                        }
                    } else {
                        /* coded values + small noise */
                        for(i = 0;i < n; i++) {
#if 0
                            noise = s->noise_table[s->noise_index];
                            *coefs++ = lrintf((((int32_t)(*coefs1++) << 9) + noise) * (*exponents++) * mult);
#else
                            inoise = s->inoise_table[s->noise_index];
			    *coefs++ = IRINT((((int64_t)*coefs1++ * *iexponents) + IRINT((int64_t)inoise * *iexponents, 25)) * imult, 30);
			    iexponents++;
			    exponents++;
#endif
			    CONDTRIG("coded values + small noise");
                            s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                        }
                    }
                }

                /* very high freqs : noise */
                n = s->block_len - s->coefs_end[bsize];
#if 0
                mult1 = mult * exponents[-1];
#else
		imult1 = IRINT((int64_t)imult * iexponents[-1], 23); /* 20+19-16 */
#endif
                for(i = 0; i < n; i++) {
#if 0
                    *coefs++ = lrintf(s->noise_table[s->noise_index] * mult1);
#else
		    *coefs++ = IRINT((int64_t)s->inoise_table[s->noise_index] * imult1, 32);
#endif
		    CONDTRIG("very high freqs : noise");
                    s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }
            } else {
                /* XXX: optimize more */
                for(i = 0;i < s->coefs_start; i++)
                    *coefs++ = 0;
                n = nb_coefs[ch];
                for(i = 0;i < n; i++) {
#if 0
                    *coefs++ = lrintf(((int32_t)coefs1[i] << 9) * exponents[i] * mult);
#else
		    *coefs++ = IRINT(((int64_t)coefs1[i] * iexponents[i]) * imult, 30);
#endif
		    CONDTRIG("no noise coding");
                }
                n = s->block_len - s->coefs_end[bsize];
                for(i = 0;i < n; i++)
                    *coefs++ = 0;
            }
        }
    }

#ifdef TRACE
    for(ch = 0; ch < s->nb_channels; ch++) {
        if (s->channel_coded[ch]) {
            dump_floats("exponents", 3, s->exponents[ch], s->block_len);
            dump_words("iexponents", s->iexponents[ch], s->block_len);
            dump_words("coefs", s->coefs[ch], s->block_len);
        }
    }
#endif

    if (s->ms_stereo && s->channel_coded[1]) {
        int32_t a, b;
        int i;

        /* nominal case for ms stereo: we do it before mdct */
        /* no need to optimize this case because it should almost
           never happen */
        if (!s->channel_coded[0]) {
            tprintf("rare ms-stereo case happened\n");
            memset(s->coefs[0], 0, sizeof(int32_t) * s->block_len);
            s->channel_coded[0] = 1;
        }

        for(i = 0; i < s->block_len; i++) {
            a = s->coefs[0][i];
            b = s->coefs[1][i];
            s->coefs[0][i] = a + b;
            s->coefs[1][i] = a - b;
        }
    }

    /* build the window : we ensure that when the windows overlap
       their squared sum is always 1 (MDCT reconstruction rule) */
    /* XXX: merge with output */
    {
        int i, next_block_len, block_len, prev_block_len, n;
        int32_t *wptr;

        block_len = s->block_len;
        prev_block_len = 1 << s->prev_block_len_bits;
        next_block_len = 1 << s->next_block_len_bits;

        /* right part */
        wptr = window + block_len;
        if (block_len <= next_block_len) {
            for(i=0;i<block_len;i++)
                *wptr++ = s->windows[bsize][i];
        } else {
            /* overlap */
            n = (block_len / 2) - (next_block_len / 2);
            for(i=0;i<n;i++)
                *wptr++ = -0x80000000;
            for(i=0;i<next_block_len;i++)
                *wptr++ = s->windows[s->frame_len_bits - s->next_block_len_bits][i];
            for(i=0;i<n;i++)
                *wptr++ = 0;
        }

        /* left part */
        wptr = window + block_len;
        if (block_len <= prev_block_len) {
            for(i=0;i<block_len;i++)
                *--wptr = s->windows[bsize][i];
        } else {
            /* overlap */
            n = (block_len / 2) - (prev_block_len / 2);
            for(i=0;i<n;i++)
                *--wptr = -0x80000000;
            for(i=0;i<prev_block_len;i++)
                *--wptr = s->windows[s->frame_len_bits - s->prev_block_len_bits][i];
            for(i=0;i<n;i++)
                *--wptr = 0;
        }
    }


    for(ch = 0; ch < s->nb_channels; ch++) {
        if (s->channel_coded[ch]) {
	    int32_t output[BLOCK_MAX_SIZE * 2] __attribute__((aligned(16)));
            int32_t *ptr;
            int i, n4, index, n;

            n = s->block_len;
            n4 = s->block_len / 2;

	    mdct_backward(n*2, s->coefs[ch], output);

            /* XXX: optimize all that by build the window and
               multipying/adding at the same time */
            /* multiply by the window */
            for(i=0;i<n * 2;i++) {
		output[i] = FXSMULT(output[i], window[i], 31);
            }

            /* add in the frame */
            index = (s->frame_len / 2) + s->block_pos - n4;
            ptr = &s->frame_out[ch][index];
            for(i=0;i<n * 2;i++) {
                *ptr += output[i];
                ptr++;
            }

            /* specific fast case for ms-stereo : add to second
               channel if it is not coded */
            if (s->ms_stereo && !s->channel_coded[1]) {
                ptr = &s->frame_out[1][index];
                for(i=0;i<n * 2;i++) {
                    *ptr += output[i];
                    ptr++;
                }
            }
        }
    }
 next:
    /* update block number */
    s->block_num++;
    s->block_pos += s->block_len;
    if (s->block_pos >= s->frame_len)
        return 1;
    else
        return 0;
}

/* decode a frame of frame_len samples */
static int wma_decode_frame(WMADecodeContext *s, int16_t *samples)
{
    int ret, i, n, a, ch, incr;
    int16_t *ptr;
    int32_t *iptr;
#ifdef TRACE
    tprintf("***decode_frame: %d size=%d\n", s->frame_count++, s->frame_len);
#endif

    /* read each block */
    s->block_num = 0;
    s->block_pos = 0;
    for(;;) {
        ret = wma_decode_block(s);
        if (ret < 0)
            return -1;
        if (ret)
            break;
    }


    /* convert frame to integer */
    n = s->frame_len;
    incr = s->nb_channels;
    for(ch = 0; ch < s->nb_channels; ch++) {

        ptr = samples + ch;
        iptr = s->frame_out[ch];

        for(i=0;i<n;i++) {

            a = IRINT(*iptr++, 9);

            if (a > 32767)
                a = 32767;
            else if (a < -32768)
                a = -32768;
            *ptr = a;

            ptr += incr;

        }

        /* prepare for next block */
        memmove(&s->frame_out[ch][0], &s->frame_out[ch][s->frame_len],
                s->frame_len * sizeof(int32_t));
        /* XXX: suppress this */
        memset(&s->frame_out[ch][s->frame_len], 0,
               s->frame_len * sizeof(int32_t));
    }



#ifdef TRACE
    dump_shorts("samples", samples, n * s->nb_channels);
#endif
    return 0;
}

static int wma_decode_superframe(AVCodecContext *avctx,
                                 void *data, int *data_size,
                                 uint8_t *buf, int buf_size)
{
    WMADecodeContext *s = (WMADecodeContext *)avctx->priv_data;
    int nb_frames, bit_offset, i, pos, len;
    uint8_t *q;
    int16_t *samples;

    tprintf("***decode_superframe:\n");

    if(buf_size==0){
        s->last_superframe_len = 0;
        return 0;
    }

    samples = (int16_t *)data;

    init_get_bits(&s->gb, buf, buf_size*8);

    if (s->use_bit_reservoir) {
        /* read super frame header */
        get_bits(&s->gb, 4); /* super frame index */
        nb_frames = get_bits(&s->gb, 4) - 1;

        bit_offset = get_bits(&s->gb, s->byte_offset_bits + 3);

        if (s->last_superframe_len > 0) {
            //        printf("skip=%d\n", s->last_bitoffset);
            /* add bit_offset bits to last frame */
            if ((s->last_superframe_len + ((bit_offset + 7) >> 3)) >
                MAX_CODED_SUPERFRAME_SIZE)
                goto fail;
            q = s->last_superframe + s->last_superframe_len;
            len = bit_offset;
            while (len > 0) {
                *q++ = (get_bits)(&s->gb, 8);
                len -= 8;
            }
            if (len > 0) {
                *q++ = (get_bits)(&s->gb, len) << (8 - len);
            }

            /* XXX: bit_offset bits into last frame */
            init_get_bits(&s->gb, s->last_superframe, MAX_CODED_SUPERFRAME_SIZE*8);
            /* skip unused bits */
            if (s->last_bitoffset > 0)
                skip_bits(&s->gb, s->last_bitoffset);
            /* this frame is stored in the last superframe and in the
               current one */
            if (wma_decode_frame(s, samples) < 0)
                goto fail;
            samples += s->nb_channels * s->frame_len;
        }

        /* read each frame starting from bit_offset */
        pos = bit_offset + 4 + 4 + s->byte_offset_bits + 3;
        init_get_bits(&s->gb, buf + (pos >> 3), (MAX_CODED_SUPERFRAME_SIZE - (pos >> 3))*8);
        len = pos & 7;
        if (len > 0)
            skip_bits(&s->gb, len);

        s->reset_block_lengths = 1;
        for(i=0;i<nb_frames;i++) {
            if (wma_decode_frame(s, samples) < 0)
                goto fail;
            samples += s->nb_channels * s->frame_len;
        }

        /* we copy the end of the frame in the last frame buffer */
        pos = get_bits_count(&s->gb) + ((bit_offset + 4 + 4 + s->byte_offset_bits + 3) & ~7);
        s->last_bitoffset = pos & 7;
        pos >>= 3;
        len = buf_size - pos;
        if (len > MAX_CODED_SUPERFRAME_SIZE || len < 0) {
            goto fail;
        }
        s->last_superframe_len = len;
        memcpy(s->last_superframe, buf + pos, len);
    } else {
        /* single frame decode */
        if (wma_decode_frame(s, samples) < 0)
            goto fail;
        samples += s->nb_channels * s->frame_len;
    }
    *data_size = (int8_t *)samples - (int8_t *)data;
    return s->block_align;
 fail:
    /* when error, we reset the bit reservoir */
    s->last_superframe_len = 0;
    return -1;
}

static int wma_decode_end(AVCodecContext *avctx)
{
    WMADecodeContext *s = (WMADecodeContext *)avctx->priv_data;
    int i;

    for(i = 0; i < s->nb_block_sizes; i++)
        free(s->windows[i]);

    if (s->use_exp_vlc) {
        free_vlc(&s->exp_vlc);
    }
    if (s->use_noise_coding) {
        free_vlc(&s->hgain_vlc);
    }
    for(i = 0;i < 2; i++) {
        free_vlc(&s->coef_vlc[i]);
        free(s->run_table[i]);
        free(s->level_table[i]);
    }

    return 0;
}

AVCodec wmav1_decoder =
{
    "wmav1",
    CODEC_TYPE_AUDIO,
    CODEC_ID_WMAV1,
    sizeof(WMADecodeContext),
    wma_decode_init,
    NULL,
    wma_decode_end,
    wma_decode_superframe,
};

AVCodec wmav2_decoder =
{
    "wmav2",
    CODEC_TYPE_AUDIO,
    CODEC_ID_WMAV2,
    sizeof(WMADecodeContext),
    wma_decode_init,
    NULL,
    wma_decode_end,
    wma_decode_superframe,
};

