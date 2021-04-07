
# Simple python script to plot a waveform with the matplotlib library
# Author: Jan Claußen (jan.claussen10@web.de)

import numpy as np
import scipy.signal as signal
from scipy.io.wavfile import read
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq, fftshift

# read audio samples
fs, data = read("timecode.wav")
data = data[0:10000] #comment this line to print full wave file (takes long)
L = len(data)
t = L/fs

spectrum = fft(data[:,0])
freqs = fftshift(fftfreq(L)) * fs

mid = L//2
P1 = spectrum[mid:L]
P2 = spectrum[0:mid]
shifted_spectrum = np.concatenate([P1,P2])

fig, (ax1, ax2) = plt.subplots(2)
fig.suptitle('Signal and spectrum')
ax1.plot(data[0:L])
ax2.plot(freqs, abs(shifted_spectrum))
plt.show()
