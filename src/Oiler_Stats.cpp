
#include "Oiler.h"

double Oiler::getRecentTimeSeconds(int rangeIndex) {
    double sum = 0.0;
    // Sum from history
    for(int i=0; i<history.count; i++) {
        // The history buffer is circular for the last 20 oiling events (to represent latest statistics).
        sum += history.timeInRanges[i][rangeIndex];
    }
    // Add current interval
    sum += currentIntervalTime[rangeIndex];
    return sum;
}

int Oiler::getRecentOilingCount(int rangeIndex) {
    int c = 0;
    for(int i=0; i<history.count; i++) {
        if (history.oilingRange[i] == rangeIndex) {
            c++;
        }
    }
    return c;
}

double Oiler::getRecentTotalTime() {
    double sum = 0.0;
    for(int i=0; i<NUM_RANGES; i++) {
        sum += getRecentTimeSeconds(i);
    }
    return sum;
}
