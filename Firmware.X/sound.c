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

#define OVERSAMPLING 8
#define SAMPLE_RATE (44100*OVERSAMPLING) // Hz

#if OVERSAMPLING > 16
    #error Oversampling is limited to between 1-16 times
#endif


///// Internal Variables //////

// Use double buffering
static volatile byte snd_buf1[BUFFER_SIZE];
static volatile byte snd_buf2[BUFFER_SIZE];

static volatile byte* write_buf = snd_buf1;      // Buffer used for capturing audio samples
static volatile byte* read_buf = snd_buf2;       // Buffer used for processing audio samples

static volatile bool flag_ready = false;         // read_buf is ready to be processed (clear it when done)
static volatile bool flag_processing = false;    // read_buf is currently being processed

static volatile uint write_idx = 0;


///// Macros /////

#define ADC_SAMPLES_PER_INT_x (OVERSAMPLING << _AD1CON2_SMPI_POSITION)

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
    AD1CON2 = ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_x | ADC_BUF_16 | ADC_ALT_INPUT_OFF;
    AD1CON3 = ADC_CONV_CLK_PB | ADC_SAMPLE_TIME_12;

    // TODO: Determine what type of result format to use (integer/fractional/etc.)

    // MuxA inputs:
    AD1CHSbits.CH0SA = 8;   // +: AN8
    AD1CHSbits.CH0SB = 0;   // -: VRef-

    // Enable ADC interrupt
    //INTSetVectorPriority(INT_AD1, 3);
    //INTSetVectorSubPriority(INT_AD1, 3);
    //INTEnable(INT_AD1, INT_ENABLED);
    IPC6bits.AD1IP = 4;
    IPC6bits.AD1IS = 3;
    IFS1bits.AD1IF = 0;
    IEC1bits.AD1IE = 1;


    ///// Configure ADC trigger timebase /////
    // NB: Only Timer3 can be used for triggering the ADC
    OpenTimer3(T3_IDLE_CON | T3_GATE_OFF | T3_PS_1_1 | T3_SOURCE_INT, pb_clock/SAMPLE_RATE);

    // Enable interrupt for debugging
    /*INTSetVectorPriority(INT_T3, 5);
    INTSetVectorSubPriority(INT_T3, 3);
    INTEnable(INT_T3, INT_ENABLED);*/


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

    //DmaChnEnable(DMA_CHANNEL);
    //DmaChnForceTxfer(DMA_CHANNEL);
}

void SndStopCapture() {
    DmaChnDisable(DMA_CHANNEL);

    T3CONbits.ON = 0;
    AD1CON1bits.ON = 0;

    // TODO: What do we do with the currently processing DMA? Can we abort it?
}

void SndProcess() {
    // Process the audio buffer
    if (flag_ready) {
        flag_processing = true;
        flag_ready = false;

        //TODO: Do work

        flag_processing = false;
    }
}


static void swap_buffers() {

    // If the buf_ready flag hasn't been cleared, then buffer has underrun
    // (ie. the application didn't get to start processing the current buffer)
    if (flag_ready) {
        _LAT(PIO_LED2) = HIGH;

    }
    // If the processing flag hasn't been cleared, then buffer has overflowed
    // (ie. it's still processing the current buffer)
    else if (flag_processing) {
        _LAT(PIO_LED2) = HIGH;
        return; // Throw away the new buffer's data and capture again
    }

    // Swap the read and write buffers
    volatile byte* tmp = read_buf;
    read_buf = write_buf;
    write_buf = tmp;

    // Signal that the read buffer is ready to be processed
    flag_ready = true;
}


///// Interrupt Service Routines /////


/*void __ISR(_TIMER_3_VECTOR, IPL5) timer3_isr(void) {
    // For debug. Called at the sampling rate of the ADC
    toggle(PIO_LED1);
    INTClearFlag(INT_T3);
}*/

void __ISR(_ADC_VECTOR, IPL6) adc_isr(void) {
    // Called when the ADC has finished sampling N samples
    _LAT(PIO_LED2) = 1;

    // Average N samples into one and add to the buffer
    #if OVERSAMPLING > 1
        uint i;
        uint sample = 0;
        uint* buf = (uint*)&ADC1BUF0;
        for (i=0; i<OVERSAMPLING; i++) {
            sample += buf[i];
        }
        sample /= OVERSAMPLING;
        write_buf[write_idx] = sample;
    #else
        write_buf[write_idx] = ADC1BUF0;
    #endif

    // Update the buffer
    if (write_idx++ == BUFFER_SIZE) {
        write_idx = 0;
        swap_buffers();
    }

    _LAT(PIO_LED2) = 0;
    INTClearFlag(INT_AD1);
}
