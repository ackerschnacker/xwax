/*
 * =====================================================================================
 *
 *       Filename:  raisedcosinefilter.c
 *
 *    Description:  Function to compute a raised-cosine filter for pulse-shaping
 *                  bit sequences 
 *
 *        Version:  1.0
 *        Created:  22.11.2020 12:05:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jan Claussen (JC), jan.claussen10@web.de
 *   Organization:  HAW Hamburg
 *
 * =====================================================================================
 *
 *   Source: https://en.wikipedia.org/wiki/Raised-cosine_filter
 */             

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float sinc(float x)
{
    /* 1 at t=0, else sinc(x) */

    return x == 0.0 ? 1.0 : sin(M_PI * x) / (M_PI * x);
}

/* 
 *  Compute a raised cosine filter where
 *
 *  beta: Roll-of factor of the sinc function
 *  T   : Symbol period
 *  Ts  : Sample period
 */

float* RaisedCosineFilter(float beta, float T, float Ts, unsigned long* L_rc)
{
    int i;
    float t;

    /* Calculate the length of the impulse response */

    unsigned long N = (unsigned long) (T/Ts) + 1;

    /* Allocate memory for the impulse response */

    float *rc = (float *) calloc(N, sizeof(N));

    /* Compute the impulse response (filter coefficients) */

    for(i=0; i < N; ++i) {

        /* 
         *  Compute the time variable and shift the impulse reponse 
         *  by half its length to shift the peak into the middle
         */

        t = (i - N/2.0) * Ts; 

        /* Compute the next coefficient */

        if (fabs(t) == T/(2*beta)) {
            rc[i] = (M_PI / (4*T) * sinc(1 / (2*beta)));
            rc[i] = rc[i] * T; // Apply correction
            continue;
        }

        else {
            rc[i] = (1/T) * sinc(t/T) * (cos(M_PI*beta*t/T)/ (1 - pow(2*beta*t/T, 2)));
            rc[i] = rc[i] * T; // Apply correction
        }
    }

    /* Store filter length in L_rc */

    *L_rc = N; 

    /* Return the impulse response*/

    return rc; 
}

