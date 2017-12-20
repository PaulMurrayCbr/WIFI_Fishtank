#ifndef WFFT_BASE
#define WFFT_BASE 1

#define DEBUG 1

#ifdef DEBUG

#define LOG(s) Serial.println(s)
#define LOGN(s) Serial.print(s)

#else

#define LOG(s)
#define LOGN(s)

#endif

#endif

