#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"                                                  // Required for manually toggling GPIO pins (clock)

#define eof            255                                                  // EOF in stdio.h -is -1, but getchar returns int 255 to avoid blocking
#define MWidth          10                                                  // Width of terminal command margin (in columns)
#define SysClock       125                                                  // System clock     (Pico default)

// Data for clock face generated by Excel spreadsheet...
uint8_t FaceX[] = {235,239,243,247,251,255,255,254,254,254,254,254,254,254,253,253,253,252,252,251,251,250,250,249,248,248,247,246,245,244,244,243,242,241,240,239,221,224,227,231,234,238,236,235,234,233,232,230,229,228,226,225,224,222,221,219,218,216,214,213,211,209,208,206,204,203,201,199,197,195,193,182,184,186,188,190,191,190,188,186,184,182,180,178,176,174,172,170,167,165,163,161,159,157,155,153,150,148,146,144,142,139,137,135,133,131,128,128,128,128,128,128,126,124,122,120,117,115,113,111,109,107,104,102,100,98,96,94,92,90,87,85,83,81,79,77,75,73,71,69,67,75,73,71,69,67,65,64,62,60,58,56,54,53,51,49,47,46,44,43,41,39,38,36,35,33,32,31,29,28,27,25,24,23,22,20,36,33,30,26,23,19,18,17,16,15,14,13,12,12,11,10,9,9,8,7,7,6,6,5,5,4,4,4,3,3,3,3,3,3,2,22,18,14,10,6,2,2,3,3,3,3,3,3,4,4,4,5,5,6,6,7,7,8,9,9,10,11,12,12,13,14,15,16,17,18,36,33,30,26,23,19,20,22,23,24,25,27,28,29,31,32,33,35,36,38,39,41,43,44,46,47,49,51,53,54,56,58,60,62,64,75,73,71,69,67,65,67,69,71,73,75,77,79,81,83,85,87,90,92,94,96,98,100,102,
104,107,109,111,113,115,117,120,122,124,126,128,128,128,128,128,128,131,133,135,137,139,142,144,146,148,150,153,155,157,159,161,163,165,167,170,172,174,176,178,180,182,184,186,188,190,182,184,186,188,190,191,193,195,197,199,201,203,204,206,208,209,211,213,214,216,218,219,221,222,224,225,226,228,229,230,232,233,234,235,236,221,224,227,231,234,238,239,240,241,242,243,244,244,245,246,247,248,248,249,250,250,251,251,252,252,253,253,253,254,254,254,254,254,254,254} ;
uint8_t FaceY[] = {128,128,128,128,128,128,128,126,124,122,120,117,115,113,111,109,107,104,102,100,98,96,94,92,90,87,85,83,81,79,77,75,73,71,69,67,75,73,71,69,67,65,64,62,60,58,56,54,53,51,49,47,46,44,43,41,39,38,36,35,33,32,31,29,28,27,25,24,23,22,20,36,33,30,26,23,19,18,17,16,15,14,13,12,12,11,10,9,9,8,7,7,6,6,5,5,4,4,4,3,3,3,3,3,3,2,22,18,14,10,6,2,2,3,3,3,3,3,3,4,4,4,5,5,6,6,7,7,8,9,9,10,11,12,12,13,14,15,16,17,18,36,33,30,26,23,19,20,22,23,24,25,27,28,29,31,32,33,35,36,38,39,41,43,44,46,47,49,51,53,54,56,58,60,62,64,75,73,71,69,67,65,67,69,71,73,75,77,79,81,83,85,87,90,92,94,96,98,100,102,104,107,109,111,113,115,117,120,122,124,126,128,128,128,128,128,128,131,133,135,137,139,142,144,146,148,150,153,155,157,159,161,163,165,167,170,172,174,176,178,180,182,184,186,188,190,182,184,186,188,190,191,193,195,197,199,201,203,204,206,208,209,211,213,214,216,218,219,221,222,224,225,226,228,229,230,232,233,234,235,236,221,224,227,231,234,238,239,240,241,242,243,244,244,245,246,247,248,248,249,250,250,251,251,252,
252,253,253,253,254,254,254,254,254,254,254,235,239,243,247,251,255,254,254,254,254,254,254,254,253,253,253,252,252,251,251,250,250,249,248,248,247,246,245,244,244,243,242,241,240,239,221,224,227,231,234,238,236,235,234,233,232,230,229,228,226,225,224,222,221,219,218,216,214,213,211,209,208,206,204,203,201,199,197,195,193,182,184,186,188,190,191,190,188,186,184,182,180,178,176,174,172,170,167,165,163,161,159,157,155,153,150,148,146,144,142,139,137,135,133,131} ;
// (Number of pixels: 421)

// Store clock hands co-ordinates...
uint8_t HandsX[192] = {} ;                                                  // Each hand requires 64 bytes - 3x64=192
uint8_t HandsY[192] = {} ;
int Hours=0, Mins=0, Secs=0, Angle, StartX, StartY, Radius ;
float Radians ;

//int tmp ;                                                                 // Debug and general use

char MarginFW[MWidth+1], MarginVW[MWidth+1] ;                               // Fixed Width & Variable Width strings to create a fixed margin
int c, i, ParmCnt = 0, Parm[4] ;
int MarginCount = 0 ;                                                       // Manual count of characters written to terminal

bool Repeating_Timer_Callback(struct repeating_timer *t) {
    int i, steps=64, MidX=128, MidY=128 ;
    // Bump the time...
    if ((++Secs)>59) Secs=0 ;                                               // Always bump seconds
    if (Secs==0) { if ((++Mins)>59 ) Mins=0 ;  }                            // Bump minutes when seconds = 0
    if ((Mins==0) && (Secs==0)) { if ((++Hours)>24) Hours=0 ; }             // Bump hours when minutes and seconds = 0

    // Calculate seconds hand...
    i=0, Radius=127 ;                                                       // Radius=Length of seconds hand
    Angle=270-(Secs*6) ;                                                    // Angle in degrees, shifted 90 degree anti-clockwise
    Radians=Angle*3.14159/180 ;                                             // Angle in radians
    StartX=Radius*cos(Radians)+MidX ;
    StartY=Radius*sin(Radians)+MidY ;
    while(i<steps) { HandsX[i]=StartX+i*(MidX-StartX)/steps ;
                     HandsY[i]=StartY+i*(MidY-StartY)/steps ;
                     i++ ; }
    // Calculate minutes hand...
    i=0, Radius=95 ;                                                        // Radius=Length of minutes hand
    Angle=270-(Mins*6) ;                                                    // Angle in degrees, shifted 90 degree anti-clockwise
    Radians=Angle*3.14159/180 ;                                             // Angle in radians
    StartX=Radius*cos(Radians)+MidX ;
    StartY=Radius*sin(Radians)+MidY ;
    i=0 ;
    while(i<steps) { HandsX[i+steps]=StartX+i*(MidX-StartX)/steps ;
                     HandsY[i+steps]=StartY+i*(MidY-StartY)/steps ;
                     i++ ; }
    // Calculate hours hand...
    i=0, Radius=64 ;                                                        // Radius=Length of hours hand
    // Note: Hours hand progresses between hours in 5 partial increments, each increment measuring 12 minutes.
    //       Each 12 minute increment adds an additional 6 degrees of rotation to the hours hand.
    Angle=5*(270-(((Hours%12)*6)+(Mins/12)%5)) ;                            // Angle in degrees, shifted 90 degree anti-clockwise,
                                                                            //   and scaled by 5 to provide range 0=>12
    Radians=Angle*3.14159/180 ;                                             // Angle in radians
    StartX=Radius*cos(Radians)+MidX ;
    StartY=Radius*sin(Radians)+MidY ;
    while(i<steps) { HandsX[i+2*steps]=StartX+i*(MidX-StartX)/steps ;
                     HandsY[i+2*steps]=StartY+i*(MidY-StartY)/steps ;
                     i++ ; }

//  printf("%s%d:%d:%d - %d\n",MarginFW,Hours,Mins,Secs,tmp) ;              // Debug

    return true;
}

void VerText (char *outText) {
    // Print version info aligned to current margin settings...
    MarginVW[MWidth] = '\0' ;                                               // Calculate padding required for command characters and cursor
    sprintf(outText, "%s|-------------------------|\n"
                     "%s|  Analog Clock Simulator |\n"
                     "%s|     Version 1.0.0       |\n"
                     "%s|    25th August 2023     |\n"
                     "%s|-------------------------|\n", 
                     MarginVW, MarginFW, MarginFW, MarginFW, MarginFW ) ;
}

void HlpText (char *outStr) {
    // Print Help text aligned to current margin settings...
    MarginVW[MWidth - 2] = '\0' ;                                            // Calculate padding required for command characters and cursor
    sprintf(outStr,"%s?   - Help\n"
                   "%sV   - Version info\n"
                   "%sX   - Invert X axis\n"
                   "%sY   - Invert Y axis\n"
                   "%sT   - Set Time: Notation: HH:MM:SS or HH,MM,SS\n"
                   "%s                HH can be either 12 or 24 hour notation\n"
                   "%s                  e.g. '03:00:00' is the same as '15:00:00'\n"
                   "%s                Delimiter can be either ':' or ','\n"
                   "%s                  e.g. '15:00:00' is the same as '15,00,00'\n"
                   "%s                MM is in the range 0<=MM<=59\n"
                   "%s                SS is in the range 0<=SS<=59\n"
                   "%s                Leading zeros can be ommited.\n"
                   "%s                  e.g. '1:2:3' is the same as '01:02:03'\n"
                   "%s                Trailing parameters can be ommited, and will be set to zero.\n"
                   "%s                  e.g. '12:15' is the same as '12:15:00'\n"
                   "%s                       '12'    is the same as '12:00:00'\n"
                   "%sL   - Set Level: Notation: 2 digit percentage 0<=nn<=100\n",
                   MarginVW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW,
                   MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW, MarginFW) ;
}

static void getLine(char *inStr) {
    int count = 0 ;
    while(1) {
        c = getchar();
        if (c == eof || c == '\n' || c == '\r') break ;                     // Non blocking exit
        putchar(c);                                                         // FullDuplex echo
        *inStr++ = c ;                                                       // Bump pointer, store character
        count ++ ;
    }
    *inStr = '\0' ;
    MarginCount += count ;                                                  // Track number of characters on current line
    return ;
}

int main() {
    char inStr[32], outStr[2000] ;                                          // General purpose input/output buffers
    bool InvX=false, InvY=false ;                                           // Flags to allow inverted output
    int outX, outY ;                                                        // Final value to send to DAC
    uint8_t Level=100 ;                                                     // % scale applied to DAC output level
    set_sys_clock_khz(SysClock*1000, true) ;                                // Set Pico clock speed
    stdio_init_all() ;

// Initialise GPIO ports for use with R-2R resistor network...
// (Setting Max slew rate and gpio drive strength keeps output linear at high frequencies)
    for (int gpio = 0; gpio < 16; gpio++) {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
        gpio_set_slew_rate(gpio, GPIO_SLEW_RATE_FAST);
        gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);
    }

    struct repeating_timer timer;
    add_repeating_timer_ms(-1000, Repeating_Timer_Callback, NULL, &timer) ;
    
    memset(MarginFW,' ',MWidth) ;                                           // Initialise Fixed Width margin...
    MarginFW[MWidth] = '\0' ;                                               //  ... and terminate
    memset(MarginVW,' ',MWidth) ;                                           // Initialise Variable Width margin...
    MarginVW[MWidth] = '\0' ;                                               //  ... and terminate

    while (!stdio_usb_connected()) { sleep_ms(100); }                       // Wait for USB connection...

// Send (optional) start-up messages to terminal...
    VerText(outStr) ;                                                       // Version text
    printf(outStr) ;                                                        // Update terminal

    while(1) {
        memset(MarginVW,' ',MWidth) ;                                       // Re-initialise Variable Width margin...
        MarginVW[MWidth] = '\0' ;                                           //  ... and terminate
        printf(">") ;                                                       // Command prompt
        MarginCount = 1 ;                                                   // Reset count and bump for command prompt

        // Clock display code...
        gpio_clr_mask(0xff) ;                                               // Clear first 16 GPIO outputs
        for (;;) {                                                          // Continual loop
            // Draw the clock face...
            for (int i=0; i<sizeof(FaceX); i++) {
                outX=FaceX[i] ;                                             // Default 8 bit output
                if (!InvX) outX=255-outX ;                                  // Inverted 8 bit output
                outX=outX*Level/100 ;                                       // Scale the output
                gpio_put_masked(0x00ff,outX) ;                              // Transfer data to the DAC A
                outY=FaceY[i] ;                                             // Default 8 bit output
                if (!InvY) outY=255-outY ;                                  // Inverted 8 bit output
                outY=outY*Level/100 ;                                       // Scale the output
                gpio_put_masked(0xff00,outY<<8) ;                           // Transfer data to the DAC B
                sleep_us(2) ;                                               // Pause for on-screen persistance
            }
            // Draw the clock hands...
            for (i=0; i<192; i++) {                                         // 3 hands @ 64 pixels each = 192
                outX=HandsX[i] ;                                            // Default 8 bit output
                if (!InvX) outX=255-outX ;                                  // Inverted 8 bit output
                outX=outX*Level/100 ;                                       // Scale the output
                gpio_put_masked(0x00ff,outX) ;                              // Transfer data to the DAC A
                outY=HandsY[i] ;                                            // Default 8 bit output
                if (!InvY) outY=255-outY ;                                  // Inverted 8 bit output
                outY=outY*Level/100 ;                                       // Scale the output
                gpio_put_masked(0xff00,outY<<8) ;                           // Transfer data to the DAC B
                sleep_us(2) ;                                               // Pause for on-screen persistance
            }
            // Check for console input...
            c = getchar_timeout_us (0);                                     // Non-blocking char input
            if (c!=EOF) {                                                   // c=EOF if no input
                // falls through here if we have an input character...
                putchar(c) ;                                                // Echo to terminal
                MarginCount = 2 ;                                           // Reset count and bump for command prompt + input
                MarginVW[MWidth - MarginCount] = '\0' ;                     // Calculate padding required  for command characters and cursor
                if ((c=='T') or (c=='t')) {                                 // Set time
                    printf("%sSet time (format HH:MM:SS)\n%s",MarginVW, MarginFW ) ;
                    getLine(inStr) ;                                        // Get the console input
                    Parm[0]=0,  Parm[1]=0,  Parm[2]=0,  Parm[3]=0 ;         // Reset all command line parameters
                    i=0, ParmCnt=0 ;                                        // Reset all command line counters
                    while (i<strlen(inStr) ) {
                        if ((inStr[i]==':')||(inStr[i]==',')) {             // Next parameter
                            ParmCnt++ ; }
                        else if (isdigit(inStr[i])) { 
                            Parm[ParmCnt] *= 10;                            // Next digit. Bump the existing decimal digits
                            Parm[ParmCnt] += inStr[i] - '0'; }              // Convert character to integer and add
                        i++ ;                                               // Next character
                    }
                    inStr[0]='\0' ;                                         // Reset input buffer
                    Hours=Parm[0]%24 ; Mins=Parm[1]%60 ; Secs=Parm[2]%60 ;  // Set the time from parameters
                    printf("\n%sClock set to %02d:%02d:%02d\n>",MarginFW,Hours,Mins,Secs) ;
                }
                if ((c=='L') or (c=='l')) {                                 // Set level
                    printf("%sSet level (percentage format 0<=nn<=100)\n%s",MarginVW, MarginFW ) ;
                    getLine(inStr) ;                                        // Get the console input
                    Parm[0]=0,  Parm[1]=0,  Parm[2]=0,  Parm[3]=0 ;         // Reset all command line parameters
                    i=0, ParmCnt=0 ;                                        // Reset all command line counters
                    while (i<strlen(inStr) ) {
                        if ((inStr[i]==':')||(inStr[i]==',')) {             // Next parameter
                            ParmCnt++ ; }
                        else if (isdigit(inStr[i])) { 
                            Parm[ParmCnt] *= 10;                            // Next digit. Bump the existing decimal digits
                            Parm[ParmCnt] += inStr[i] - '0'; }              // Convert character to integer and add
                        i++ ;                                               // Next character
                    }
                    inStr[0]='\0' ;                                         // Reset input buffer
                    Level=Parm[0] ;                                         // Set the level from parameter
                    printf("\n%sLevel set to %02d%%\n>",MarginFW,Level) ;
                }
                else if ((c=='x') or (c=='X')) {
                    InvX = !InvX ;
                    if (InvX) printf("%sX axis inverted.\n>",MarginVW) ;   // Print current status
                    else      printf("%sX axis not inverted.\n>",MarginVW) ; 
                }
                else if ((c=='y') or (c=='Y')) {
                    InvY = !InvY ;
                    if (InvY) printf("%sY axis inverted.\n>",MarginVW) ;   // Print current status
                    else      printf("%sY axis not inverted.\n>",MarginVW) ; 
                }
                else if (c=='?') {
                    HlpText(outStr) ;                                       // Create output string...
                    printf("%s\n>",outStr) ;                                // ... and print it
                }
                else if ((c=='v') or (c=='V')) {
                    VerText(outStr) ;                                       // Create output string...
                    printf("%s\n>",outStr) ;                                // ... and print it
                }
                else {
                    printf("%sUnknown command - press '?' for help.\n>",MarginVW) ;
                }
            }
        }
    }
}
