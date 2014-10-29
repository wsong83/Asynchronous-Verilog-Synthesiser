# asynchronous Verilog test script

set search_path "../nova/src"

analyze -format verilog Beha_BitStream_ram.v
analyze -format verilog BitStream_buffer.v
analyze -format verilog BitStream_controller.v
analyze -format verilog bitstream_gclk_gen.v
analyze -format verilog BitStream_parser_FSM_gating.v
analyze -format verilog bs_decoding.v
analyze -format verilog cavlc_consumed_bits_decoding.v
analyze -format verilog cavlc_decoder.v
analyze -format verilog CodedBlockPattern_decoding.v
analyze -format verilog dependent_variable_decoding.v
analyze -format verilog DF_mem_ctrl.v
analyze -format verilog DF_pipeline.v
analyze -format verilog DF_reg_ctrl.v
analyze -format verilog DF_top.v
analyze -format verilog end_of_blk_decoding.v
analyze -format verilog exp_golomb_decoding.v
analyze -format verilog ext_frame_RAM0_wrapper.v
analyze -format verilog ext_frame_RAM1_wrapper.v
analyze -format verilog ext_RAM_ctrl.v
analyze -format verilog heading_one_detector.v
analyze -format verilog hybrid_pipeline_ctrl.v
analyze -format verilog Inter_mv_decoding.v
analyze -format verilog Inter_pred_CPE.v
analyze -format verilog Inter_pred_LPE.v
analyze -format verilog Inter_pred_pipeline.v
analyze -format verilog Inter_pred_reg_ctrl.v
analyze -format verilog Inter_pred_sliding_window.v
analyze -format verilog Inter_pred_top.v
analyze -format verilog Intra4x4_PredMode_decoding.v
analyze -format verilog Intra_pred_PE.v
analyze -format verilog Intra_pred_pipeline.v
analyze -format verilog Intra_pred_reg_ctrl.v
analyze -format verilog Intra_pred_top.v
analyze -format verilog IQIT.v
analyze -format verilog level_decoding.v
analyze -format verilog nC_decoding.v
analyze -format verilog nova.v
analyze -format verilog NumCoeffTrailingOnes_decoding.v
analyze -format verilog pc_decoding.v
analyze -format verilog QP_decoding.v
analyze -format verilog ram_async_1r_sync_1w.v
analyze -format verilog ram_sync_1r_sync_1w.v
analyze -format verilog rec_DF_RAM_ctrl.v
analyze -format verilog rec_gclk_gen.v
analyze -format verilog reconstruction.v
analyze -format verilog run_decoding.v
analyze -format verilog sum.v
analyze -format verilog syntax_decoding.v
analyze -format verilog total_zeros_decoding.v

#elaborate nova
#extract_sdfg

