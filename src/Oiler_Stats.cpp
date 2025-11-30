
double Oiler::getRecentTimeSeconds(int rangeIndex) {
    double sum = 0.0;
    // Sum from history
    for(int i=0; i<history.count; i++) {
        // The history buffer is circular, but we just iterate over valid entries 0..count-1?
        // No, history.head points to next write.
        // If count < 20, valid entries are 0 to count-1.
        // If count == 20, valid entries are 0 to 19.
        // Wait, if I use a circular buffer, the valid entries are everywhere.
        // But I only care about the sum. Order doesn't matter for sum.
        // So iterating 0..19 is fine if I check if they are valid?
        // Actually, if count < 20, only indices 0 to count-1 have been written to.
        // If count == 20, all indices have been written to.
        // So iterating 0 to count-1 is correct because I fill linearly until wrap.
        // Wait, if I wrap, head goes to 0, but count stays 20.
        // So if count == 20, I iterate 0..19. Correct.
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
