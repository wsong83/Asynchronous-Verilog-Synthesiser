module test_module (o, c, f);
   output [2:0] o, c, f;
   wire [2:0]   o, c, f;
   reg [4+25:4'd35] sig, tmp;
   
   assign {o,{1{c}}} = {{3'b11}, {3{2'b10}}, {2{1'b1,sig,1'b1}}};
   assign f = 1===1'b1 ? sig[3 +: 1][3 -: 1] : {2'd3, 5'h3} + 3**4;
   
endmodule // test

module test_module;
endmodule // test_module

/*
 
