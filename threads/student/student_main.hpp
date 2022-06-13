#pragma once
#include "../consts.hpp"
#include <unistd.h>
#include <list>
#include <algorithm>

int* wineDemands;
int* goCounters;
int wineDemand;
int* winemakersClocks;
int* offers;
bool* freeStudents;
bool amILider;

#define TAG_WINE_DEMAND 6
#define TAG_BATON 7
#define TAG_GO 8
#define TAG_HOMEBASE 9

#define MAX_WINE 6
#define MAX_SLEEP 3

typedef struct{
    int wineDemands [STUDENTS] ;
    int goCounters [STUDENTS];
    int wineOffers [WINEMAKERS];
    int winemakersClocks [WINEMAKERS];
    bool freeStudents[STUDENTS];
}msg_s;


