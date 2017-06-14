/*
 * WMA compatible codec
 * Copyright (c) 2002-2007 The FFmpeg Project.
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

#ifndef WMA_H
#define WMA_H

#include "bitstream.h"
#include "dsputil.h"
//Qwert
#define  lrintf(f) ((int32_t)(f))
#define  llrintf(f) ((int32_t)(f))
/* size of blocks */
#define BLOCK_MIN_BITS 7
#define BLOCK_MAX_BITS 11
#define BLOCK_MAX_SIZE (1 << BLOCK_MAX_BITS)

#define BLOCK_NB_SIZES (BLOCK_MAX_BITS - BLOCK_MIN_BITS + 1)

/* XXX: find exact max size */
#define HIGH_BAND_MAX_SIZE 16

#define NB_LSP_COEFS 10

/* XXX: is it a suitable value ? */
#define MAX_CODED_SUPERFRAME_SIZE 16384

#define MAX_CHANNELS 2

#define NOISE_TAB_SIZE 8192

#define LSP_POW_BITS 7

#define EXP_VLC_TAB_SIZE 63

/* Fixed point ops */
#ifdef NO_INTEGER_ROUNDING
#define IRINT(_v, _s)		((_v) >> (_s))
#else
#define IRINT(_v, _s) ({						\
    typeof (_v) __v = (_v);						\
    typeof (__v) __r = ((typeof (__v))1) << ((_s) - 1);			\
    __v += __r;								\
    __v >> (_s);							\
})
#endif

#ifdef NO_FPTOFX_ASSERT
#define FPTOFXS(_fp, _s)	((int32_t)(lrintf((_fp) * (int32_t)((1LL<<(_s))-1))))
#define NFPTOFXS(_fp, _s)	((int32_t)(lrintf((_fp) * (int32_t)(-(1LL<<(_s))))))
//#define FPTOFXU(_fp, _s)	((uint32_t)(llrintf((_fp) * (uint32_t)((1LL<<(_s))-1))))
#define FPTOFXU(_fp, _s)	((uint32_t)(lrintf((_fp) * (uint32_t)((1LL<<(_s))-1))))
#else
#define FPTOFXS(_fp, _s) ({					\
    float __fp = (_fp);						\
    assert((__fp >= (float)-(1<<(31-(_s)))) &&			\
	   (__fp <= (float)(1<<(31-(_s)))));			\
    ((int32_t)(lrintf(__fp * (int32_t)((1LL<<(_s))-1))));	\
})
#define NFPTOFXS(_fp, _s) ({					\
    float __fp = (_fp);						\
    assert((__fp >= (float)-(1<<(31-(_s)))) &&			\
	   (__fp <= (float)(1<<(31-(_s)))));			\
    ((int32_t)(lrintf(__fp * (int32_t)(-(1LL<<(_s))))));	\
})
#define FPTOFXU(_fp, _s) ({					\
    float __fp = (_fp);						\
    assert((__fp >= 0.f) && (__fp <= (float)(1<<(32-(_s)))));	\
    ((uint32_t)(llrintf(__fp * (uint32_t)((1LL<<(_s))-1))));	\
})
#endif

#define FXSMULT(_a, _b, _s)	IRINT((int64_t)(_a) * (_b), (_s))
#define FXUMULT(_a, _b, _s)	IRINT((uint64_t)(_a) * (_b), (_s)))

#if 0
#define CONDTRIG(_s) do {						\
    static int hit = 0;							\
    if (!hit) {								\
	hit = 1;							\
	dprintf("\nLine %d, condition '%s' hit\n", __LINE__, (_s));	\
    }									\
} while (0)
#else
#define CONDTRIG(_s)
#endif

typedef struct WMADecodeContext_t {
    GetBitContext gb;
    int sample_rate;
    int nb_channels;
    int bit_rate;
    int version; /* 1 = 0x160 (WMAV1), 2 = 0x161 (WMAV2) */
    int block_align;
    int use_bit_reservoir;
    int use_variable_block_len;
    int use_exp_vlc;  /* exponent coding: 0 = lsp, 1 = vlc + delta */
    int use_noise_coding; /* true if perceptual noise is added */
    int byte_offset_bits;
    VLC exp_vlc;
    int exponent_sizes[BLOCK_NB_SIZES];
    uint16_t exponent_bands[BLOCK_NB_SIZES][25];
    int high_band_start[BLOCK_NB_SIZES]; /* index of first coef in high band */
    int coefs_start;               /* first coded coef */
    int coefs_end[BLOCK_NB_SIZES]; /* max number of coded coefficients */
    int exponent_high_sizes[BLOCK_NB_SIZES];
    int exponent_high_bands[BLOCK_NB_SIZES][HIGH_BAND_MAX_SIZE];
    VLC hgain_vlc;

    /* coded values in high bands */
    int high_band_coded[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];
    int high_band_values[MAX_CHANNELS][HIGH_BAND_MAX_SIZE];

    /* there are two possible tables for spectral coefficients */
    VLC coef_vlc[2];
    uint16_t *run_table[2];
    uint16_t *level_table[2];
    /* frame info */
    int frame_len;       /* frame length in samples */
    int frame_len_bits;  /* frame_len = 1 << frame_len_bits */
    int nb_block_sizes;  /* number of block sizes */
    /* block info */
    int reset_block_lengths;
    int block_len_bits; /* log2 of current block length */
    int next_block_len_bits; /* log2 of next block length */
    int prev_block_len_bits; /* log2 of prev block length */
    int block_len; /* block length in samples */
    int block_num; /* block number in current frame */
    int block_pos; /* current position in frame */
    uint8_t ms_stereo; /* true if mid/side stereo mode */
    uint8_t channel_coded[MAX_CHANNELS]; /* true if channel is coded */
    float exponents[MAX_CHANNELS][BLOCK_MAX_SIZE] __attribute__((aligned(16)));
    uint32_t iexponents[MAX_CHANNELS][BLOCK_MAX_SIZE] __attribute__((aligned(16))); /* << 19 */
    float max_exponent[MAX_CHANNELS];
    int16_t coefs1[MAX_CHANNELS][BLOCK_MAX_SIZE];
    int32_t coefs[MAX_CHANNELS][BLOCK_MAX_SIZE] __attribute__((aligned(16)));
    int32_t *windows[BLOCK_NB_SIZES];
    /* output buffer for one frame and the last for IMDCT windowing */
    int32_t frame_out[MAX_CHANNELS][BLOCK_MAX_SIZE * 2] __attribute__((aligned(16)));
    /* last frame info */
    uint8_t last_superframe[MAX_CODED_SUPERFRAME_SIZE + 4]; /* padding added */
    int last_bitoffset;
    int last_superframe_len;
    float noise_table[NOISE_TAB_SIZE];
    int32_t inoise_table[NOISE_TAB_SIZE];	/* << 16 */
    int noise_index;
    float noise_mult; /* XXX: suppress that and integrate it in the noise array */
    /* lsp_to_curve tables */
    float lsp_cos_table[BLOCK_MAX_SIZE];
    float lsp_pow_e_table[256];
    float lsp_pow_m_table1[(1 << LSP_POW_BITS)];
    float lsp_pow_m_table2[(1 << LSP_POW_BITS)];
    /* exp vlc table */
    float pow_10_to_yover16[EXP_VLC_TAB_SIZE];
    uint32_t ipow_10_to_yover16[EXP_VLC_TAB_SIZE];


#ifdef TRACE
    int frame_count;
#endif
} WMADecodeContext;

typedef struct CoefVLCTable {
    int n; /* total number of codes */
    int max_level;
    const uint32_t *huffcodes; /* VLC bit values */
    const uint8_t *huffbits;   /* VLC bit size */
    const uint16_t *levels; /* table to build run/level tables */
} CoefVLCTable;

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len);
static void wma_exp_vlc_init(WMADecodeContext *s);

#endif
