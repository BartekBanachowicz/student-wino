#pragma once
#include "../main_thread.hpp"

int lClock;
int* wineDemands;
int wineDemand;
int* offers;
bool amILider;

#define TAG_WINE_DEMAND 6
#define TAG_BATON 7
#define TAG_GO 8
#define TAG_HOMEBASE 9

//TODO: struct msg