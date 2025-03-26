module lenet (
    input wire clk,
    input wire rst_n,

    input wire compute_start,
    output reg compute_finish,

    // Quantization scale
    input wire [31:0] scale_CONV1,
    input wire [31:0] scale_CONV2,
    input wire [31:0] scale_CONV3,
    input wire [31:0] scale_FC1,
    input wire [31:0] scale_FC2,

    // Weight sram, dual port
    output reg [ 3:0] sram_weight_wea0,
    output reg [15:0] sram_weight_addr0,
    output reg [31:0] sram_weight_wdata0,
    input wire [31:0] sram_weight_rdata0,
    output reg [ 3:0] sram_weight_wea1,
    output reg [15:0] sram_weight_addr1,
    output reg [31:0] sram_weight_wdata1,
    input wire [31:0] sram_weight_rdata1,

    // Activation sram, dual port
    output reg [ 3:0] sram_act_wea0,
    output reg [15:0] sram_act_addr0,
    output reg [31:0] sram_act_wdata0,
    input wire [31:0] sram_act_rdata0,
    output reg [ 3:0] sram_act_wea1,
    output reg [15:0] sram_act_addr1,
    output reg [31:0] sram_act_wdata1,
    input wire [31:0] sram_act_rdata1
);
    // Add your design here

    // flip flop before input and output data ports
    reg rst_n_ff;

    reg compute_start_ff;

    // output
    reg compute_finish_ff;

    // Quantization scale
    reg signed [31:0] scale_CONV1_ff;
    reg signed [31:0] scale_CONV2_ff;
    reg signed [31:0] scale_CONV3_ff;
    reg signed [31:0] scale_FC1_ff;
    reg signed [31:0] scale_FC2_ff;

    // Weight sram, dual port, output
    // reg [ 3:0] sram_weight_wea0_ff;
    // reg [15:0] sram_weight_addr0_ff;
    // reg [31:0] sram_weight_wdata0_ff;

    // reg [ 3:0] sram_weight_wea1_ff;
    // reg [15:0] sram_weight_addr1_ff;
    // reg [31:0] sram_weight_wdata1_ff;

    // input
    reg [31:0] sram_weight_rdata0_ff;
    reg [31:0] sram_weight_rdata1_ff;

    // Activation sram, dual port, output
    // reg [ 3:0] sram_act_wea0_ff;
    // reg [15:0] sram_act_addr0_ff;
    // reg [31:0] sram_act_wdata0_ff;

    // reg [ 3:0] sram_act_wea1_ff;
    // reg [15:0] sram_act_addr1_ff;
    // reg [31:0] sram_act_wdata1_ff;

    // input
    reg [31:0] sram_act_rdata0_ff;
    reg [31:0] sram_act_rdata1_ff;

    /********************************************/
    // lenet module FSM state and next state
    reg [ 2:0] state;
    reg [ 2:0] next_state;
    reg all_finish;
    
    // each state
    parameter IDLE_STATE  = 3'b000;
    parameter CONV1_STATE = 3'b001;
    parameter CONV2_STATE = 3'b010;
    parameter CONV3_STATE = 3'b011;
    parameter FC1W_STATE  = 3'b100;
    parameter FC2W_STATE  = 3'b101;
    parameter FC2B_STATE  = 3'b110;
    parameter DONE_STATE  = 3'b111;

    // 決定送進module內的scale
    reg signed [31:0] send_scale;
    /********************************************/
    // conv1 PO wire
    reg  conv1_start;
    wire conv1_finish;

    wire [ 3:0] conv1_sram_weight_wea0;
    wire [15:0] conv1_sram_weight_addr0;
    wire [31:0] conv1_sram_weight_wdata0;

    wire [ 3:0] conv1_sram_weight_wea1;
    wire [15:0] conv1_sram_weight_addr1;
    wire [31:0] conv1_sram_weight_wdata1;

    wire [ 3:0] conv1_sram_act_wea0;
    wire [15:0] conv1_sram_act_addr0;
    wire [31:0] conv1_sram_act_wdata0;

    wire [ 3:0] conv1_sram_act_wea1;
    wire [15:0] conv1_sram_act_addr1;
    wire [31:0] conv1_sram_act_wdata1;

    /********************************************/
    // fc1 PO wire
    reg  fc1_start;
    wire fc1_finish;

    wire [ 3:0] fc1_sram_weight_wea0;
    wire [15:0] fc1_sram_weight_addr0;
    wire [31:0] fc1_sram_weight_wdata0;

    wire [ 3:0] fc1_sram_weight_wea1;
    wire [15:0] fc1_sram_weight_addr1;
    wire [31:0] fc1_sram_weight_wdata1;

    wire [ 3:0] fc1_sram_act_wea0;
    wire [15:0] fc1_sram_act_addr0;
    wire [31:0] fc1_sram_act_wdata0;

    wire [ 3:0] fc1_sram_act_wea1;
    wire [15:0] fc1_sram_act_addr1;
    wire [31:0] fc1_sram_act_wdata1;
    /********************************************/

    // update next state to cur state
    always @(posedge clk or negedge rst_n_ff) begin
        if(!rst_n_ff)
            state <= IDLE_STATE;
        else
            state <= next_state;
    end

    // next state update
    always @(*) begin
        next_state = 0;

        case(state) 
            IDLE_STATE: begin
                if(compute_start_ff)
                    next_state = CONV1_STATE;
                else
                    next_state = IDLE_STATE;
            end
            CONV1_STATE: begin
                if(conv1_finish)
                    next_state = CONV2_STATE;      
                else
                    next_state = CONV1_STATE;
            end
            CONV2_STATE: begin
                if(conv1_finish)
                    next_state = CONV3_STATE;       // change later
                else
                    next_state = CONV2_STATE;
            end
            CONV3_STATE: begin
                if(fc1_finish)
                    next_state = FC1W_STATE;       // change later
                else
                    next_state = CONV3_STATE;
            end
            FC1W_STATE: begin
                if(fc1_finish)
                    next_state = FC2W_STATE;       // change later
                else
                    next_state = FC1W_STATE;
            end
            FC2W_STATE: begin
                if(fc1_finish)
                    next_state = DONE_STATE;       // change later
                else
                    next_state = FC2W_STATE;
            end
            DONE_STATE:
                    next_state = IDLE_STATE;
            default:
                next_state = IDLE_STATE;
        endcase
    end

    // control signal
    always @(*) begin
        conv1_start = 0;
        fc1_start = 0;
        all_finish = 0;

        case(state)
            IDLE_STATE: begin

            end
            CONV1_STATE: begin
                conv1_start = 1;
            end
            CONV2_STATE: begin
                conv1_start = 1;
            end
            CONV3_STATE: begin
                fc1_start = 1;
            end
            FC1W_STATE: begin
                fc1_start = 1;
            end
            FC2W_STATE: begin
                fc1_start = 1;
            end
            DONE_STATE:
                all_finish = 1;                     
        endcase
    end

    // special reset signal
    always @(posedge clk) begin
        rst_n_ff <= rst_n;
    end

    // PI filp flop
    always @(posedge clk or negedge rst_n_ff) begin
        if(!rst_n_ff) begin
            compute_start_ff            <= 0;

            scale_CONV1_ff              <= 0;
            scale_CONV2_ff              <= 0;
            scale_CONV3_ff              <= 0;
            scale_FC1_ff                <= 0;
            scale_FC2_ff                <= 0;

            sram_weight_rdata0_ff       <= 0;
            sram_weight_rdata1_ff       <= 0;

            sram_act_rdata0_ff          <= 0; 
            sram_act_rdata1_ff          <= 0; 
        end
        else begin
            compute_start_ff            <= compute_start;

            scale_CONV1_ff              <= scale_CONV1;
            scale_CONV2_ff              <= scale_CONV2;
            scale_CONV3_ff              <= scale_CONV3;
            scale_FC1_ff                <= scale_FC1;
            scale_FC2_ff                <= scale_FC2;

            sram_weight_rdata0_ff       <= sram_weight_rdata0;
            sram_weight_rdata1_ff       <= sram_weight_rdata1;

            sram_act_rdata0_ff          <= sram_act_rdata0; 
            sram_act_rdata1_ff          <= sram_act_rdata1; 
        end
    end  

    // PO filp flop
    // should change later
    always @(posedge clk or negedge rst_n_ff) begin
        if(!rst_n_ff) begin
            compute_finish          <= 0;

            sram_weight_wea0        <= 0;
            sram_weight_addr0       <= 0;
            sram_weight_wdata0      <= 0;
            sram_weight_wea1        <= 0;
            sram_weight_addr1       <= 0;
            sram_weight_wdata1      <= 0;

            sram_act_wea0        <= 0;
            sram_act_addr0       <= 0;
            sram_act_wdata0      <= 0;
            sram_act_wea1        <= 0;
            sram_act_addr1       <= 0;
            sram_act_wdata1      <= 0;
        end
        else begin
            compute_finish         <= all_finish;               // TODO

            sram_weight_wea0       <= (state <= CONV2_STATE) ? conv1_sram_weight_wea0 : fc1_sram_weight_wea0;
            sram_weight_addr0      <= (state <= CONV2_STATE) ? conv1_sram_weight_addr0 : fc1_sram_weight_addr0;
            sram_weight_wdata0     <= (state <= CONV2_STATE) ? conv1_sram_weight_wdata0 : fc1_sram_weight_wdata0;
            sram_weight_wea1       <= (state <= CONV2_STATE) ? conv1_sram_weight_wea1 : fc1_sram_weight_wea1;
            sram_weight_addr1      <= (state <= CONV2_STATE) ? conv1_sram_weight_addr1 : fc1_sram_weight_addr1;
            sram_weight_wdata1     <= (state <= CONV2_STATE) ? conv1_sram_weight_wdata1 : fc1_sram_weight_wdata1;

            sram_act_wea0          <= (state <= CONV2_STATE) ? conv1_sram_act_wea0 : fc1_sram_act_wea0;
            sram_act_addr0         <= (state <= CONV2_STATE) ? conv1_sram_act_addr0 : fc1_sram_act_addr0;
            sram_act_wdata0        <= (state <= CONV2_STATE) ? conv1_sram_act_wdata0 : fc1_sram_act_wdata0;
            sram_act_wea1          <= (state <= CONV2_STATE) ? conv1_sram_act_wea1 : fc1_sram_act_wea1;
            sram_act_addr1         <= (state <= CONV2_STATE) ? conv1_sram_act_addr1 : fc1_sram_act_addr1;
            sram_act_wdata1        <= (state <= CONV2_STATE) ? conv1_sram_act_wdata1 : fc1_sram_act_wdata1;
        end
    end    

    always @(*) begin
        send_scale = 0;

        case(state)
            CONV1_STATE:
                send_scale = scale_CONV1_ff;
            CONV2_STATE:
                send_scale = scale_CONV2_ff;
            CONV3_STATE:
                send_scale = scale_CONV3_ff;
            FC1W_STATE:
                send_scale = scale_FC1_ff;
            FC2W_STATE:
                send_scale = scale_FC2_ff;
            FC2B_STATE:
                send_scale = scale_FC2_ff;
            DONE_STATE:
                send_scale = 0;
            default:
                send_scale = 0;
        endcase
    end

    FC fc(
        .clk(clk),
        .rst_n(rst_n_ff),  

        .fc1_start(fc1_start),
        .fc1_finish(fc1_finish),

        .scale_FC1(send_scale),
 
        .sram_weight_wea0(fc1_sram_weight_wea0),
        .sram_weight_addr0(fc1_sram_weight_addr0),
        .sram_weight_wdata0(fc1_sram_weight_wdata0),
        .sram_weight_rdata0(sram_weight_rdata0_ff),
        .sram_weight_wea1(fc1_sram_weight_wea1),
        .sram_weight_addr1(fc1_sram_weight_addr1),
        .sram_weight_wdata1(fc1_sram_weight_wdata1),
        .sram_weight_rdata1(sram_weight_rdata1_ff),

        .sram_act_wea0(fc1_sram_act_wea0),
        .sram_act_addr0(fc1_sram_act_addr0),
        .sram_act_wdata0(fc1_sram_act_wdata0),
        .sram_act_rdata0(sram_act_rdata0_ff),
        .sram_act_wea1(fc1_sram_act_wea1),
        .sram_act_addr1(fc1_sram_act_addr1),
        .sram_act_wdata1(fc1_sram_act_wdata1),
        .sram_act_rdata1(sram_act_rdata1_ff),

        .outter_state(state)
    );

    conv1 conv1_1(
        .clk(clk),
        .rst_n(rst_n_ff),  

        .conv1_start(conv1_start),
        .conv1_finish(conv1_finish),

        .scale_CONV1(send_scale),
 
        .sram_weight_wea0(conv1_sram_weight_wea0),
        .sram_weight_addr0(conv1_sram_weight_addr0),
        .sram_weight_wdata0(conv1_sram_weight_wdata0),
        .sram_weight_rdata0(sram_weight_rdata0_ff),
        .sram_weight_wea1(conv1_sram_weight_wea1),
        .sram_weight_addr1(conv1_sram_weight_addr1),
        .sram_weight_wdata1(conv1_sram_weight_wdata1),
        .sram_weight_rdata1(sram_weight_rdata1_ff),

        .sram_act_wea0(conv1_sram_act_wea0),
        .sram_act_addr0(conv1_sram_act_addr0),
        .sram_act_wdata0(conv1_sram_act_wdata0),
        .sram_act_rdata0(sram_act_rdata0_ff),
        .sram_act_wea1(conv1_sram_act_wea1),
        .sram_act_addr1(conv1_sram_act_addr1),
        .sram_act_wdata1(conv1_sram_act_wdata1),
        .sram_act_rdata1(sram_act_rdata1_ff),

        .is_conv1(state[0])
    );

endmodule

module conv1(
    input wire clk,
    input wire rst_n,  

    input wire conv1_start,
    output reg conv1_finish,

    input wire [31:0] scale_CONV1,

    // Weight sram, dual port
    output reg [ 3:0] sram_weight_wea0,
    output reg [15:0] sram_weight_addr0,
    output reg [31:0] sram_weight_wdata0,
    input wire [31:0] sram_weight_rdata0,
    output reg [ 3:0] sram_weight_wea1,
    output reg [15:0] sram_weight_addr1,
    output reg [31:0] sram_weight_wdata1,
    input wire [31:0] sram_weight_rdata1,

    // Activation sram, dual port
    output reg [ 3:0] sram_act_wea0,
    output reg [15:0] sram_act_addr0,
    output reg [31:0] sram_act_wdata0,
    input wire [31:0] sram_act_rdata0,
    output reg [ 3:0] sram_act_wea1,
    output reg [15:0] sram_act_addr1,
    output reg [31:0] sram_act_wdata1,
    input wire [31:0] sram_act_rdata1,

    // some range setting
    input wire is_conv1    // true: 現在做conv1運算，false: 現在做conv2運算
);
    parameter IDLE_STATE  = 2'b00;
    parameter READ_STATE  = 2'b01;
    parameter WRITE_STATE = 2'b10;
    parameter DONE_STATE  = 2'b11;

    reg [ 1:0] state;
    reg [ 1:0] next_state;

    // oeo
    reg [ 2:0] inputMapIdx, next_inputMapIdx;
    reg [ 4:0] inputRowidx, next_inputRowidx;
    reg [ 2:0] inputColIdx, next_inputColIdx;

    reg [ 2:0] weightConvRowIdx, next_weightConvRowIdx;
    reg [ 2:0] actConvRowIdx, next_actConvRowIdx;

    reg [ 2:0] weightConvRowIdx_delay3[ 2:0];
    reg [ 2:0] actConvRowIdx_delay3 [2:0];
    
    reg [ 4:0] weightMapIdx, next_weightMapIdx;

    reg [ 3:0] outputCnt, next_outputCnt;   // 累積8個寫出去一次
    reg [ 3:0] outputCnt_delay3 [ 2:0];   // 累積8個寫出去一次

    reg [ 8:0] writeOutputCnt, next_writeOutputCnt; // 寫到sram的counter數量

    // I hate this...
    reg [ 2:0] writeDelayCounter, next_writeDelayCounter;
    reg [ 1:0] readTopThreeCycleCounter, next_readTopThreeCycleCounter;
    reg [ 1:0] doneDelayCounter, next_doneDelayCounter;

    reg convRowDelayCounter, next_convRowDelayCounter;  // 每個weight row要維持 2 cycles
    reg actRowDelayCounter, next_actRowDelayCounter;  // 除了第0和第5個row外，每個act row要維持 2 cycles

    reg [ 7:0] actWriteData [ 0:7];
    reg [ 7:0] next_actWriteData [ 0:7];
    reg [ 7:0] remain_res, next_remain_res;

    // limit
    wire [ 2:0] input_map_sz;
    wire [ 4:0] input_row_sz;
    wire [ 2:0] input_col_sz;
    wire [ 8:0] input_sram_offset;

    wire [ 4:0] weight_map_sz;
    wire [ 5:0] weight_sram_offset;

    wire [10:0] output_sram_offset;

    wire [ 2:0] sram_data_delay;

    wire [ 5:0] weight_map_element_num;  // 每個output map要跨多少index 

    wire lastrow_odd;   // 如果為1代表最後一個col只能提供一組解

    // partial sum
    reg signed [31:0] partial_sum_1[ 0:3];  // 每4個一組
    reg signed [31:0] partial_sum_2[ 0:3];  
    reg signed [31:0] next_partial_sum_1[ 0:3];  // 每4個一組
    reg signed [31:0] next_partial_sum_2[ 0:3];  

    reg signed [15:0] approx_res_1[ 0:3];   // (next_partial sum*res) >>> 16
    reg signed [15:0] approx_res_2[ 0:3];   // (next_partial sum*res) >>> 16

    reg signed [7:0] relu_res_1[ 0:3];
    reg signed [7:0] relu_res_2[ 0:3];

    reg signed [7:0] maxPool_res_1;
    reg signed [7:0] maxPool_res_2;

    // loop index
    integer i, j, k;

    reg signed [31:0] MAC_res[ 0:3];


    assign input_map_sz = (is_conv1) ? 3'd0 : 3'd5;
    assign input_row_sz = (is_conv1) ? 5'd13 : 5'd4;
    assign input_col_sz = (is_conv1) ? 3'd6 : 3'd2;     // 代表以該column為首的6*6 window
    assign input_sram_offset = (is_conv1) ? 9'd0 : 9'd256;

    assign weight_map_sz = (is_conv1) ? 5'd6 : 5'd16;
    assign weight_sram_offset = (is_conv1) ? 6'd0 : 6'd60;

    assign output_sram_offset = (is_conv1) ? 11'd256 : 11'd592;

    assign sram_data_delay = 3'd3;      // 多把write延長1個cycle

    assign weight_map_element_num = (is_conv1) ? 6'd10 : 6'd60;

    // ex. conv2的output_row_sz = 4，他每個row的最後一個col只能提供一組解
    assign lastrow_odd = !(input_row_sz[0]); 
    

    /****************** FSM *****************/
    // state transistion
    always @* begin
        case(state)
            IDLE_STATE: begin
                if(conv1_start)
                    next_state = READ_STATE;
                else
                    next_state = IDLE_STATE;
            end
            READ_STATE: begin 
                // TODO
                // 這樣代表做了mod (未來的值比現在小)
                // 或是全部都執行完畢了
                if((outputCnt > next_outputCnt) || (weightMapIdx == weight_map_sz && (actConvRowIdx == 3'd5 && inputMapIdx == input_map_sz && inputColIdx == input_col_sz && inputRowidx == input_row_sz)))
                    next_state = WRITE_STATE;
                else
                    next_state = READ_STATE;
            end
            WRITE_STATE: begin  
                if(writeDelayCounter == sram_data_delay) begin
                    if(weightMapIdx == weight_map_sz)
                        next_state = DONE_STATE;
                    else
                        next_state = READ_STATE;
                end
                else
                   next_state = WRITE_STATE;
            end
            DONE_STATE: begin 
                if(doneDelayCounter == 2)
                    next_state = IDLE_STATE;
                else
                    next_state = DONE_STATE;

            end
        endcase
    end

    // FSM output
    always @* begin
        conv1_finish = 0;

        sram_weight_wea0 = 0;
        sram_weight_wea1 = 0;

        sram_act_wea0 = 0;
        sram_act_wea1 = 0;

        sram_act_addr0 = 0;
        sram_act_addr1 = 0;

        sram_weight_addr0 = 0;
        sram_weight_addr1 = 0;

        sram_act_wdata0 = 0;
        sram_act_wdata1 = 0;

        sram_weight_wdata0 = 0;
        sram_weight_wdata1 = 0;

        // conv2中每個input layer有56個elements(4*14) => x*64 - x*8
        case(state)
            READ_STATE: begin
                sram_act_addr0 = input_sram_offset + ((inputMapIdx<<6)-(inputMapIdx<<3)) + ((inputRowidx << 1)+actConvRowIdx)*(input_col_sz+2) + inputColIdx;
                sram_act_addr1 = input_sram_offset + ((inputMapIdx<<6)-(inputMapIdx<<3)) + ((inputRowidx << 1)+actConvRowIdx)*(input_col_sz+2) + inputColIdx + 1;

                // TODO conv2
                // 考慮input map
                sram_weight_addr0 = weight_sram_offset + (weightMapIdx*weight_map_element_num) + ((inputMapIdx << 3) + (inputMapIdx << 1)) + (weightConvRowIdx << 1);
                sram_weight_addr1 = weight_sram_offset + (weightMapIdx*weight_map_element_num) + ((inputMapIdx << 3) + (inputMapIdx << 1)) + (weightConvRowIdx << 1)  + 1;
            end
            WRITE_STATE: begin
                if(writeDelayCounter == 3) begin
                    sram_act_wea0 = 4'b1111;
                    sram_act_wea1 = 4'b1111;    

                    sram_act_wdata0[ 7: 0] = actWriteData[0];
                    sram_act_wdata0[15: 8] = actWriteData[1];
                    sram_act_wdata0[23:16] = actWriteData[2];
                    sram_act_wdata0[31:24] = actWriteData[3]; 

                    sram_act_wdata1[ 7: 0] = actWriteData[4];
                    sram_act_wdata1[15: 8] = actWriteData[5];
                    sram_act_wdata1[23:16] = actWriteData[6];
                    sram_act_wdata1[31:24] = actWriteData[7];      

                    sram_act_addr0 = output_sram_offset + (writeOutputCnt << 1);
                    sram_act_addr1 = output_sram_offset + (writeOutputCnt << 1) + 1;      
                end
            end
            DONE_STATE: begin
                if(doneDelayCounter == 2)
                    conv1_finish = 1;
            end
        endcase
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            state <= IDLE_STATE;
        else
            state <= next_state;
    end
    /****************** FSM *****************/

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            inputMapIdx <= 0;
            inputRowidx <= 0;
            inputColIdx <= 0;

            weightConvRowIdx <= 0;
            actConvRowIdx <= 0;

            weightConvRowIdx_delay3[0] <= 0;
            weightConvRowIdx_delay3[1] <= 0;
            weightConvRowIdx_delay3[2] <= 0;

            actConvRowIdx_delay3[0] <= 0;
            actConvRowIdx_delay3[1] <= 0;
            actConvRowIdx_delay3[2] <= 0;

            weightMapIdx <= 0;

            writeDelayCounter <= 0;

            readTopThreeCycleCounter <= 0;
            doneDelayCounter <= 0;

            outputCnt <= 0;
            outputCnt_delay3[0] <= 0;
            outputCnt_delay3[1] <= 0;
            outputCnt_delay3[2] <= 0;

            convRowDelayCounter <= 0;
            actRowDelayCounter <= 0;

            remain_res <= 0;

            writeOutputCnt <= 0;

            for(i = 0; i < 4; i=i+1) begin
                partial_sum_1[i] <= 0;
                partial_sum_2[i] <= 0;
            end

            for(i = 0; i < 8; i = i+1) 
                actWriteData[i] <= 0;
        end
        else begin
            inputMapIdx <= next_inputMapIdx;
            inputRowidx <= next_inputRowidx;
            inputColIdx <= next_inputColIdx;

            weightConvRowIdx <= next_weightConvRowIdx;
            actConvRowIdx <= next_actConvRowIdx;

            weightConvRowIdx_delay3[0] <= weightConvRowIdx;
            weightConvRowIdx_delay3[1] <= weightConvRowIdx_delay3[0];
            weightConvRowIdx_delay3[2] <= weightConvRowIdx_delay3[1];

            actConvRowIdx_delay3[0] <= actConvRowIdx;
            actConvRowIdx_delay3[1] <= actConvRowIdx_delay3[0];
            actConvRowIdx_delay3[2] <= actConvRowIdx_delay3[1];

            weightMapIdx <= next_weightMapIdx;

            writeDelayCounter <= next_writeDelayCounter;

            readTopThreeCycleCounter <= next_readTopThreeCycleCounter;
            doneDelayCounter <= next_doneDelayCounter;

            outputCnt <= next_outputCnt;
            outputCnt_delay3[0] <= outputCnt;
            outputCnt_delay3[1] <= outputCnt_delay3[0];
            outputCnt_delay3[2] <= outputCnt_delay3[1];

            convRowDelayCounter <= next_convRowDelayCounter;
            actRowDelayCounter <= next_actRowDelayCounter;

            remain_res <= next_remain_res;

            writeOutputCnt <= next_writeOutputCnt;

            for(i = 0; i < 4; i=i+1) begin
                partial_sum_1[i] <= next_partial_sum_1[i];
                partial_sum_2[i] <= next_partial_sum_2[i];
            end

            for(i = 0; i < 8; i = i+1) 
                actWriteData[i] <= next_actWriteData[i];
        end
    end

    // ******************控制IA Row讀取******** //
    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 0)
                next_actConvRowIdx = actConvRowIdx + 1;
            else if(actConvRowIdx == 3'd5)
                next_actConvRowIdx = 0;
            else if(actRowDelayCounter == 1)
                next_actConvRowIdx = actConvRowIdx + 1;
            else
                next_actConvRowIdx = actConvRowIdx;
        end
        else if(state == DONE_STATE)
            next_actConvRowIdx = 0;
        else
            next_actConvRowIdx = actConvRowIdx;        
    end

    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 0 || actConvRowIdx == 3'd5)
                next_actRowDelayCounter = 0;
            else if(actRowDelayCounter < 1)
                next_actRowDelayCounter = actRowDelayCounter + 1;
            else
                next_actRowDelayCounter = 0;
        end
        else
            next_actRowDelayCounter = 0;
    end
    // ******************控制IA Row讀取******** //

    // ******************控制Weight Row讀取******** //
    always @(*) begin
        if(state == READ_STATE) begin
            if(convRowDelayCounter == 1)
                next_weightConvRowIdx = (weightConvRowIdx == 3'd4) ? 0 : weightConvRowIdx + 1;
            else
                next_weightConvRowIdx = weightConvRowIdx;
        end
        else if(state == DONE_STATE)
            next_weightConvRowIdx = 0;
        else
            next_weightConvRowIdx = weightConvRowIdx;
    end

    always @(*) begin
        if(state == READ_STATE) begin
            if(convRowDelayCounter < 1)
                next_convRowDelayCounter = convRowDelayCounter + 1;
            else
                next_convRowDelayCounter = 0;
        end
        else
            next_convRowDelayCounter = 0;
    end
    // ******************控制Weight Row讀取******** //

    // ************ Input Map Index ************** //
    // 讀完actConvRow時，換到下一個input map
    // 如果input map index == input_map_sz, 代表有"2個byte"結果運算完成!! => 兩個case (case1)在conv2運算中outputCnt + 2，累積4個即可輸出
    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 3'd5)
                next_inputMapIdx = (inputMapIdx == input_map_sz) ? 0 : inputMapIdx + 1;
            else
                next_inputMapIdx = inputMapIdx;
        end
        else if(state == DONE_STATE)
            next_inputMapIdx = 0;
        else
            next_inputMapIdx = inputMapIdx;
    end
    // ************ Input Map Index ************** //

    // ************ Input Col Index ************** //
    // 代表IA 最左邊的column index
    // 讀完所有input map時，切換到下一個col
    // 如果input col index == input_col_sz, 切換到下一個input row
    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 3'd5 && inputMapIdx == input_map_sz)
                next_inputColIdx = (inputColIdx == input_col_sz) ? 0 : inputColIdx + 1;
            else
                next_inputColIdx = inputColIdx;
        end
        else if(state == DONE_STATE)
            next_inputColIdx = 0;
        else
            next_inputColIdx = inputColIdx;
    end
    // ************ Input Col Index ************** //

    // ************* Input Row Index ************ //
    // 代表6*6 convolution window中第一個row的index
    // 如果inputRowidx = input_row_sz，切換到下一個weight map
    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 3'd5 && inputMapIdx == input_map_sz && inputColIdx == input_col_sz)
                next_inputRowidx = (inputRowidx == input_row_sz) ? 0 : inputRowidx + 1;
            else
                next_inputRowidx = inputRowidx;
        end
        else if(state == DONE_STATE)
            next_inputRowidx = 0;
        else
            next_inputRowidx = inputRowidx;
    end
    // ************* Input Row Index ************ //

    // ************ weight map index *********** //
    // 代表當前的weight map index
    // 如果weight map index == weight map sz，這一整層運算就做完了
    always @(*) begin
        if(state == READ_STATE) begin
            if(actConvRowIdx == 3'd5 && inputMapIdx == input_map_sz && inputColIdx == input_col_sz && inputRowidx == input_row_sz)
                next_weightMapIdx = (weightMapIdx == weight_map_sz) ? weight_map_sz : weightMapIdx + 1;
            else
                next_weightMapIdx = weightMapIdx;
        end
        else if(state == DONE_STATE)
            next_weightMapIdx = 0;
        else
            next_weightMapIdx = weightMapIdx;
    end
    // ************ weight map index *********** //

    // *********** output count ***************// 
    // 代表當前有多少個準備好輸出的output
    // 如果outputCnt== 7 或是在conv1中每個row走到底，輸出一次答案
    always @(*) begin
        next_outputCnt = outputCnt;

        if (state == READ_STATE) begin
            if (actConvRowIdx == 3'd5 && inputMapIdx == input_map_sz) begin
                if (is_conv1) begin
                    if(inputColIdx == input_col_sz)
                        next_outputCnt = 0;
                    else
                        next_outputCnt = (outputCnt + 2) & 4'd7;
                end 
                else begin  // conv2
                    if (inputColIdx == input_col_sz && lastrow_odd) // 每個row的最後一段col只能提供一個byte的輸出
                        next_outputCnt = (outputCnt + 1) & 4'd7;
                    else
                        next_outputCnt = (outputCnt + 2) & 4'd7;
                end
            end
        end else if (state == DONE_STATE) begin
            next_outputCnt = 0;
        end
    end
    // *********** output count ***************//

    // *********** delay counter ************* //
    // 每個read state前三個cycle的值為錯誤位置，不要計算
    always @(*) begin
        if(state == READ_STATE) begin
            if(readTopThreeCycleCounter < 2'd3)
                next_readTopThreeCycleCounter = readTopThreeCycleCounter + 1;
            else
                next_readTopThreeCycleCounter = readTopThreeCycleCounter;
        end
        else if(next_state == READ_STATE)  // note 和之前不同的寫法
            next_readTopThreeCycleCounter = 0;
        else
            next_readTopThreeCycleCounter = readTopThreeCycleCounter;
    end

    // 將write state延後三個cycle，以處理完所有數據
    always @(*) begin
        if(state == WRITE_STATE) begin
            if(writeDelayCounter < sram_data_delay)
                next_writeDelayCounter = writeDelayCounter + 1;
            else
                next_writeDelayCounter = 0;
        end
        else
            next_writeDelayCounter = 0;
    end

    always @(*) begin
        if(state == DONE_STATE) begin
            if(doneDelayCounter < 2)
                next_doneDelayCounter = doneDelayCounter + 1;
            else
                next_doneDelayCounter = 0;
        end
        else
            next_doneDelayCounter = 0;
    end
    // *********** delay counter ************* //

    // *********** MAC result ******************//
    always @(*) begin
        MAC_res[0] =  $signed(sram_act_rdata0[ 7: 0]) * $signed(sram_weight_rdata0[ 7: 0])
                    + $signed(sram_act_rdata0[15: 8]) * $signed(sram_weight_rdata0[15: 8])
                    + $signed(sram_act_rdata0[23:16]) * $signed(sram_weight_rdata0[23:16])
                    + $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[31:24])
                    + $signed(sram_act_rdata1[ 7: 0]) * $signed(sram_weight_rdata1[ 7: 0]);

        MAC_res[1] =  $signed(sram_act_rdata0[15: 8]) * $signed(sram_weight_rdata0[ 7: 0])
                    + $signed(sram_act_rdata0[23:16]) * $signed(sram_weight_rdata0[15: 8])
                    + $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[23:16])
                    + $signed(sram_act_rdata1[ 7: 0]) * $signed(sram_weight_rdata0[31:24])
                    + $signed(sram_act_rdata1[15: 8]) * $signed(sram_weight_rdata1[ 7: 0]);

        MAC_res[2] =  $signed(sram_act_rdata0[23:16]) * $signed(sram_weight_rdata0[ 7: 0])
                    + $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[15: 8])
                    + $signed(sram_act_rdata1[ 7: 0]) * $signed(sram_weight_rdata0[23:16])
                    + $signed(sram_act_rdata1[15: 8]) * $signed(sram_weight_rdata0[31:24])
                    + $signed(sram_act_rdata1[23:16]) * $signed(sram_weight_rdata1[ 7: 0]);

        MAC_res[3] =  $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[ 7: 0])
                    + $signed(sram_act_rdata1[ 7: 0]) * $signed(sram_weight_rdata0[15: 8])
                    + $signed(sram_act_rdata1[15: 8]) * $signed(sram_weight_rdata0[23:16])
                    + $signed(sram_act_rdata1[23:16]) * $signed(sram_weight_rdata0[31:24])
                    + $signed(sram_act_rdata1[31:24]) * $signed(sram_weight_rdata1[ 7: 0]);
    end

    // *********** partial sum **************** //
    always @(*) begin
        next_partial_sum_1[0] = 0;
        next_partial_sum_1[1] = 0;
        next_partial_sum_1[2] = 0;
        next_partial_sum_1[3] = 0;

        next_partial_sum_2[0] = 0;
        next_partial_sum_2[1] = 0;
        next_partial_sum_2[2] = 0;
        next_partial_sum_2[3] = 0;

        if(state == WRITE_STATE || state == READ_STATE) begin
            if(readTopThreeCycleCounter == 3 && outputCnt_delay3[2] == outputCnt_delay3[1]) begin
                if(actConvRowIdx_delay3[2] == weightConvRowIdx_delay3[2]) begin
                    next_partial_sum_1[0] = partial_sum_1[0] + MAC_res[0];
                    next_partial_sum_1[1] = partial_sum_1[1] + MAC_res[1];
                    next_partial_sum_1[2] = partial_sum_1[2];
                    next_partial_sum_1[3] = partial_sum_1[3];

                    next_partial_sum_2[0] = partial_sum_2[0] + MAC_res[2];
                    next_partial_sum_2[1] = partial_sum_2[1] + MAC_res[3];
                    next_partial_sum_2[2] = partial_sum_2[2];
                    next_partial_sum_2[3] = partial_sum_2[3];
                end
                else begin
                    next_partial_sum_1[0] = partial_sum_1[0];
                    next_partial_sum_1[1] = partial_sum_1[1];
                    next_partial_sum_1[2] = partial_sum_1[2] + MAC_res[0];
                    next_partial_sum_1[3] = partial_sum_1[3] + MAC_res[1];

                    next_partial_sum_2[0] = partial_sum_2[0];
                    next_partial_sum_2[1] = partial_sum_2[1];
                    next_partial_sum_2[2] = partial_sum_2[2] + MAC_res[2];
                    next_partial_sum_2[3] = partial_sum_2[3] + MAC_res[3];                    
                end
            end
            else begin
                next_partial_sum_1[0] = 0;
                next_partial_sum_1[1] = 0;
                next_partial_sum_1[2] = 0;
                next_partial_sum_1[3] = 0;

                next_partial_sum_2[0] = 0;
                next_partial_sum_2[1] = 0;
                next_partial_sum_2[2] = 0;
                next_partial_sum_2[3] = 0;            
            end
        end
    end

    // ********************** approx res *************** //
    always @(*) begin
        approx_res_1[0] = ((partial_sum_1[0]             )*scale_CONV1) >>> 16;
        approx_res_1[1] = ((partial_sum_1[1]             )*scale_CONV1) >>> 16;
        approx_res_1[2] = ((partial_sum_1[2] + MAC_res[0])*scale_CONV1) >>> 16;
        approx_res_1[3] = ((partial_sum_1[3] + MAC_res[1])*scale_CONV1) >>> 16;

        approx_res_2[0] = ((partial_sum_2[0]             )*scale_CONV1) >>> 16;
        approx_res_2[1] = ((partial_sum_2[1]             )*scale_CONV1) >>> 16;
        approx_res_2[2] = ((partial_sum_2[2] + MAC_res[2])*scale_CONV1) >>> 16;
        approx_res_2[3] = ((partial_sum_2[3] + MAC_res[3])*scale_CONV1) >>> 16;          
    end

    // ******************* relu res ******************* //
    always @(*) begin
        relu_res_1[0] = (approx_res_1[0][15] == 1) ? 0 : approx_res_1[0][7:0];    // 負數
        relu_res_1[1] = (approx_res_1[1][15] == 1) ? 0 : approx_res_1[1][7:0];    // 負數
        relu_res_1[2] = (approx_res_1[2][15] == 1) ? 0 : approx_res_1[2][7:0];    // 負數
        relu_res_1[3] = (approx_res_1[3][15] == 1) ? 0 : approx_res_1[3][7:0];    // 負數

        relu_res_2[0] = (approx_res_2[0][15] == 1) ? 0 : approx_res_2[0][7:0];    // 負數
        relu_res_2[1] = (approx_res_2[1][15] == 1) ? 0 : approx_res_2[1][7:0];    // 負數
        relu_res_2[2] = (approx_res_2[2][15] == 1) ? 0 : approx_res_2[2][7:0];    // 負數
        relu_res_2[3] = (approx_res_2[3][15] == 1) ? 0 : approx_res_2[3][7:0];    // 負數        
    end

    reg signed [7:0] temp1, temp2, temp3, temp4;
    // ****************** max pool ******************* //
    always @(*) begin
        temp1 = (relu_res_1[0] > relu_res_1[1]) ? relu_res_1[0] : relu_res_1[1];
        temp2 = (relu_res_1[2] > relu_res_1[3]) ? relu_res_1[2] : relu_res_1[3];
        maxPool_res_1 = (temp1 > temp2) ? temp1 : temp2;
        
        temp3 = (relu_res_2[0] > relu_res_2[1]) ? relu_res_2[0] : relu_res_2[1];
        temp4 = (relu_res_2[2] > relu_res_2[3]) ? relu_res_2[2] : relu_res_2[3];
        maxPool_res_2 = (temp3 > temp4) ? temp3 : temp4;
    end

    // *********** Write data *******************  //
    always @(*) begin
        for(i = 0; i < 8; i = i+1)
            next_actWriteData[i] = actWriteData[i];
        next_remain_res = remain_res;

        if(writeDelayCounter == 3) begin
            for(i = 1; i < 8; i = i+1)
                next_actWriteData[i] = 0;         
            next_actWriteData[0] =  next_remain_res;
        end
        else if(outputCnt_delay3[2] == 7 && outputCnt_delay3[1] == 1) begin
            next_actWriteData[7] = maxPool_res_1;
            next_remain_res      = maxPool_res_2;
        end
        else if(outputCnt_delay3[2] != outputCnt_delay3[1]) begin
            next_actWriteData[outputCnt_delay3[2]]   = maxPool_res_1;
            next_actWriteData[outputCnt_delay3[2]+1] = maxPool_res_2;
        end
    end

    // *********** output counter to SRAM ******** //
    always @(*) begin
        next_writeOutputCnt = 0;

        if(state == READ_STATE || state == WRITE_STATE) begin
            if(writeDelayCounter == 3)
                next_writeOutputCnt = writeOutputCnt + 1;
            else
                next_writeOutputCnt = writeOutputCnt;
        end
    end

endmodule

// parameter CONV3_STATE = 3'b011;
// parameter FC1W_STATE  = 3'b100;
// parameter FC2W_STATE  = 3'b101;
// parameter FC2B_STATE  = 3'b110;
module FC (
    input wire clk,
    input wire rst_n,  

    input wire fc1_start,
    output reg fc1_finish,

    input wire [31:0] scale_FC1,

    // Weight sram, dual port
    output reg [ 3:0] sram_weight_wea0,
    output reg [15:0] sram_weight_addr0,
    output reg [31:0] sram_weight_wdata0,
    input wire [31:0] sram_weight_rdata0,
    output reg [ 3:0] sram_weight_wea1,
    output reg [15:0] sram_weight_addr1,
    output reg [31:0] sram_weight_wdata1,
    input wire [31:0] sram_weight_rdata1,

    // Activation sram, dual port
    output reg [ 3:0] sram_act_wea0,
    output reg [15:0] sram_act_addr0,
    output reg [31:0] sram_act_wdata0,
    input wire [31:0] sram_act_rdata0,
    output reg [ 3:0] sram_act_wea1,
    output reg [15:0] sram_act_addr1,
    output reg [31:0] sram_act_wdata1,
    input wire [31:0] sram_act_rdata1,

    input wire [2:0] outter_state
);
    reg signed [31:0] partial_sum; 

    wire signed [15:0] fc1_approx_res;

    parameter IDLE_STATE  = 2'b00;
    parameter READ_STATE  = 2'b01;
    parameter WRITE_STATE = 2'b10;
    parameter DONE_STATE  = 2'b11;

    reg [ 1:0] state;
    reg [ 1:0] next_state;
    
    reg [ 5:0] input_count;
    reg [ 8:0] output_count;
    reg [ 2:0] ready_count;     // 準備好可以寫入的資料數
    reg [ 1:0] delay_counter;

    // sz
    reg [9:0] input_sram_offset;
    always @(*) begin
        input_sram_offset = 0;

        case(outter_state)
            3:
                input_sram_offset = 592;
            4:
                input_sram_offset = 692;
            5:
                input_sram_offset = 722;
        endcase
    end 

    reg [9:0] output_sram_offset;
    always @(*) begin
        output_sram_offset = 0;

        case(outter_state)
            3:
                output_sram_offset = 692;
            4:
                output_sram_offset = 722;
            5:
                output_sram_offset = 743;
        endcase
    end 

    reg [15:0] weight_sram_offset;
    always @(*) begin
        weight_sram_offset = 0;

        case(outter_state)
            3:
                weight_sram_offset = 1020;
            4:
                weight_sram_offset = 13020;
            5:
                weight_sram_offset = 15540;
        endcase
    end 

    reg [8:0] outputCnt_sz;
    always @(*) begin
        outputCnt_sz = 0;

        case(outter_state)
            3:
                outputCnt_sz = 120;
            4:
                outputCnt_sz = 88;
            5:
                outputCnt_sz = 10;  // TODO
        endcase
    end     

    reg [7:0] outputMap_sz;
    always @(*) begin
        outputMap_sz = 0;

        case(outter_state)
            3:
                outputMap_sz = 100;
            4:
                outputMap_sz = 30;
            5:
                outputMap_sz = 21;
        endcase
    end 

    reg [5:0] inputCount_sz;
    always @(*) begin
        inputCount_sz = 0;

        case(outter_state)
            3:
                inputCount_sz = 51;
            4:
                inputCount_sz = 16;
            5:
                inputCount_sz = 12;         // 可能要+1
        endcase
    end 

    reg [2:0] ready_sz;
    always @(*) begin
        if(outter_state == 5)
            ready_sz = 1;
        else
            ready_sz = 7;
    end

    // state transistion
    always @* begin
        case(state)
            IDLE_STATE: begin
                if(fc1_start)
                    next_state = READ_STATE;
                else
                    next_state = IDLE_STATE;
            end
            READ_STATE: begin 
                if(input_count == inputCount_sz + 2 && (ready_count == ready_sz || output_count >= outputCnt_sz))   // 原本數到14就好，多數3個cycle來確保所有IA都存入regsiter中
                    next_state = WRITE_STATE;
                else
                    next_state = READ_STATE;
            end
            WRITE_STATE: begin   
                if(output_count >= outputCnt_sz)
                    next_state = DONE_STATE;
                else
                    next_state = READ_STATE;
            end
            DONE_STATE: begin 
                if(delay_counter == 2)
                    next_state = IDLE_STATE;
                else
                    next_state = DONE_STATE;
            end
        endcase
    end

    // FSM output
    always @* begin
        case(state)
            READ_STATE: begin
                fc1_finish = 0;

                sram_act_addr0 = input_count*2 + input_sram_offset;
                sram_act_addr1 = input_count*2 + input_sram_offset + 1;

                if(outter_state == 5 && input_count >= 11) begin
                    sram_weight_addr0 = 15750 + output_count;
                    sram_weight_addr1 = 15750 + output_count;
                end
                else begin
                    sram_weight_addr0 = input_count*2 + output_count*outputMap_sz + weight_sram_offset;
                    sram_weight_addr1 = input_count*2 + output_count*outputMap_sz + weight_sram_offset + 1; 
                end
                
                sram_act_wea0 = 0;
                sram_act_wea1 = 0;  

                sram_weight_wea0 = 0;
                sram_weight_wea1 = 0;    

                sram_weight_wdata0 = 0;
                sram_weight_wdata1 = 0;
            end
            WRITE_STATE: begin
                fc1_finish = 0;

                if(outter_state != 5) begin
                    sram_act_addr0 = (((output_count >> 3) - 1) << 1) + output_sram_offset;
                    sram_act_addr1 = (((output_count >> 3) - 1) << 1) + output_sram_offset + 1;
                end
                else begin  //
                    sram_act_addr0 = output_sram_offset + output_count - 2;
                    sram_act_addr1 = output_sram_offset + output_count - 1;
                end 


                sram_weight_addr0 = 0; 
                sram_weight_addr1 = 0;

                sram_act_wea0 = 4'b1111;
                sram_act_wea1 = 4'b1111;

                sram_weight_wea0 = 0;
                sram_weight_wea1 = 0;

                sram_weight_wdata0 = 0;
                sram_weight_wdata1 = 0;                
            end
            DONE_STATE: begin
                if(delay_counter == 2)
                    fc1_finish = 1;
                else
                    fc1_finish = 0;

                sram_act_addr0 = 0;
                sram_act_addr1 = 0;

                sram_weight_addr0 = 0; 
                sram_weight_addr1 = 0;

                sram_act_wea0 = 0;
                sram_act_wea1 = 0;  

                sram_weight_wea0 = 0;
                sram_weight_wea1 = 0;

                sram_weight_wdata0 = 0;
                sram_weight_wdata1 = 0;                
            end
            default: begin
                fc1_finish = 0;

                sram_act_addr0 = 0;
                sram_act_addr1 = 0;

                sram_weight_addr0 = 0; 
                sram_weight_addr1 = 0;

                sram_act_wea0 = 0;
                sram_act_wea1 = 0; 

                sram_weight_wea0 = 0;
                sram_weight_wea1 = 0;

                sram_weight_wdata0 = 0;
                sram_weight_wdata1 = 0;                
            end
        endcase
    end
    
    assign fc1_approx_res = ((partial_sum * scale_FC1) >>> 16);
    wire signed [7:0] relu_res = (fc1_approx_res[15] == 1) ? 0 : fc1_approx_res[7:0];    // 負數

    // 處理partial sum
    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            partial_sum <= 0;
        else if(state == READ_STATE && input_count >= 3) begin
            if(outter_state == 5 && input_count == 13)
                partial_sum <= partial_sum  + $signed(sram_act_rdata0[ 7: 0]) * $signed(sram_weight_rdata0[ 7: 0])
                                            + $signed(sram_act_rdata0[15: 8]) * $signed(sram_weight_rdata0[15: 8])
                                            + $signed(sram_act_rdata0[23:16]) * $signed(sram_weight_rdata0[23:16])
                                            + $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[31:24]);           
            else
            partial_sum <= partial_sum + $signed(sram_act_rdata0[ 7: 0]) * $signed(sram_weight_rdata0[ 7: 0])
                                       + $signed(sram_act_rdata0[15: 8]) * $signed(sram_weight_rdata0[15: 8])
                                       + $signed(sram_act_rdata0[23:16]) * $signed(sram_weight_rdata0[23:16])
                                       + $signed(sram_act_rdata0[31:24]) * $signed(sram_weight_rdata0[31:24])
 
                                       + $signed(sram_act_rdata1[ 7: 0]) * $signed(sram_weight_rdata1[ 7: 0])
                                       + $signed(sram_act_rdata1[15: 8]) * $signed(sram_weight_rdata1[15: 8])
                                       + $signed(sram_act_rdata1[23:16]) * $signed(sram_weight_rdata1[23:16])
                                       + $signed(sram_act_rdata1[31:24]) * $signed(sram_weight_rdata1[31:24]);
        end
        else
            partial_sum <= 0;
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            state <= IDLE_STATE;
        else
            state <= next_state;
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            input_count <= 0;
        else if(input_count <= inputCount_sz+1 && state == READ_STATE)    // 最大數到17
            input_count <= input_count + 1;
        else
            input_count <= 0;
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            output_count <= 0;
        else if(input_count == inputCount_sz+2)
            output_count <= output_count + 1;
        else if(state == DONE_STATE)
            output_count <= 0;
        else
            output_count <= output_count;
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            ready_count <= 0;
        else if(input_count == inputCount_sz+2 && ready_count <= ready_sz && state == READ_STATE)
            ready_count <= ready_count + 1;
        else if(state == READ_STATE)
            ready_count <= ready_count;
        else
            ready_count <= 0;
    end

    reg [31:0] next_sram_act_wdata0;
    reg [31:0] next_sram_act_wdata1;

    always @(*) begin
        next_sram_act_wdata0 = sram_act_wdata0;
        next_sram_act_wdata1 = sram_act_wdata1;

        if(outter_state == 5) begin
            if(ready_count == 0) begin
                next_sram_act_wdata0 = (partial_sum ) + sram_weight_rdata0;
                next_sram_act_wdata1 = sram_act_wdata1;
            end
            else if(ready_count == 1) begin
                next_sram_act_wdata0 = sram_act_wdata0;
                next_sram_act_wdata1 = (partial_sum ) + sram_weight_rdata0;
            end
            else begin
                next_sram_act_wdata0 = 0;
                next_sram_act_wdata1 = 0;
            end
        end
        else if(input_count == inputCount_sz+2) begin
            if(ready_count <= 3'd3) begin
                next_sram_act_wdata0[(ready_count*8)+:8] = relu_res;
                next_sram_act_wdata1 = sram_act_wdata1;
            end
            else begin
                next_sram_act_wdata0 = sram_act_wdata0;
                next_sram_act_wdata1[((ready_count-4)*8)+:8] = relu_res;            
            end
        end
        else if(state == READ_STATE) begin
            next_sram_act_wdata0 = sram_act_wdata0;
            next_sram_act_wdata1 = sram_act_wdata1;
        end
        else begin
            next_sram_act_wdata0 = 0;
            next_sram_act_wdata1 = 0;                
        end
    end

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            sram_act_wdata0 <= 0;
            sram_act_wdata1 <= 0;
        end
        else begin
            sram_act_wdata0 <= next_sram_act_wdata0;
            sram_act_wdata1 <= next_sram_act_wdata1;
        end
    end 

    always @(posedge clk or negedge rst_n) begin
        if(!rst_n)
            delay_counter <= 0;
        else if(state == DONE_STATE && delay_counter <= 2)
            delay_counter <= delay_counter + 1;
        else
            delay_counter <= 0;
    end
endmodule