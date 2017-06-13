#ifndef __iop_dmc_in_defs_asm_h
#define __iop_dmc_in_defs_asm_h

/*
 * This file is autogenerated from
 *   file:           ../../inst/io_proc/rtl/iop_dmc_in.r
 *     id:           iop_dmc_in.r,v 1.26 2005/02/16 09:14:17 niklaspa Exp
 *     last modfied: Mon Apr 11 16:08:45 2005
 *
 *   by /n/asic/design/tools/rdesc/src/rdes2c -asm --outfile asm/iop_dmc_in_defs_asm.h ../../inst/io_proc/rtl/iop_dmc_in.r
 *      id: $Id: //WIFI_SOC/MP/SDK_5_0_0_0/RT288x_SDK/source/linux-2.6.36.x/arch/cris/include/arch-v32/arch/hwregs/iop/asm/iop_dmc_in_defs_asm.h#1 $
 * Any changes here will be lost.
 *
 * -*- buffer-read-only: t -*-
 */

#ifndef REG_FIELD
#define REG_FIELD( scope, reg, field, value ) \
  REG_FIELD_X_( value, reg_##scope##_##reg##___##field##___lsb )
#define REG_FIELD_X_( value, shift ) ((value) << shift)
#endif

#ifndef REG_STATE
#define REG_STATE( scope, reg, field, symbolic_value ) \
  REG_STATE_X_( regk_##scope##_##symbolic_value, reg_##scope##_##reg##___##field##___lsb )
#define REG_STATE_X_( k, shift ) (k << shift)
#endif

#ifndef REG_MASK
#define REG_MASK( scope, reg, field ) \
  REG_MASK_X_( reg_##scope##_##reg##___##field##___width, reg_##scope##_##reg##___##field##___lsb )
#define REG_MASK_X_( width, lsb ) (((1 << width)-1) << lsb)
#endif

#ifndef REG_LSB
#define REG_LSB( scope, reg, field ) reg_##scope##_##reg##___##field##___lsb
#endif

#ifndef REG_BIT
#define REG_BIT( scope, reg, field ) reg_##scope##_##reg##___##field##___bit
#endif

#ifndef REG_ADDR
#define REG_ADDR( scope, inst, reg ) REG_ADDR_X_(inst, reg_##scope##_##reg##_offset)
#define REG_ADDR_X_( inst, offs ) ((inst) + offs)
#endif

#ifndef REG_ADDR_VECT
#define REG_ADDR_VECT( scope, inst, reg, index ) \
         REG_ADDR_VECT_X_(inst, reg_##scope##_##reg##_offset, index, \
			 STRIDE_##scope##_##reg )
#define REG_ADDR_VECT_X_( inst, offs, index, stride ) \
                          ((inst) + offs + (index) * stride)
#endif

/* Register rw_cfg, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_cfg___sth_intr___lsb 0
#define reg_iop_dmc_in_rw_cfg___sth_intr___width 3
#define reg_iop_dmc_in_rw_cfg___last_dis_dif___lsb 3
#define reg_iop_dmc_in_rw_cfg___last_dis_dif___width 1
#define reg_iop_dmc_in_rw_cfg___last_dis_dif___bit 3
#define reg_iop_dmc_in_rw_cfg_offset 0

/* Register rw_ctrl, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_ctrl___dif_en___lsb 0
#define reg_iop_dmc_in_rw_ctrl___dif_en___width 1
#define reg_iop_dmc_in_rw_ctrl___dif_en___bit 0
#define reg_iop_dmc_in_rw_ctrl___dif_dis___lsb 1
#define reg_iop_dmc_in_rw_ctrl___dif_dis___width 1
#define reg_iop_dmc_in_rw_ctrl___dif_dis___bit 1
#define reg_iop_dmc_in_rw_ctrl___stream_clr___lsb 2
#define reg_iop_dmc_in_rw_ctrl___stream_clr___width 1
#define reg_iop_dmc_in_rw_ctrl___stream_clr___bit 2
#define reg_iop_dmc_in_rw_ctrl_offset 4

/* Register r_stat, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_stat___dif_en___lsb 0
#define reg_iop_dmc_in_r_stat___dif_en___width 1
#define reg_iop_dmc_in_r_stat___dif_en___bit 0
#define reg_iop_dmc_in_r_stat_offset 8

/* Register rw_stream_cmd, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_stream_cmd___cmd___lsb 0
#define reg_iop_dmc_in_rw_stream_cmd___cmd___width 10
#define reg_iop_dmc_in_rw_stream_cmd___n___lsb 16
#define reg_iop_dmc_in_rw_stream_cmd___n___width 8
#define reg_iop_dmc_in_rw_stream_cmd_offset 12

/* Register rw_stream_wr_data, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_stream_wr_data_offset 16

/* Register rw_stream_wr_data_last, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_stream_wr_data_last_offset 20

/* Register rw_stream_ctrl, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_stream_ctrl___eop___lsb 0
#define reg_iop_dmc_in_rw_stream_ctrl___eop___width 1
#define reg_iop_dmc_in_rw_stream_ctrl___eop___bit 0
#define reg_iop_dmc_in_rw_stream_ctrl___wait___lsb 1
#define reg_iop_dmc_in_rw_stream_ctrl___wait___width 1
#define reg_iop_dmc_in_rw_stream_ctrl___wait___bit 1
#define reg_iop_dmc_in_rw_stream_ctrl___keep_md___lsb 2
#define reg_iop_dmc_in_rw_stream_ctrl___keep_md___width 1
#define reg_iop_dmc_in_rw_stream_ctrl___keep_md___bit 2
#define reg_iop_dmc_in_rw_stream_ctrl___size___lsb 3
#define reg_iop_dmc_in_rw_stream_ctrl___size___width 3
#define reg_iop_dmc_in_rw_stream_ctrl_offset 24

/* Register r_stream_stat, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_stream_stat___sth___lsb 0
#define reg_iop_dmc_in_r_stream_stat___sth___width 7
#define reg_iop_dmc_in_r_stream_stat___full___lsb 16
#define reg_iop_dmc_in_r_stream_stat___full___width 1
#define reg_iop_dmc_in_r_stream_stat___full___bit 16
#define reg_iop_dmc_in_r_stream_stat___last_pkt___lsb 17
#define reg_iop_dmc_in_r_stream_stat___last_pkt___width 1
#define reg_iop_dmc_in_r_stream_stat___last_pkt___bit 17
#define reg_iop_dmc_in_r_stream_stat___data_md_valid___lsb 18
#define reg_iop_dmc_in_r_stream_stat___data_md_valid___width 1
#define reg_iop_dmc_in_r_stream_stat___data_md_valid___bit 18
#define reg_iop_dmc_in_r_stream_stat___ctxt_md_valid___lsb 19
#define reg_iop_dmc_in_r_stream_stat___ctxt_md_valid___width 1
#define reg_iop_dmc_in_r_stream_stat___ctxt_md_valid___bit 19
#define reg_iop_dmc_in_r_stream_stat___group_md_valid___lsb 20
#define reg_iop_dmc_in_r_stream_stat___group_md_valid___width 1
#define reg_iop_dmc_in_r_stream_stat___group_md_valid___bit 20
#define reg_iop_dmc_in_r_stream_stat___stream_busy___lsb 21
#define reg_iop_dmc_in_r_stream_stat___stream_busy___width 1
#define reg_iop_dmc_in_r_stream_stat___stream_busy___bit 21
#define reg_iop_dmc_in_r_stream_stat___cmd_rdy___lsb 22
#define reg_iop_dmc_in_r_stream_stat___cmd_rdy___width 1
#define reg_iop_dmc_in_r_stream_stat___cmd_rdy___bit 22
#define reg_iop_dmc_in_r_stream_stat_offset 28

/* Register r_data_descr, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_data_descr___ctrl___lsb 0
#define reg_iop_dmc_in_r_data_descr___ctrl___width 8
#define reg_iop_dmc_in_r_data_descr___stat___lsb 8
#define reg_iop_dmc_in_r_data_descr___stat___width 8
#define reg_iop_dmc_in_r_data_descr___md___lsb 16
#define reg_iop_dmc_in_r_data_descr___md___width 16
#define reg_iop_dmc_in_r_data_descr_offset 32

/* Register r_ctxt_descr, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_ctxt_descr___ctrl___lsb 0
#define reg_iop_dmc_in_r_ctxt_descr___ctrl___width 8
#define reg_iop_dmc_in_r_ctxt_descr___stat___lsb 8
#define reg_iop_dmc_in_r_ctxt_descr___stat___width 8
#define reg_iop_dmc_in_r_ctxt_descr___md0___lsb 16
#define reg_iop_dmc_in_r_ctxt_descr___md0___width 16
#define reg_iop_dmc_in_r_ctxt_descr_offset 36

/* Register r_ctxt_descr_md1, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_ctxt_descr_md1_offset 40

/* Register r_ctxt_descr_md2, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_ctxt_descr_md2_offset 44

/* Register r_group_descr, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_group_descr___ctrl___lsb 0
#define reg_iop_dmc_in_r_group_descr___ctrl___width 8
#define reg_iop_dmc_in_r_group_descr___stat___lsb 8
#define reg_iop_dmc_in_r_group_descr___stat___width 8
#define reg_iop_dmc_in_r_group_descr___md___lsb 16
#define reg_iop_dmc_in_r_group_descr___md___width 16
#define reg_iop_dmc_in_r_group_descr_offset 56

/* Register rw_data_descr, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_data_descr___md___lsb 16
#define reg_iop_dmc_in_rw_data_descr___md___width 16
#define reg_iop_dmc_in_rw_data_descr_offset 60

/* Register rw_ctxt_descr, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_ctxt_descr___md0___lsb 16
#define reg_iop_dmc_in_rw_ctxt_descr___md0___width 16
#define reg_iop_dmc_in_rw_ctxt_descr_offset 64

/* Register rw_ctxt_descr_md1, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_ctxt_descr_md1_offset 68

/* Register rw_ctxt_descr_md2, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_ctxt_descr_md2_offset 72

/* Register rw_group_descr, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_group_descr___md___lsb 16
#define reg_iop_dmc_in_rw_group_descr___md___width 16
#define reg_iop_dmc_in_rw_group_descr_offset 84

/* Register rw_intr_mask, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_intr_mask___data_md___lsb 0
#define reg_iop_dmc_in_rw_intr_mask___data_md___width 1
#define reg_iop_dmc_in_rw_intr_mask___data_md___bit 0
#define reg_iop_dmc_in_rw_intr_mask___ctxt_md___lsb 1
#define reg_iop_dmc_in_rw_intr_mask___ctxt_md___width 1
#define reg_iop_dmc_in_rw_intr_mask___ctxt_md___bit 1
#define reg_iop_dmc_in_rw_intr_mask___group_md___lsb 2
#define reg_iop_dmc_in_rw_intr_mask___group_md___width 1
#define reg_iop_dmc_in_rw_intr_mask___group_md___bit 2
#define reg_iop_dmc_in_rw_intr_mask___cmd_rdy___lsb 3
#define reg_iop_dmc_in_rw_intr_mask___cmd_rdy___width 1
#define reg_iop_dmc_in_rw_intr_mask___cmd_rdy___bit 3
#define reg_iop_dmc_in_rw_intr_mask___sth___lsb 4
#define reg_iop_dmc_in_rw_intr_mask___sth___width 1
#define reg_iop_dmc_in_rw_intr_mask___sth___bit 4
#define reg_iop_dmc_in_rw_intr_mask___full___lsb 5
#define reg_iop_dmc_in_rw_intr_mask___full___width 1
#define reg_iop_dmc_in_rw_intr_mask___full___bit 5
#define reg_iop_dmc_in_rw_intr_mask_offset 88

/* Register rw_ack_intr, scope iop_dmc_in, type rw */
#define reg_iop_dmc_in_rw_ack_intr___data_md___lsb 0
#define reg_iop_dmc_in_rw_ack_intr___data_md___width 1
#define reg_iop_dmc_in_rw_ack_intr___data_md___bit 0
#define reg_iop_dmc_in_rw_ack_intr___ctxt_md___lsb 1
#define reg_iop_dmc_in_rw_ack_intr___ctxt_md___width 1
#define reg_iop_dmc_in_rw_ack_intr___ctxt_md___bit 1
#define reg_iop_dmc_in_rw_ack_intr___group_md___lsb 2
#define reg_iop_dmc_in_rw_ack_intr___group_md___width 1
#define reg_iop_dmc_in_rw_ack_intr___group_md___bit 2
#define reg_iop_dmc_in_rw_ack_intr___cmd_rdy___lsb 3
#define reg_iop_dmc_in_rw_ack_intr___cmd_rdy___width 1
#define reg_iop_dmc_in_rw_ack_intr___cmd_rdy___bit 3
#define reg_iop_dmc_in_rw_ack_intr___sth___lsb 4
#define reg_iop_dmc_in_rw_ack_intr___sth___width 1
#define reg_iop_dmc_in_rw_ack_intr___sth___bit 4
#define reg_iop_dmc_in_rw_ack_intr___full___lsb 5
#define reg_iop_dmc_in_rw_ack_intr___full___width 1
#define reg_iop_dmc_in_rw_ack_intr___full___bit 5
#define reg_iop_dmc_in_rw_ack_intr_offset 92

/* Register r_intr, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_intr___data_md___lsb 0
#define reg_iop_dmc_in_r_intr___data_md___width 1
#define reg_iop_dmc_in_r_intr___data_md___bit 0
#define reg_iop_dmc_in_r_intr___ctxt_md___lsb 1
#define reg_iop_dmc_in_r_intr___ctxt_md___width 1
#define reg_iop_dmc_in_r_intr___ctxt_md___bit 1
#define reg_iop_dmc_in_r_intr___group_md___lsb 2
#define reg_iop_dmc_in_r_intr___group_md___width 1
#define reg_iop_dmc_in_r_intr___group_md___bit 2
#define reg_iop_dmc_in_r_intr___cmd_rdy___lsb 3
#define reg_iop_dmc_in_r_intr___cmd_rdy___width 1
#define reg_iop_dmc_in_r_intr___cmd_rdy___bit 3
#define reg_iop_dmc_in_r_intr___sth___lsb 4
#define reg_iop_dmc_in_r_intr___sth___width 1
#define reg_iop_dmc_in_r_intr___sth___bit 4
#define reg_iop_dmc_in_r_intr___full___lsb 5
#define reg_iop_dmc_in_r_intr___full___width 1
#define reg_iop_dmc_in_r_intr___full___bit 5
#define reg_iop_dmc_in_r_intr_offset 96

/* Register r_masked_intr, scope iop_dmc_in, type r */
#define reg_iop_dmc_in_r_masked_intr___data_md___lsb 0
#define reg_iop_dmc_in_r_masked_intr___data_md___width 1
#define reg_iop_dmc_in_r_masked_intr___data_md___bit 0
#define reg_iop_dmc_in_r_masked_intr___ctxt_md___lsb 1
#define reg_iop_dmc_in_r_masked_intr___ctxt_md___width 1
#define reg_iop_dmc_in_r_masked_intr___ctxt_md___bit 1
#define reg_iop_dmc_in_r_masked_intr___group_md___lsb 2
#define reg_iop_dmc_in_r_masked_intr___group_md___width 1
#define reg_iop_dmc_in_r_masked_intr___group_md___bit 2
#define reg_iop_dmc_in_r_masked_intr___cmd_rdy___lsb 3
#define reg_iop_dmc_in_r_masked_intr___cmd_rdy___width 1
#define reg_iop_dmc_in_r_masked_intr___cmd_rdy___bit 3
#define reg_iop_dmc_in_r_masked_intr___sth___lsb 4
#define reg_iop_dmc_in_r_masked_intr___sth___width 1
#define reg_iop_dmc_in_r_masked_intr___sth___bit 4
#define reg_iop_dmc_in_r_masked_intr___full___lsb 5
#define reg_iop_dmc_in_r_masked_intr___full___width 1
#define reg_iop_dmc_in_r_masked_intr___full___bit 5
#define reg_iop_dmc_in_r_masked_intr_offset 100


/* Constants */
#define regk_iop_dmc_in_ack_pkt                   0x00000100
#define regk_iop_dmc_in_array                     0x00000008
#define regk_iop_dmc_in_burst                     0x00000020
#define regk_iop_dmc_in_copy_next                 0x00000010
#define regk_iop_dmc_in_copy_up                   0x00000020
#define regk_iop_dmc_in_dis_c                     0x00000010
#define regk_iop_dmc_in_dis_g                     0x00000020
#define regk_iop_dmc_in_lim1                      0x00000000
#define regk_iop_dmc_in_lim16                     0x00000004
#define regk_iop_dmc_in_lim2                      0x00000001
#define regk_iop_dmc_in_lim32                     0x00000005
#define regk_iop_dmc_in_lim4                      0x00000002
#define regk_iop_dmc_in_lim64                     0x00000006
#define regk_iop_dmc_in_lim8                      0x00000003
#define regk_iop_dmc_in_load_c                    0x00000200
#define regk_iop_dmc_in_load_c_n                  0x00000280
#define regk_iop_dmc_in_load_c_next               0x00000240
#define regk_iop_dmc_in_load_d                    0x00000140
#define regk_iop_dmc_in_load_g                    0x00000300
#define regk_iop_dmc_in_load_g_down               0x000003c0
#define regk_iop_dmc_in_load_g_next               0x00000340
#define regk_iop_dmc_in_load_g_up                 0x00000380
#define regk_iop_dmc_in_next_en                   0x00000010
#define regk_iop_dmc_in_next_pkt                  0x00000010
#define regk_iop_dmc_in_no                        0x00000000
#define regk_iop_dmc_in_restore                   0x00000020
#define regk_iop_dmc_in_rw_cfg_default            0x00000000
#define regk_iop_dmc_in_rw_ctxt_descr_default     0x00000000
#define regk_iop_dmc_in_rw_ctxt_descr_md1_default  0x00000000
#define regk_iop_dmc_in_rw_ctxt_descr_md2_default  0x00000000
#define regk_iop_dmc_in_rw_data_descr_default     0x00000000
#define regk_iop_dmc_in_rw_group_descr_default    0x00000000
#define regk_iop_dmc_in_rw_intr_mask_default      0x00000000
#define regk_iop_dmc_in_rw_stream_ctrl_default    0x00000000
#define regk_iop_dmc_in_save_down                 0x00000020
#define regk_iop_dmc_in_save_up                   0x00000020
#define regk_iop_dmc_in_set_reg                   0x00000050
#define regk_iop_dmc_in_set_w_size1               0x00000190
#define regk_iop_dmc_in_set_w_size2               0x000001a0
#define regk_iop_dmc_in_set_w_size4               0x000001c0
#define regk_iop_dmc_in_store_c                   0x00000002
#define regk_iop_dmc_in_store_descr               0x00000000
#define regk_iop_dmc_in_store_g                   0x00000004
#define regk_iop_dmc_in_store_md                  0x00000001
#define regk_iop_dmc_in_update_down               0x00000020
#define regk_iop_dmc_in_yes                       0x00000001
#endif /* __iop_dmc_in_defs_asm_h */
