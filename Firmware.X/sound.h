/* 
 * File:   sound.h
 * Author: Jared
 *
 * Created on 13 June 2014, 8:06 PM
 */

#ifndef SOUND_H
#define	SOUND_H

// Initialize the sound capture module
void SndInitialize();

// Start capturing audio
void SndStartCapture();

// Stop capturing audio
void SndStopCapture();

// Process the audio and compute the FFT
void SndProcess();


#endif	/* SOUND_H */

