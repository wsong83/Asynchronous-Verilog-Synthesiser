module test_module (o, c);
   output [2:0] o, c;
   wire [2:0]   o, c;
   reg [4+25:4'd35] sig, tmp;
   
   assign {o,{1{c}}} = {{3'b11}, {3{2'b10}}, {2{1'b1,sig,1'b1}}};
   
endmodule // test

module test_module;
endmodule // test_module

/*
 
 

