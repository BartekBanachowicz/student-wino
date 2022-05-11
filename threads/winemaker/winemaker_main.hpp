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
//QUESTION: jak dzielimy studentów i winiarzy? (na razie zrobiłam najpierw 5 procesów winiarzy, a pozostałe 3 - studentów)

#define TAG_DEMAND 3 //I need safe place
#define TAG_FREE 4 //I left safe place

int wineAmount; // amount of wine made by me - my offer
int safePlaces; // number of places in critical section
int wmakersAfterMe; // after me in queue to critical section
bool demand = false; //do I need critical section
int acksLeft; // how many acks needed to enter critical section

void produceWine();
void meetStudent(int studentRank, int wineToGive);
void askForSafePlace();


