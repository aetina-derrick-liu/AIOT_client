#ifndef __LIFESPANCALCULATOR_HPP__
#define __LIFESPANCALCULATOR_HPP__

typedef struct{
	long CreateDate;
	float InitHealth;
	float TotalCapacity;
	float CurrentCapacity;
	int PECycle;
}LIFESPAN_DATA;

int CalculateLifeSpan(char* StorageSN, float Health);

#endif