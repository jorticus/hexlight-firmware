/* 
 * File:   adc.h
 * Author: Jared
 *
 * Created on 13 June 2014, 8:06 PM
 */

#ifndef ADC_H
#define	ADC_H

// Initialize the sound capture module
void ADCInitialize();

// Start capturing audio
void ADCStartCapture();

// Stop capturing audio
void ADCStopCapture();



extern volatile byte* read_buf;

extern volatile bool flag_ready;         // read_buf is ready to be processed (clear it when done)
extern volatile bool flag_processing;    // read_buf is currently being processed (set this yourself and clear when done)


#endif	/* ADC_H */

