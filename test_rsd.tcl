# asynchronous Verilog test script

set search_path "../reed_solomon_decoder/rtl"

suppress_message "SYN-PORT-2"
#suppress_message "ELAB-VAR-2"
#suppress_message "ELAB-VAR-3"

analyze -format verilog GF_matrix_ascending_binary.v
analyze -format verilog DP_RAM.v
analyze -format verilog GF_matrix_dec.v
analyze -format verilog BM_lamda.v
analyze -format verilog GF_mult_add_syndromes.v
analyze -format verilog Omega_Phy.v
analyze -format verilog RS_dec.v
analyze -format verilog error_correction.v
analyze -format verilog input_syndromes.v
analyze -format verilog lamda_roots.v
analyze -format verilog out_stage.v
analyze -format verilog transport_in2out.v

elaborate RS_dec
extract_sdfg 

