#pragma once
#include "../consts.hpp"

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <queue>
#include<array>

#define MAX_WINE_WINEMAKER 10
#define SAFE_PLACES 1 

#define TAG_SAFE_PLACE_DEMAND 3 //I need safe place


extern int wineAmount; // amount of wine made by me - my offer
extern int safePlaces; // number of places in critical section
extern int wmakersAfterMe; // after me in queue to critical section
extern bool demand; //do I need critical section
extern int acksLeft; // how many acks needed to enter critical section

int winemakerMain();
void produceWine();
void meetStudent(int studentRank, int wineToGive, bool* activeMeeting);
bool askForSafePlace();


