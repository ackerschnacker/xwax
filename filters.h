#ifndef FILTER_H

#define FILTER_H

#include <math.h>
#include <stdbool.h>

int lp_xh = 0;
double lp_Wc, lp_c;

int left_old = 0;
int right_old = 0;
int primary_old = 0;
int secondary_old = 0;

int ema_primary_old = 0;
int ema_secondary_old = 0;

/* 
 * Applies a lowpass filter to the input signal x.
 * Wc is the normalized cut-off frequency 0 < Wc < 1, i.e. 2 * cutoff_freq / sampling_freq
 */
inline int aplowpass(const int x)
{
    int ap_y;
    int y;
    int lp_xh_new = 0;

    lp_xh_new = x - lp_c*lp_xh;
    ap_y = lp_c * lp_xh_new + lp_xh;
    lp_xh = lp_xh_new;
    y = 0.5 * (x + ap_y);

    return y;
}

/* 
 * Initializes the lowpass filter with a given sampling rate and cutoff-frequency 
 */
inline void aplowpass_init(const unsigned short cutoff_freq, const unsigned short sampling_freq)
{
    lp_Wc = 2.0 * cutoff_freq / sampling_freq;
    lp_c = (tan(M_PI * lp_Wc / 2) - 1) / (tan(M_PI * lp_Wc / 2) + 1);
}

inline int ema(int x, int *ema_old, double alpha)
{
	int y = alpha * x + (1 - alpha) * *ema_old;
	*ema_old = y;
	return 2 * y; 
}

inline int discrete_derivative(int x, int *x_old)
{
	int y = x - *x_old;
	*x_old = x;
	return 2 * y;
}
#endif /* end of include guard FILTER_H */
