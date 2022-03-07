#ifndef __THRESHOLDCHECKER_HPP__
#define __THRESHOLDCHECKER_HPP__

typedef struct {
	int Func;
	char Enable;
	float Value;
}THRESHOLD_SETTING;

void CheckStorageThreshold(char* SN, int Temperature, int AvgEraseCount, int Lifespan);
#endif