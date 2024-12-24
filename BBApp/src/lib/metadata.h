

#ifndef METADATA_H
#define METADATA_H
#include<string>
#include <time.h>
extern int metaCols, metaRows;
extern int startFreq, endFreq, stepFreq;
extern int hops;
extern int currentHopNumber;
extern time_t scanEnd, scanBeg;
extern float avgScanDur, sumScanDur;
extern std::string firstAcqTimestamp, lastAcqTimestamp;
extern int cntTimeStamps;
extern int actual_samplerate;
extern int excludedBINS;
extern int actualBINS;
extern double cropFreqOffset;

#endif // METADATA_H
