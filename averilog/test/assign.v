module test_module (o, f, c, o);
   parameter wd = 4;
   
   output [wd-1:0] o, f, c, d;
   wire [2:0]   o, c, f;
   reg [4+25:4'd35] sig, tmp=5;
   integer intvar, f;
   
   assign {o,{1{c}}} = {{3'b11}, {3{2'b10}}, {2{1'b1,sig,1'b1}}};
   assign f = t===1'b1 ? sig[3 +: 1][3 -: 1] : {2'd3, 5'h3} + 3**4;

   genvar i = 0;

   or_gate U0 (.out(o), .fout(f), .cin(c));
   or_gate #(.width(wd)) U1 (.out(o), .fout(f), .cin(c));
   or_gate U1 ();
   any u_0 (c);
   not (o, f, sig[0]), (c, sig[1]);

   always @(posedge clk) begin:BCLK
    if(tmp == 3) begin:TT
      tmp <= 5;
    end else if(tmp == 5)
      while(tmp < 10)
        tmp <= tmp + 1;
    else
      for(i=0; i<12; i=i+1) begin
        reg [2:0] m;
        integer m_int;
        tmp <= tmp + i;
      end
   end 

   generate
     for(i=0; i<5; i = i + 1) begin:G1
       assign c[i] = tmp[i];
     end

    case(sig)
    2'b00: assign www = sig;
    2'b01: assign xxx = sig + 1;
    endcase

    always
      o = 34;

   endgenerate

    begin
     a = b;
    end
   
endmodule // test

module test_module;
endmodule // test_module

/*
 
