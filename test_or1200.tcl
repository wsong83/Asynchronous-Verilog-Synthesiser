# asynchronous Verilog test script

set search_path "../or1200-rel1/rtl/verilog"

suppress_message "SYN-PORT-2"
suppress_message "ELAB-VAR-2"
suppress_message "ELAB-VAR-3"

analyze -format verilog or1200_alu.v
analyze -format verilog or1200_amultp2_32x32.v
analyze -format verilog or1200_cfgr.v
analyze -format verilog or1200_cpu.v
analyze -format verilog or1200_ctrl.v
analyze -format verilog or1200_dc_fsm.v
analyze -format verilog or1200_dc_ram.v
analyze -format verilog or1200_dc_tag.v
analyze -format verilog or1200_dc_top.v
analyze -format verilog or1200_dmmu_tlb.v
analyze -format verilog or1200_dmmu_top.v
analyze -format verilog or1200_dpram_32x32.v
analyze -format verilog or1200_du.v
analyze -format verilog or1200_except.v
analyze -format verilog or1200_freeze.v
analyze -format verilog or1200_genpc.v
analyze -format verilog or1200_gmultp2_32x32.v
analyze -format verilog or1200_ic_fsm.v
analyze -format verilog or1200_ic_ram.v
analyze -format verilog or1200_ic_tag.v
analyze -format verilog or1200_ic_top.v
analyze -format verilog or1200_if.v
analyze -format verilog or1200_immu_tlb.v
analyze -format verilog or1200_immu_top.v
analyze -format verilog or1200_iwb_biu.v
analyze -format verilog or1200_lsu.v
analyze -format verilog or1200_mem2reg.v
analyze -format verilog or1200_mult_mac.v
analyze -format verilog or1200_operandmuxes.v
analyze -format verilog or1200_pic.v
analyze -format verilog or1200_pm.v
analyze -format verilog or1200_qmem_top.v
analyze -format verilog or1200_reg2mem.v
analyze -format verilog or1200_rf.v
analyze -format verilog or1200_rfram_generic.v
analyze -format verilog or1200_sb.v
analyze -format verilog or1200_sb_fifo.v
analyze -format verilog or1200_spram_1024x32.v
analyze -format verilog or1200_spram_1024x8.v
analyze -format verilog or1200_spram_2048x32.v
analyze -format verilog or1200_spram_2048x8.v
analyze -format verilog or1200_spram_256x21.v
analyze -format verilog or1200_spram_512x20.v
analyze -format verilog or1200_spram_64x14.v
analyze -format verilog or1200_spram_64x22.v
analyze -format verilog or1200_spram_64x24.v
analyze -format verilog or1200_sprs.v
analyze -format verilog or1200_top.v
analyze -format verilog or1200_tt.v
analyze -format verilog or1200_wb_biu.v
analyze -format verilog or1200_wbmux.v

elaborate or1200_top
extract_sdfg 

