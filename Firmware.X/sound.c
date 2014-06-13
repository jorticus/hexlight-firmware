/*
 * Sound capture module
 * Author: Jared Sanson
 *
 * Records audio from the microphone and computes the spectrum
 */


#include <p32xxxx.h>
#include <peripheral/timer.h>
#include <peripheral/adc10.h>
#include <peripheral/dma.h>
#include "common.h"
#include "hardware.h"

///// Configuration /////

#define DMA_CHANNEL DMA_CHANNEL0
#define DMA_VECTOR _DMA0_VECTOR

#define BUFFER_SIZE 64  // Maximum is 65536
//TODO: How big should this buffer be for the FFT?
// Would it make sense to do some sort of hybrid double-buffer + circular buffer
// implementation if the FFT requires a very large buffer? ie. the circular buffer is updated
// in chunks.

#define SAMPLE_RATE 44100 // Hz
#define OVERSAMPLING 1 // times



///// Internal Variables //////

// Use double buffering
static volatile byte snd_buf1[BUFFER_SIZE];
static volatile byte snd_buf2[BUFFER_SIZE];

static volatile byte* write_buf = snd_buf1;      // Buffer used for capturing audio samples
static volatile byte* read_buf = snd_buf2;       // Buffer used for processing audio samples

static volatile bool flag_ready = false;         // read_buf is ready to be processed (clear it when done)
static volatile bool flag_processing = false;    // read_buf is currently being processed


///// Macros /////

#define SAMPLE_TIMER_PERIOD 123 // TODO: Determine proper period


///// Code /////

void SndInitialize() {
    ///// Init ADC /////

    // A1 - Configure analog input pins in AD1PCFG
    // B1 - Select analog inputs to the ADC MUXs in AD1CHS
    // C1 - Select the format of the result FORM
    // C2 - Select the sample clock source SSRC
    // D1 - Select the voltage reference VCFG
    // D2 - Select the scan mode CSCNA
    // D3 - Select the number of conversions per interrupt SMP
    // D4 - Set buffer fill mode BUFM
    // D5 - Select the Mux to be connected to the ADC in ALTS
    // E1 - Select the ADC clock source ADRC
    // E2 - Select the sample time SAMC
    // E3 - Select the ADC clock prescaler ADCS
    // F - turn the ADC module on

    TRISBbits.TRISB8 = INPUT;
    AD1PCFGbits.PCFG8 = ANALOG;

    // 1 sample per ADC interrupt -> required for DMA
    // Using only MuxA (no alt MuxB), with a 16-bit buffer, using 16-bit fractional format.
    // Clock should be derived from peripheral clock, and sampling should start on timer match
    AD1CON1 = ADC_MODULE_OFF | ADC_IDLE_CONTINUE | ADC_FORMAT_FRACT16 | ADC_CLK_TMR | ADC_AUTO_SAMPLING_ON;
    AD1CON2 = ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_1 | ADC_BUF_16 | ADC_ALT_INPUT_OFF;
    AD1CON3 = ADC_CONV_CLK_PB | ADC_SAMPLE_TIME_12;

    // TODO: Determine what type of result format to use (integer/fractional/etc.)
    // TODO: What sample rate to use? Should we oversample?
    //       Can the timer module provide the desired sample rate, or should we just sample as fast as possible?

    // MuxA inputs:
    AD1CHSbits.CH0SA = 8;   // +: AN8
    AD1CHSbits.CH0SB = 0;   // -: VRef-
    


    ///// Configure ADC trigger timebase /////
    // NB: Only Timer3 can be used for triggering the ADC
    TMR3 = 0;
    PR3 = SAMPLE_TIMER_PERIOD;
    T3CON = T3_IDLE_CON | T3_GATE_OFF | T3_PS_1_1 | T3_SOURCE_INT;


    ///// Configure DMA /////
    DmaChnOpen(DMA_CHANNEL, DMA_CHN_PRI0, DMA_OPEN_DEFAULT);

    // Tell the DMA to automatically fill the buffer from the ADC interrupt
    DmaChnSetEventControl(DMA_CHANNEL, DMA_EV_START_IRQ(_ADC_IRQ));
    DmaChnSetTxfer(DMA_CHANNEL, (void*)&ADC1BUF0, write_buf, 1, BUFFER_SIZE, 1);

    DmaChnSetEvEnableFlags(DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    INTSetVectorPriority(INT_VECTOR_DMA(DMA_CHANNEL), INT_PRIORITY_LEVEL_5);
    INTSetVectorSubPriority(INT_VECTOR(DMA_CHANNEL), INT_SUB_PRIORITY_LEVEL_3);
    INTEnable(INT_SOURCE_DMA(DMA_CHANNEL), INT_ENABLED);


    // Clear buffers
    uint i;
    for (i=0; i<BUFFER_SIZE; i++) {
        snd_buf1[i] = 0;
        snd_buf2[i] = 0;
    }
}

void SndStartCapture() {
    AD1CON1bits.ON = 1;
    T3CONbits.ON = 1;

    DmaChnEnable(DMA_CHANNEL);
    //DmaChnForceTxfer(DMA_CHANNEL);
}

void SndStopCapture() {
    DmaChnDisable(DMA_CHANNEL);

    T3CONbits.ON = 0;
    AD1CON1bits.ON = 0;

    // TODO: What do we do with the currently processing DMA? Can we abort it?
}

void SndProcess() {
    // Process FFT stuff in the background
    // FFT data will be available in the FFT buffer when flagged.
    if (flag_ready) {
        flag_processing = true;
        flag_ready = false;



        flag_processing = false;
    }
}


static void swap_buffers() {
    // Swap the read and write buffers
    byte* tmp = read_buf;
    read_buf = write_buf;
    write_buf = tmp;

    // Tell the DMA to use the new write buffer
    DmaChnSetTxfer(DMA_CHANNEL, (void*)&ADC1BUF0, write_buf, 1, BUFFER_SIZE, 1);

    // Signal that the read buffer is ready to be processed
    flag_ready = true;
}


///// Interrupt Service Routines /////

void __ISR(DMA_VECTOR, IPL6AUTO) sound_dma_isr(void) {
    // Called when a chunk of audio data has been recorded into the write buffer.
    toggle(PIO_LED2);


    // If the buf_ready flag hasn't been cleared, then buffer has underrun
    // (ie. the application didn't get start processing the current buffer)
    if (flag_ready) {


    }
    // If the processing flag hasn't been cleared, then buffer has overflowed
    // (ie. it's still processing the current buffer)
    else if (flag_processing) {

        return; // Throw away the new buffer's data and capture again
    }

    // Swap the read and write buffers
    swap_buffers();

    DmaChnClrEvFlags(DMA_CHANNEL, DMA_EV_BLOCK_DONE);
    INTClearFlag(INT_SOURCE_DMA(DMA_CHANNEL));
}

void __ISR(_TIMER_3_VECTOR, IPL5AUTO) timer3_isr(void) {
    // For debug, called at the sampling rate of the ADC
    toggle(PIO_LED1);
}
