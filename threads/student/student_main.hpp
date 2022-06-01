#pragma once
#include "../main_thread.hpp"

int* wineDemands;
int* goCounters;
int wineDemand;
int* winemakersClocks;
int* offers;
bool amILider;

#define TAG_WINE_DEMAND 6
#define TAG_BATON 7
#define TAG_GO 8
#define TAG_HOMEBASE 9

#define MAX_WINE 6
#define MAX_SLEEP 3

typedef struct msg_s{
    int wineDemands [STUDENTS] ;
    int goCounters [STUDENTS];
    int wineOffers [WINEMAKERS];
    int winemakersClocks [WINEMAKERS];
};


