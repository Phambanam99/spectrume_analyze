#include "metadata.h"

// Định nghĩa và khởi tạo các biến toàn cục
int metaCols = 0;
int metaRows = 0;
int startFreq = 0;
int endFreq = 0;
int stepFreq = 0;
int hops = 1;
int currentHopNumber = 0;

time_t scanEnd = 0;
time_t scanBeg = 0;

float avgScanDur = 0.0f;
float sumScanDur = 0.0f;

std::string firstAcqTimestamp = "";
std::string lastAcqTimestamp = "";

int cntTimeStamps = 0;
int actual_samplerate = 0;
int excludedBINS = 0;
int actualBINS = 0;

double cropFreqOffset = 0.0;
