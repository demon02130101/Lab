module lenet_rtl_basic_dma64( clk, rst, dma_read_chnl_valid, dma_read_chnl_data, dma_read_chnl_ready,
/* <<--params-list-->> */
conf_info_scale_CONV2,
conf_info_scale_CONV3,
conf_info_scale_CONV1,
conf_info_scale_FC2,
conf_info_scale_FC1,
conf_done, acc_done, debug, dma_read_ctrl_valid, dma_read_ctrl_data_index, dma_read_ctrl_data_length, dma_read_ctrl_data_size, dma_read_ctrl_ready, dma_write_ctrl_valid, dma_write_ctrl_data_index, dma_write_ctrl_data_length, dma_write_ctrl_data_size, dma_write_ctrl_ready, dma_write_chnl_valid, dma_write_chnl_data, dma_write_chnl_ready);

   input clk;
   input rst;

   /* <<--params-def-->> */
   input wire [31:0]  conf_info_scale_CONV2;
   input wire [31:0]  conf_info_scale_CONV3;
   input wire [31:0]  conf_info_scale_CONV1;
   input wire [31:0]  conf_info_scale_FC2;
   input wire [31:0]  conf_info_scale_FC1;
   input wire 	       conf_done;

   input wire 	       dma_read_ctrl_ready;
   output reg	       dma_read_ctrl_valid;
   output reg [31:0]  dma_read_ctrl_data_index;
   output reg [31:0]  dma_read_ctrl_data_length;
   output reg [ 2:0]  dma_read_ctrl_data_size;

   output reg	       dma_read_chnl_ready;
   input wire 	       dma_read_chnl_valid;
   input wire [63:0]  dma_read_chnl_data;

   input wire         dma_write_ctrl_ready;
   output reg	       dma_write_ctrl_valid;
   output reg [31:0]  dma_write_ctrl_data_index;
   output reg [31:0]  dma_write_ctrl_data_length;
   output reg [ 2:0]  dma_write_ctrl_data_size;

   input wire 	       dma_write_chnl_ready;
   output reg	       dma_write_chnl_valid;
   output reg [63:0]  dma_write_chnl_data;

   output reg     	 acc_done;
   output reg [31:0]  debug;
   
   ///////////////////////////////////
   // Add your design here


   // Remove this part
   always @(posedge clk) begin
      acc_done <= conf_done;
   end
   
endmodule
