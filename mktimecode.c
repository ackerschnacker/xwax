/*
 * Copyright (C) 2018 Mark Hills <mark@xwax.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Edited by Jan Claussen <jan.claussen10@web.de>
 *
 * Experimental program to generate a timecode signal for use
 * with xwax.
 *
 * Command to generate the timecode: 
 *     ./mktimecode | sox -t raw -r 44100 -c 2 -e signed -b 16 - timecode.wav
 *
 * Command to plot the wave and spectrum: 
 *     python plotWave.py
 * 
 * Only the first 5000 samples are plotted. If more are needed, change the plotWave.py accordingly.
 */ 

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include "raisedcosinefilter.h"

#define BANNER "xwax timecode generator " \
    "(C) Copyright 2018 Mark Hills <mark@xwax.org>"


#define OFFSETMOD 1          // Set to 1 for offset modulation and 0 for amplitude modulation 
#define SAMPLERATE 44100     // Sampling rate in bits/s
#define CARRIERFREQ 3000     // Frequency of the carrier wave in Hz
#define SYMBOLRATE 1200      // Rate of symbols (bits)
#define SEED 0x59017         // First bit for the Linear Feedback Shift Register
#define TAPS 0x361e4         // Taps for Linear Feedback Shift Register
#define BITS 20              // Length of the Linear Feedback Shift Register

#define MAX(x,y) ((x)>(y)?(x):(y))
typedef unsigned int bits_t;

static inline bits_t lfsr(bits_t code, bits_t taps);
static inline bits_t fwd(bits_t current, bits_t taps, unsigned int nbits);
static inline float convolve(bits_t x_delays[], float* rc, int N, bits_t x_n);

bits_t *p_x, *p_input;
int i;
float filterOut;

int main(int argc, char *argv[])
{
    unsigned int n;
    int length = 0;
    bits_t bitsequence = SEED;
    bits_t bit = bitsequence&0x1;

    fputs(BANNER, stderr);
    fputc('\n', stderr);

    fprintf(stderr, "Generating %d-bit %d Hz timecode sampled at %d kHz\n",
            BITS, CARRIERFREQ, SAMPLERATE);

    int f = CARRIERFREQ;
    int fs = SAMPLERATE;
    float t, cycle;
    float complex z;
    float x, y;
    short ch[2];

    /* Offset to pull the signal back to 0 afterwards*/

    float complex offset = 0.5 * (1+I);                
   
    /* Variables for the Root Raised Cosine filter */

    unsigned long N_coeff = 0; // Number of filter coefficients
    float *rc;                 // Pointer to the impulse response (filter coefficients)

    /* 
     *  The filter parameters for both signals are equal, which means that if a 
     *  matched filter is needed for demodulation, the same filter can be used.
     */

    if(OFFSETMOD) {
        float beta = 0.1;
        rc = RaisedCosineFilter(beta, 1.0/SYMBOLRATE, 1.0/SAMPLERATE*4, &N_coeff);
    }
    else {
        float beta = 0.1;
        rc = RaisedCosineFilter(beta, 1.0/SYMBOLRATE, 1.0/SAMPLERATE*4, &N_coeff);
    }

    float dc_gain = 0;

    for (i = 0; i < N_coeff; ++i) 
        dc_gain += rc[i];
        
    dc_gain /= N_coeff;

    bits_t bit_delays[N_coeff];

    for (i = 0; i < N_coeff; ++i) {
       bit_delays[i] = 0; 
    }

    p_input = bit_delays;
    float mod;

    for (n = 0; ; n++) {

        /* Update time in seconds and number of cycles */

        t = (double)n / fs;
        cycle = t * SYMBOLRATE;

        /* Create pulse-shaped bit sequence */

        bit = bitsequence & 0x1;
        mod = convolve(bit_delays, rc, N_coeff, bit) / dc_gain / 12; // Why divide by 12?

        /* Create carrier as a complex exponential */

        z = cexp(I*2*M_PI*f*n/fs);      

        /* Modulate the carrier with the pulse-shaped bitstream */

        if(OFFSETMOD) 
            z = 0.5 * (z + mod*(0.707+0.707*I) - offset);
        else
            z = 0.5 * (-z * (1 + mod*(0.707+0.707*I)));

        /* Convert the complex signal into real signals */

        x = creal(z);
        y = cimag(z);

        /* 
         *  Uncomment to compare the pulse-shaped bit sequence to the
         *  original bit sequence
         */

        //x = mod;
        //y = bit;
        
        /* Write out 16-bit PCM data of both channels*/

        ch[0] = round(32678 * x);
        ch[1] = round(32678 * y);

        fwrite(ch, sizeof(signed short), 2, stdout);

        /* Advance the bitstream if required */

        if ((int)cycle > length ) {
            assert((int)cycle - length == 1);
            bitsequence = fwd(bitsequence, TAPS, BITS);
            if (bitsequence == SEED) /* LFSR period reached */
                break;
            length = cycle;
        }
    }

    free(rc);

    fprintf(stderr, "Generated %0.1f seconds of timecode\n",
            (double)length / CARRIERFREQ);

    fprintf(stderr, "\n");
    fprintf(stderr, "    {\n");
    fprintf(stderr, "        .resolution = %d,\n", CARRIERFREQ);
    fprintf(stderr, "        .bits = %d,\n", BITS);
    fprintf(stderr, "        .seed = 0x%08x,\n", SEED);
    fprintf(stderr, "        .taps = 0x%08x,\n", TAPS);
    fprintf(stderr, "        .length = %d,\n", length);
    fprintf(stderr, "        .safe = %d,\n", MAX(0, length - 4 * CARRIERFREQ));
    fprintf(stderr, "    }\n");

    return 0;
}

/*
 * Function to calculate the next bit in the LFSR sequence
 */

static inline bits_t lfsr(bits_t code, bits_t taps)
{
    bits_t taken;
    int xrs;

    taken = code & taps;
    xrs = 0;
    while (taken != 0x0) {
        xrs += taken & 0x1;
        taken >>= 1;
    }

    return xrs & 0x1;
}

/*
 * LFSR in the forward direction
 */

static inline bits_t fwd(bits_t current, bits_t taps, unsigned int nbits)
{
    bits_t l;

    /* New bits are added at the MSB; shift right by one */

    l = lfsr(current, taps | 0x1);
    return (current >> 1) | (l << (nbits - 1));
}

/*
 * Convolution function with a circular buffer
 */

static inline float convolve(bits_t x_delays[], float *rc, int N, bits_t x_n)
{
    filterOut = 0;

    *p_input = x_n;     // Store newest input value in x_delays
    p_x = p_input;      // Make a copy of the pointer

    /* Convolve the delays with the filter coefficients */

    if(++p_input > &x_delays[N-1])
        p_input = x_delays;

    for(i = 0; i < N; i++) {

        filterOut += (*p_x--) * rc[i];

        if(p_x < x_delays)
            p_x = &x_delays[N-1];
    }

    /* Return and apply gain correction */

    return filterOut;
}
