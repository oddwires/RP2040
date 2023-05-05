#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include <math.h>
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "blink.pio.h"
#include "DAC.pio.h"

/////////////////////////////
// Define GPIO connections...
/////////////////////////////

// Note: The SPI Port only works through specific pins, so this port is defined first.
// SPI Port connections...                          // ┌──────────┬───────────────┬─────────────┐──────────────┐
                                                    // │ PGA2040  │ Connection    │ MCP41010    │ Nixie module │
                                                    // ├──────────┼───────────────┼─────────────┤──────────────┤
#define PIN_RX          16                          // │ GPIO 16  │ RX/spi1_rx    │             │      -       │
//#define PIN_CS          17                        // │ GPIO 17  │ CS/spi1_cs    │             │              │ can this be re-defined ?
#define PIN_CLK         18                          // │ GPIO 18  │ CLK/spi1_clk  │             │  SCK (blue)  │
#define PIN_TX          19                          // │ GPIO 19  │ TX/spi1_tx    │             │  SDI (green) │
#define Nixie_CS        21                          // │ GPIO 21  │ Chip select   │             │  SS1 (white) │
                                                    // └──────────┴───────────────┴─────────────┘──────────────┘
#define SPI_PORT        spi0                        // These SPI connections require the use of RP2040 SPI port 0

#define _A               0                           // DAC channel alias
#define _B               1
#define _Up              1
#define _Down           -1
#define LED             20                          // GPIO connected to LED
#define BitMapSize     256                          // Match X to Y resolution
#define _Sine_           0                          // Permited values for variable WaveForm_Type
#define _Square_         1
#define _Triangle_       2
#define _GPIO_           0
#define _PIO_            1
#define _BM_start_       2
#define _SM_             3
#define _SM_codeBot_     4
#define _SM_codeTop_     5
#define _DMA_ctrl_       6
#define _DMA_data_       7
#define _Funct_          8
#define _Phase_          9
#define _Freq_          10
#define _Range_         11
//#define _DutyC_         12
#define _DAC_div_       13
#define eof            255                          // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking
//#define BitMapSize      360                       // won't work - DMA needs to operate as a power of 2

unsigned short DAC_channel_mask = 0 ;               // Binary mask to simultaneously start all DMA channels
const uint32_t transfer_count = BitMapSize ;        // Number of DMA transfers per event
int WaveForm_Type;
const uint startLineLength = 8;                     // the linebuffer will automatically grow for longer lines
int ParmCnt = 0, Parm[4] ;                          // Storage for 4 command line parameters
int SelectedChan, c, i = 0, dirn = 1 ;
char LastCmd[30];                                   // TBD - check required size
const char * HelpText = 
"\tUsage...\n"
"\t  ?            - Usage\n"
"\t  S            - Status\n"
"\t  I            - System info\n"
"\t  <A/B/C>f+    - Frequency + 1\n"
"\t  <A/B/C>f-    - Frequency - 1\n"
"\t  <A/B/C>fnnn  - Frequency                  ( 0->999 )\n"
"\t  <A/B/C>p+    - Phase + 1\n"
"\t  <A/B/C>p-    - Phase - 1\n"
"\t  <A/B/C>pnnn  - Phase                      ( 0->360 degrees )\n"
"\t  <A/B/C>h     - Frequency multiplier  Hz\n"
"\t  <A/B/C>k     - Frequency multiplier KHz\n"
"\t  <A/B/C>snnn  - Sine wave + harmonic       ( 0->9 )\n"
"\t  <A/B/C>q+    - Duty Cycle + 1\n"
"\t  <A/B/C>q-    - Duty Cycle - 1\n"
"\t  <A/B/C>qnnn  - Square wave + duty cycle   ( 0->100%% )\n"
"\t  <A/B/C>t+    - Rise time + 1\n"
"\t  <A/B/C>t-    - Rise time - 1\n"
"\t  <A/B/C>tnnn  - Triangle wave + Rise time  ( 0->100%% )\n"
"\t  <A/B/C>w     - Sweep frequency\n"
"\t  <A/B/C>      - DAC channel A,B or Both\n"
"\t        nnn    - Three digit numeric value\n";

class DACchannel {
    unsigned short DAC_data[BitMapSize] __attribute__ ((aligned(2048))) ;   // Align DAC data (2048d = 0800h)
    int Funct, Freq, Range, Phase, DutyC ;
    uint StateMachine, ctrl_chan, data_chan, GPIO, SM_WrapBot, SM_WrapTop ; // Variabes used by the getter function...
    float DAC_div ;
    PIO pio;                                                                // Class wide var to share value with setter function

public:
// Setter functions...
    void ReInit () {
    // Re-initialises DMA channels to their initial state.
    // Note: 1) DMA channels are not restarted, allowing an atomic (simultaneous) restart of both DAC channels later.
    //       2) Cannot use dma_hw->abort on chained DMA channels, so using disable and re-enable instead.
    //       3) This needs to be performed across both DAC channels to ensure phase sync is maintained.
    // Disable both DMA channels associated with this DAC...
        hw_clear_bits(&dma_hw->ch[data_chan].al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
        hw_clear_bits(&dma_hw->ch[ctrl_chan].al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);        
    // Reset the data transfer DMA's to the start of the data Bitmap...
        dma_channel_set_read_addr(data_chan, &DAC_data[0], false);
    // Re-enable both DMA channels associated with this DAC...
        hw_set_bits(&dma_hw->ch[data_chan].al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
        hw_set_bits(&dma_hw->ch[ctrl_chan].al1_ctrl, DMA_CH0_CTRL_TRIG_EN_BITS);
    }
    void SetFunct (int _value)  { Funct = _value ; }                        // Function    (Sine/Triangl/Square)
    void SetDutyC (int _value)  { DutyC = _value ; }                        // Duty cycle  (0->100%)
    void SetRange (int _value)  { Range = _value ;                          // Range       (Hz/KHz)
                                  DACspeed(Freq * Range) ; }               // Update State Machine run speed
    void SetFreq  (int _value)  { Freq  = _value ;                          // Frequency   (numeric)
                                  DACspeed(Freq * Range) ; }                // Update State machine run speed    
    void SetPhase (int _value)  { Phase = _value ;                          // Phase shift (0->360 degrees)
                                  DataCalc() ; }                            // Recalc Bitmap using new phase value
    int BumpFreq (int _value)   { Freq += _value ;
                                  if (Freq >= 1000) { Freq = 0 ;   }        // Endwrap
                                  if (Freq < 0)     { Freq = 999 ; }        // Endwrap
                                  DACspeed(Freq * Range) ; 
                                  return (Freq) ; }
    int BumpPhase (int _value)  { Phase += _value ;
                                  if (Phase == 360) { Phase = 0 ;   }       // Endwrap
                                  if (Phase  < 0  ) { Phase = 360 ; }       // Endwrap
                                  DataCalc();                               // Update Bitmap data to include new DAC phase
                                  return (Phase) ; }
    int BumpDuty (int _value)   { DutyC += _value ;
                                  if (DutyC == 100) { DutyC = 0 ;   }       // Endwrap
                                  if (DutyC < 0   ) { DutyC = 100 ; }       // Endwrap
                                  DataCalc(); 
                                  return (DutyC) ;  }                       // Update Bitmap with new Duty Cycle value

    void DACspeed (int _frequency) {
        // If DAC_div exceeds 2^16 (65,536), the registers wrap around, and the State Machine clock will be incorrect.
        // A slow version of the DAC State Machine is used for frequencies below 17Hz, allowing the value of DAC_div to
        // be kept within range.
        float DAC_freq = _frequency * BitMapSize;                               // Target frequency...
        DAC_div = 2 * (float)clock_get_hz(clk_sys) / DAC_freq;                  // ...calculate the PIO clock divider required for the given Target frequency
        float Fout = 2 * (float)clock_get_hz(clk_sys) / (BitMapSize * DAC_div); // Actual output frequency
         if (_frequency >= 34) {                                                // Fast DAC ( Frequency range from 34Hz to 999Khz )
            SM_WrapTop = SM_WrapBot ;                                           // SM program memory = 1 op-code
            pio_sm_set_wrap (pio, StateMachine, SM_WrapBot, SM_WrapTop) ;       // Fast loop (1 clock cycle)
            // If the previous frequency was < 33Hz, we will have just shrunk the assembler from 4 op-codes down to 1.
            // This leaves the State Machine PC pointing outside of the new WRAP statement, which crashes the SM.
            // To avoid this, we need to also reset the State Machine program counter...
            pio->sm[StateMachine].instr = SM_WrapBot ;                          // Reset State Machine PC to start of code
            pio_sm_set_clkdiv(pio, StateMachine, DAC_div);                      // Set the State Machine clock 
        } else {                                                                // Slow DAC ( 1Hz=>33Hz )
            DAC_div = DAC_div / 64;                                             // Adjust DAC_div to keep within useable range
            DAC_freq = DAC_freq * 64;
            SM_WrapTop = SM_WrapBot + 3 ;                                       // SM program memory = 4 op-codes
            pio_sm_set_wrap (pio, StateMachine, SM_WrapBot, SM_WrapTop) ;       // slow loop (64 clock cycles)
            // If the previous frequency was >= 34Hz, we will have just expanded the assembler code from 1 op-code up to 4.
            // The State Machine PC will still be pointing to an op-code within the new WRAP statement, so will not crash.
            pio_sm_set_clkdiv(pio, StateMachine, DAC_div);                      // Set the State Machine clock speed
        }
    }

    void DataCalc () {
    //    int i,h_index, v_offset = BitMapSize/2 - 1;                                                 // Shift sine waves up above X axis
        int i,j, v_offset = 256/2 - 1;                                                                // Shift sine waves up above X axis
        int _phase;
        const float _2Pi = 6.283;                                                                     // 2*Pi
        float a,b,x1,x2,g1,g2;

    // Scale the phase shift to match data size...    
        _phase = Phase * BitMapSize / 360 ;                                                            // Input  range: 0 -> 360 (degrees)
                                                                                                      // Output range: 0 -> 255 (bytes)
        switch (Funct) {
            case _Sine_:
                DutyC = DutyC % 10;                                                                   // Sine value cycles after 7
                for (i=0; i<BitMapSize; i++) {
    // Add the phase offset and wrap data beyond buffer end back to the buffer start...
                    j = ( i + _phase ) % BitMapSize;                                                  // Horizontal index
                    a = v_offset * sin((float)_2Pi*i / (float)BitMapSize);                            // Fundamental frequency...
                    if (DutyC >= 1) { a += v_offset/3  * sin((float)_2Pi*3*i  / (float)BitMapSize); } // Add  3rd harmonic
                    if (DutyC >= 2) { a += v_offset/5  * sin((float)_2Pi*5*i  / (float)BitMapSize); } // Add  5th harmonic
                    if (DutyC >= 3) { a += v_offset/7  * sin((float)_2Pi*7*i  / (float)BitMapSize); } // Add  7th harmonic
                    if (DutyC >= 4) { a += v_offset/9  * sin((float)_2Pi*9*i  / (float)BitMapSize); } // Add  9th harmonic
                    if (DutyC >= 5) { a += v_offset/11 * sin((float)_2Pi*11*i / (float)BitMapSize); } // Add 11th harmonic
                    if (DutyC >= 6) { a += v_offset/13 * sin((float)_2Pi*13*i / (float)BitMapSize); } // Add 13th harmonic
                    if (DutyC >= 7) { a += v_offset/15 * sin((float)_2Pi*15*i / (float)BitMapSize); } // Add 15th harmonic
                    if (DutyC >= 8) { a += v_offset/17 * sin((float)_2Pi*17*i / (float)BitMapSize); } // Add 17th harmonic
                    if (DutyC >= 9) { a += v_offset/19 * sin((float)_2Pi*19*i / (float)BitMapSize); } // Add 19th harmonic
                    DAC_data[j] = (int)(a)+v_offset;                                                  // Sum all harmonics and add vertical offset
                }
                break;
            case _Square_: 
                b = DutyC * BitMapSize / 100;                                                         // Convert % to value
                for (i=0; i<BitMapSize; i++) {
                    j = ( i + _phase ) % BitMapSize;                                                  // Horizontal index
                    if (b <= i) { DAC_data[j] = 0;   }                                                // First section low
                    else        { DAC_data[j] = 255; }                                                // Second section high
                }
                break;
            case _Triangle_: 
                x1 = (DutyC * BitMapSize / 100) -1;                                                   // Number of data points to peak
                x2 = BitMapSize - x1;                                                                 // Number of data points after peak
                g1 = (BitMapSize - 1) / x1;                                                           // Rising gradient (Max val = BitMapSize -1)
                g2 = (BitMapSize - 1) / x2;                                                           // Falling gradient (Max val = BitMapSize -1)
                for (i=0; i<BitMapSize; i++) {
                    j = ( i + _phase ) % BitMapSize;                                                  // Horizontal index
                    if (i <= x1) { DAC_data[j] = i * g1; }                                            // Rising  section of waveform...
                    if (i > x1)  { DAC_data[j] = (BitMapSize - 1) - ((i - x1) * g2); }                // Falling section of waveform
                }
        }
    }
    // Getter functions...
    int Get_Resource (int _index) {
        int result;
        switch (_index) {
            case _GPIO_:          result = GPIO;                break;
            case _PIO_:           result = pio_get_index(pio);  break;
            case _BM_start_:      result = (int)&DAC_data[0];   break;
            case _SM_:            result = StateMachine;        break;
            case _SM_codeBot_:    result = SM_WrapBot;          break;
            case _SM_codeTop_:    result = SM_WrapTop;          break;
            case _DMA_ctrl_:      result = ctrl_chan;           break;
            case _DMA_data_:      result = data_chan;           break;
            case _Funct_:         result = Funct;               break;
            case _Phase_:         result = Phase;               break;
            case _Freq_:          result = Freq;                break;
            case _Range_:         result = Range;               break;
            case _DAC_div_:       result = DAC_div;             break;
        }
    return (result);
    }

public:
    // Each DAC channel consists of...
    //    DMA => FIFO => State Machine => GPIO pins => R2R module
    // Note: The PIO clock dividers are 16-bit integer, 8-bit fractional, with first-order delta-sigma for the fractional divider.
    //       This means the clock divisor can vary between 1 and 65536, in increments of 1/256.
    //       If DAC_div exceeds 2^16 (65,536), the registers will wrap around, and the State Machine clock will be incorrect.
    //       For frequencies below 34Hz, an additional 63 op-code delay is inserted into the State Machine assembler code. This slows
    //       down the State Machine operation by a factor of 64, keeping the value of DAC_div within range.
    // Parameters...
    //       _pio = the required PIO channel
    //       _GPIO = the port connecting to the MSB of the R-2-R resistor network.
    // Constructor
    int DAC_chan(PIO _pio, uint _GPIO) {
        pio = _pio, GPIO = _GPIO;                                               // copy parameters to class vars
        int _offset;
        StateMachine = pio_claim_unused_sm(_pio, true);                         // Find a free state machine on the specified PIO - error if there are none.
        ctrl_chan = dma_claim_unused_channel(true);                             // Find 2 x free DMA channels for the DAC (12 available)
        data_chan = dma_claim_unused_channel(true);

        // Configure the state machine to run the DAC program...
            _offset = pio_add_program(_pio, &pio_DAC_program);                  // Use helper function included in the .pio file.
            SM_WrapBot = _offset;
            pio_DAC_program_init(_pio, StateMachine, _offset, _GPIO);

        //  Setup the DAC control channel...
        //  The control channel transfers two words into the data channel's control registers, then halts. The write address wraps on a two-word
        //  (eight-byte) boundary, so that the control channel writes the same two registers when it is next triggered.
        dma_channel_config fc = dma_channel_get_default_config(ctrl_chan);      // default configs
        channel_config_set_transfer_data_size(&fc, DMA_SIZE_32);                // 32-bit txfers
        channel_config_set_read_increment(&fc, false);                          // no read incrementing
        channel_config_set_write_increment(&fc, false);                         // no write incrementing
        dma_channel_configure(
            ctrl_chan,
            &fc,
            &dma_hw->ch[data_chan].al1_transfer_count_trig,                     // txfer to transfer count trigger
            &transfer_count,
            1,
            false
        );
        //  Setup the DAC data channel...
        //  32 bit transfers. Read address increments after each transfer.
        fc = dma_channel_get_default_config(data_chan);
        channel_config_set_transfer_data_size(&fc, DMA_SIZE_32);                // 32-bit txfers
        channel_config_set_read_increment(&fc, true);                           // increment the read adddress
        channel_config_set_write_increment(&fc, false);                         // don't increment write address
        channel_config_set_dreq(&fc, pio_get_dreq(_pio, StateMachine, true));   // Transfer when PIO SM TX FIFO has space
        channel_config_set_chain_to(&fc, ctrl_chan);                            // chain to the controller DMA channel
        channel_config_set_ring(&fc, false, 9);                                 // 8 bit DAC 1<<9 byte boundary on read ptr. This is why we needed alignment!
        dma_channel_configure(
            data_chan,                                                          // Channel to be configured
            &fc,                                                                // The configuration we just created
            &_pio->txf[StateMachine],                                           // Write to FIFO
            DAC_data,                                                           // The initial read address (AT NATURAL ALIGNMENT POINT)
            BitMapSize,                                                         // Number of transfers; in this case each is 2 byte.
            false                                                               // Don't start immediately. All 4 control channels need to start simultaneously
                                                                                // to ensure the correct phase shift is applied.
        );
        DAC_channel_mask += (1u << ctrl_chan) ;                                 // Save details of DMA control channel to global variable. This facilitates
                                                                                // atomic restarts of both channels, and ensures phase lock between channels.

        return(StateMachine);
    }
};

class blink_forever {                                                      // Class to initialise a state machine to blink a GPIO pin
PIO pio ;                                                                   // Class wide variables to share value with setter function
uint pioNum, StateMachine, Freq, _offset ;
public:
    blink_forever(PIO _pio) {
        pio = _pio;                                                        // transfer parameter to class wide var
        pioNum = pio_get_index(_pio);
        StateMachine = pio_claim_unused_sm(_pio, true);                    // Find a free state machine on the specified PIO - error if there are none.
        _offset = pio_add_program(_pio, &pio_blink_program);
        blink_program_init(_pio, StateMachine, _offset, LED );
        pio_sm_set_enabled(_pio, StateMachine, true);
    }

    // Setter function...
    void Set_Frequency(int _frequency){
        Freq = _frequency;                                                  // Copy parm to class var
        // Frequency scaled by 2000 as blink.pio requires this number of cycles to complete...
        float DAC_div = (float)clock_get_hz(clk_sys) /((float)_frequency*2000);
        pio_sm_set_clkdiv(pio, StateMachine, DAC_div);                      // Set the State Machine clock speed
    }

    // Getter function...
    int Get_Resource (int _index) {
        int result;
        switch (_index) {
            case _GPIO_:          result = LED;               break;
            case _SM_:            result = StateMachine;      break;
            case _PIO_:           result = pioNum;            break;
            case _Freq_:          result = Freq;              break;
        }
    return (result);
    }
};

void ChanInfo ( DACchannel DACchannel[], int _chanNum) {
// Print current channel parameters to the console...
    char Chan, WaveStr[9], MultStr[4];
    int value = DACchannel[_chanNum].Get_Resource(_Funct_);

    int test = DACchannel[_chanNum].Get_Resource(_Phase_);

    switch ( value ) {
        case _Sine_:     strcpy(WaveStr, "Sine");     break;
        case _Triangle_: strcpy(WaveStr, "Triangle"); break;
        case _Square_:   strcpy(WaveStr,"Square");
    }
    _chanNum == 0 ? Chan = 'A' : Chan = 'B';
    DACchannel[_chanNum].Get_Resource(_Range_) == 1 ? strcpy(MultStr,"Hz ") : strcpy(MultStr,"KHz");
    printf("\tChannel %c: Freq:%03d%s Phase:%03d  Wave:%s\n", Chan, DACchannel[_chanNum].Get_Resource(_Freq_),
        MultStr, DACchannel[_chanNum].Get_Resource(_Phase_), WaveStr);
}

void SysInfo ( DACchannel DACchannel[], blink_forever LED_blinky) {
    // Print system and resource allocation details...
    int a,b,c,d ;
    a = LED_blinky.Get_Resource(_PIO_);
    b = LED_blinky.Get_Resource(_SM_);
    c = LED_blinky.Get_Resource(_GPIO_);
    d = LED_blinky.Get_Resource(_Freq_);
    printf("\n|-----------------------------------------------------------|\n");
    printf("| Waveform Generator Ver: 0.0.1       Date: 21/03/2013      |\n");
    printf("|-----------------------------|-----------------------------|\n");
    printf("| LED blinker                 |                             |\n");
    printf("|-----------------------------|                             |\n");
    printf("|   PIO:          %2d          |  Key:                       |\n",a);
    printf("|   SM:           %2d          |   SM = State machine        |\n",b);
    printf("|   GPIO:         %2d          |   BM = Bitmap               |\n",c);
    printf("|   Frequency:    %2dHz        |                             |\n",d);
    printf("|-----------------------------|-----------------------------|\n");
    printf("| DAC channel A               | DAC channel B               |\n");
    a = DACchannel[_A].Get_Resource(_Freq_), b = DACchannel[_B].Get_Resource(_Freq_);
    printf("| Frequency:     %3d          | Frequency:     %3d          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_DAC_div_), b = DACchannel[_B].Get_Resource(_DAC_div_);
    printf("| Divider:     %05x          | Divider:     %05x          |\n",a,b);
    printf("|-----------------------------|-----------------------------|\n");
    a = DACchannel[_A].Get_Resource(_PIO_), b = DACchannel[_B].Get_Resource(_PIO_);
    printf("| PIO:             %d          | PIO:             %d          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_GPIO_), b = DACchannel[_B].Get_Resource(_GPIO_);
    printf("| GPIO:          %d-%d          | GPIO:         %d-%d          |\n",a,a+7,b,b+7);
    printf("| BM size:  %8d          | BM size:  %8d          |\n", BitMapSize,  BitMapSize);
    a = DACchannel[_A].Get_Resource(_BM_start_), b = DACchannel[_B].Get_Resource(_BM_start_);
    printf("| BM start: %x          | BM start: %x          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_SM_), b = DACchannel[_B].Get_Resource(_SM_);
    printf("| SM:              %d          | SM:              %d          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_SM_codeBot_), b = DACchannel[_B].Get_Resource(_SM_codeBot_);
    printf("| Wrap Bottom:    %2x          | Wrap Bottom:    %2x          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_SM_codeTop_), b = DACchannel[_B].Get_Resource(_SM_codeTop_);
    printf("| Wrap Top:       %2x          | Wrap Top:       %2x          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_DMA_ctrl_), b = DACchannel[_B].Get_Resource(_DMA_ctrl_);
    printf("| DMA ctrl:       %2d          | DMA ctrl:       %2d          |\n",a,b);
    a = DACchannel[_A].Get_Resource(_DMA_data_), b = DACchannel[_B].Get_Resource(_DMA_data_);
    printf("| DMA data:       %2d          | DMA data:       %2d          |\n",a,b);
    printf("|--------------|--------------|--------------|--------------|\n");
}

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(Nixie_CS, 0);                                                      // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(Nixie_CS, 1);
    asm volatile("nop \n nop \n nop");
}

static void SPI_Nixie_Write(int _data) {
    uint8_t buff[2];
    buff[0] = _data / 256;                                                      // MSB data
    buff[1] = _data % 256;                                                      // LSB data
    cs_select();
    spi_write_blocking(SPI_PORT, buff, 2);
    cs_deselect();
}

static char * getLine(bool fullDuplex = false, char lineBreak = '\n') {
/*
 *  read a line of any  length from stdio (grows)
 *
 *  @param fullDuplex input will echo on entry (terminal mode) when false
 *  @param linebreak defaults to "\n", but "\r" may be needed for terminals
 *  @return entered line on heap - don't forget calling free() to get memory back
 */
    // th line buffer
    // will allocated by pico_malloc module if <cstdlib> gets included
    char * pStart = (char*)malloc(startLineLength); 
    char * pPos = pStart;  // next character position
    size_t maxLen = startLineLength; // current max buffer size
    size_t len = maxLen; // current max length
    int c;

    if(!pStart) {
        return NULL; // out of memory or dysfunctional heap
    }

    while(1) {
        c = getchar(); // expect next character entry
        if(c == eof || c == lineBreak) {
            break;     // non blocking exit
        }
         if (fullDuplex) {
            putchar(c); // echo for fullDuplex terminals
        }
        if(--len == 0) { // allow larger buffer
            len = maxLen;
            // double the current line buffer size
            char *pNew  = (char*)realloc(pStart, maxLen *= 2);
            if(!pNew) {
                free(pStart);
                return NULL; // out of memory abort
            }
            // fix pointer for new buffer
            pPos = pNew + (pPos - pStart);
            pStart = pNew;
        }
        // stop reading if lineBreak character entered 
        if((*pPos++ = c) == lineBreak) {
            break;
        }
    }
    *pPos = '\0';   // set string end mark
    return pStart;
}

int main() {
    stdio_init_all();

// Set SPI0 at 0.5MHz.
    spi_init(SPI_PORT, 500 * 1000);
    gpio_set_function(PIN_CLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_TX, GPIO_FUNC_SPI);

// Chip select is active-low, so initialise to a driven-high state...
    gpio_init(Nixie_CS);
    gpio_set_dir(Nixie_CS, GPIO_OUT);
    gpio_put(Nixie_CS, 1);

// Initialise remaining SPI connections...
    gpio_set_dir(PIN_CLK, GPIO_OUT);
    gpio_set_dir(PIN_TX, GPIO_OUT);

    DACchannel DACchannel[2];                                                // Array to hold the two DAC channel objects

// Set up the objects controlling the various State Machines...
// Note: Both DAC channels need to be on the same PIO to acheive accurate phase sync.
    DACchannel[_A].DAC_chan(pio1,0);                              // First  DAC channel object in array - resistor network connected to GPIO0->7
    DACchannel[_B].DAC_chan(pio1,8);                              // Second DAC channel object in array - resistor network connected to GPIO8->15
    blink_forever LED_blinky(pio0);                                          // Onboard LED blinky object

// Set default run time settings...
    DACchannel[_A].SetRange(1),         DACchannel[_B].SetRange(1) ;         // Hz
    DACchannel[_A].SetFunct(_Sine_),    DACchannel[_B].SetFunct(_Sine_) ;    // Sine wave, no harmonics
    DACchannel[_A].SetDutyC(50),        DACchannel[_B].SetDutyC(50) ;        // 50% Duty cycle
    DACchannel[_A].SetFreq(100),        DACchannel[_B].SetFreq(100) ;        // 100
    DACchannel[_A].SetPhase(0),         DACchannel[_B].SetPhase(180) ;       // 180 phase diff + generate the two Bitmaps
    strcpy(LastCmd,"?") ;                                                    // Hitting return will give 'Help'

    SPI_Nixie_Write(DACchannel[_A].Get_Resource(_Freq_));                    // Frequency => Nixie display

// Set LED to slow flash indicates waiting for USB connection...
    LED_blinky.Set_Frequency(1);                                             // 1Hz

// Wait for USB connection...
    while (!stdio_usb_connected()) { sleep_ms(100); }

// USB connection established, set LED to rapid flash...
    LED_blinky.Set_Frequency(10);                                            // 10Hz

    SysInfo(DACchannel, LED_blinky);                                         // Show configuration (optional)
//  printf(HelpText);                                                        // Show instructions  (optional)

// Starting all 4 DMA channels simultaneously ensures phase sync across all State Machines...
    dma_start_channel_mask(DAC_channel_mask);

    while(1) {
        ParmCnt=0, Parm[0]=0,  Parm[1]=0,  Parm[2]=0,  Parm[3]=0; 
        printf(">") ;                                                                       // Command prompt

        char *inString = getLine(true, '\r') ;

        // Zero length string = 'CR' pressed...
        if (strlen(inString) == 0) { strcpy(inString,LastCmd) ;                             // Repeat last command    
                                     printf("%s", inString) ; }

        // Check for single character instructions...
        if (inString[0] == '?') { printf(HelpText);         }                               // Help text
        if (inString[0] == 'S') { ChanInfo(DACchannel, _A);                                 // Status info
                                  ChanInfo(DACchannel, _B); }
        if (inString[0] == 'I') { SysInfo(DACchannel, LED_blinky); }

        // Select DAC channel A or B...
        if (inString[0] == 'A') { SelectedChan = 0b0001; }                                  // Channel A
        if (inString[0] == 'B') { SelectedChan = 0b0010; }                                  // Channel B
        if (inString[0] == 'C') { SelectedChan = 0b0011; }                                  // Channel A & B

        if ((inString[2] != '+') && (inString[2] != '-')) {
            // Not bumping a value, so extract the value of Parm[0]...
            i = 1 ;                                                                         // Skip chars 0 & 1
            while (i++ < strlen(inString)-1 ) {                                             // Start at char 2
                if ( inString[i] == ',' ) { ParmCnt++ ; }                                   // Next parameter
                else                      { Parm[ParmCnt] *= 10;                            // Next digit. Bump the existing decimal digits
                                            Parm[ParmCnt] += inString[i] - '0'; }           // Convert character to integer and add
            }
        }
        // Perform the selected command...
        switch ( inString[1] ) {
            case 'w':                                                                       // Frequency sweep
                i = Parm[0];
                for (;;) {
                    DACchannel[_A].ReInit();                                                // Stop DAC channel A and re-initialise DMA to start of Bitmap data
                    DACchannel[_B].ReInit();                                                // Stop DAC channel B and re-initialise DMA to start of Bitmap data
                    if (SelectedChan & 0b01) {
                        DACchannel[_A].SetFreq(i); 
                        ChanInfo(DACchannel, _A);                                           // Update the terminal
                    }
                    if (SelectedChan & 0b10) {
                        DACchannel[_B].SetFreq(i);
                        ChanInfo(DACchannel, _B);                                           // Update the terminal
                        }
                    dma_start_channel_mask(DAC_channel_mask);                               // Atomically Restart all 4 DMA channels...
                    SPI_Nixie_Write(i);                                                     // Update Nixie display
                    if (i==Parm[0]) { dirn = 1;
                                      sleep_ms(Parm[3]); }                                  // Count up from zero, pause at end
                    if (i>=Parm[1]) { dirn =-1; 
                                      sleep_ms(Parm[3]); }                                  // Count down from 100, pause at start
                    i = i + dirn;
                    c = getchar_timeout_us (0);                                             // Non-blocking char input
                    if ((c>=32) & (c<=126)) { break; }                                      // exit on keypress
                    sleep_ms(Parm[2]);                                                      // Speed of scan
                }
                break;
            case 's':                                                                       // Sine wave
                if (SelectedChan & 0b01) {
                    DACchannel[_A].SetFunct(_Sine_);
                    DACchannel[_A].SetDutyC(Parm[0]);
                    DACchannel[_A].DataCalc();
                }
                if (SelectedChan & 0b10) {
                    DACchannel[_B].SetFunct(_Sine_);
                    DACchannel[_B].SetDutyC(Parm[0]);
                    DACchannel[_B].DataCalc();
                }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 't':                                                                       // Triangle wave
                if (inString[2] == '+') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpDuty(_Up); }    // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpDuty(_Up); }    // Bump + grab new value for SPI
                }
                else if (inString[2] == '-') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpDuty(_Down); }  // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpDuty(_Down); }  // Bump + grab new value for SPI
                }
                else {
                    // Not bumping the value, so set the absolute value from Parm[0]...
                    if ( Parm[0] > 100 ) { Parm[0] = 100; }                                 // Hard limit @ 100%
                    if (SelectedChan & 0b01) {
                        DACchannel[_A].SetFunct(_Triangle_);
                        DACchannel[_A].SetDutyC(Parm[0]);
                        DACchannel[_A].DataCalc();
                    }
                    if (SelectedChan & 0b10) {
                        DACchannel[_B].SetFunct(_Triangle_);
                        DACchannel[_B].SetDutyC(Parm[0]);
                        DACchannel[_B].DataCalc();
                    }
                }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 'q':                                                                       // sQuare wave
                if (inString[2] == '+') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpDuty(_Up); }    // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpDuty(_Up); }    // Bump + grab new value for SPI
                }
                else if (inString[2] == '-') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpDuty(_Down); }  // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpDuty(_Down); }  // Bump + grab new value for SPI
                }
                else {
                    // Not bumping the value, so set the absolute value from Parm[0]...
                    if ( Parm[0] > 100 ) { Parm[0] = 100; }                                 // Hard limit @ 100%
                    if (SelectedChan & 0b01) {
                        DACchannel[_A].SetFunct(_Square_);
                        DACchannel[_A].SetDutyC(Parm[0]);
                        DACchannel[_A].DataCalc();
                    }
                    if (SelectedChan & 0b10) {
                        DACchannel[_B].SetFunct(_Square_);
                        DACchannel[_B].SetDutyC(Parm[0]);
                        DACchannel[_B].DataCalc();
                    }
                }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 'h':                                                                       // Set Hz
                if (SelectedChan & 0b01) { DACchannel[_A].SetRange(1); }
                if (SelectedChan & 0b10) { DACchannel[_B].SetRange(1); }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 'k':                                                                       // Set KHz
                if (SelectedChan & 0b01) { DACchannel[_A].SetRange(1000); }
                if (SelectedChan & 0b10) { DACchannel[_B].SetRange(1000); }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 'f':                                                                       // Frequency setting...
                if (inString[2] == '+') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpFreq(_Up); }    // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpFreq(_Up); }    // Bump + grab new value for SPI
                }
                else if (inString[2] == '-') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpFreq(_Down); }  // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpFreq(_Down); }  // Bump + grab new value for SPI
                }
                else {
                // Not bumping the value, so set the absolute value from Parm[0]...
                    DACchannel[_A].ReInit();                                                // Stop DAC channel A and re-initialise DMA to start of Bitmap data
                    DACchannel[_B].ReInit();                                                // Stop DAC channel B and re-initialise DMA to start of Bitmap data
                    if (SelectedChan & 0b01) { DACchannel[_A].SetFreq(Parm[0]); }           // Update State Machine clock speed
                    if (SelectedChan & 0b10) { DACchannel[_B].SetFreq(Parm[0]); }           // Update State Machine clock speed
                    dma_start_channel_mask(DAC_channel_mask);                               // Atomic restart all 4 DMA channels
                }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            case 'p':                                                                       // Phase settings...
                if (inString[2] == '+') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpPhase(_Up); }   // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpPhase(_Up); }   // Bump + grab new value for SPI
                }
                else if (inString[2] == '-') { 
                    if (SelectedChan & 0b01) { Parm[0] = DACchannel[_A].BumpPhase(_Down); } // Bump + grab new value for SPI
                    if (SelectedChan & 0b10) { Parm[0] = DACchannel[_B].BumpPhase(_Down); } // Bump + grab new value for SPI
                }
                else {
                // Not bumping the value, so set the absolute value from Parm[0]...
                    DACchannel[_A].ReInit();                                                // Stop DAC channel A and re-initialise DMA to start of Bitmap data
                    DACchannel[_B].ReInit();                                                // Stop DAC channel B and re-initialise DMA to start of Bitmap data
                    if (SelectedChan & 0b01) { DACchannel[_A].SetPhase(Parm[0]); }          // Update DAC phase
                    if (SelectedChan & 0b10) { DACchannel[_B].SetPhase(Parm[0]); }          // Update DAC phase.
                    dma_start_channel_mask(DAC_channel_mask);                               // Atomic restart all 4 DMA channels
                }
                if (SelectedChan & 0b01) { ChanInfo(DACchannel, _A); }                      // Update the terminal
                if (SelectedChan & 0b10) { ChanInfo(DACchannel, _B); }
                break;
            default:
                if ((inString[0] != 'S') && (inString[0] != 'I') && (inString[0] != '?')) {
                    printf("\tUnknown command\n"); }
        }
        SPI_Nixie_Write(Parm[0]);                                                           // Update Nixie display
        strcpy(LastCmd, inString) ;                                                         // Preserve last command
        free(inString);                                                                     // free buffer
    }
    return 0;
}
