#pragma once
#include "../main_thread.hpp"

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>

//TODO: wczytywać jako parametr
#define MAX_WINE 10
#define MAX_SLEEP 5
#define SAFE_PLACES 2

#define TAG_SAFE_PLACE_DEMAND 3 //I need safe place


int wineAmount; // amount of wine made by me - my offer
int safePlaces; // number of places in critical section
int wmakersAfterMe; // after me in queue to critical section
bool demand = false; //do I need critical section
int acksLeft; // how many acks needed to enter critical section

void produceWine();
void meetStudent(int studentRank, int wineToGive);
void askForSafePlace();


