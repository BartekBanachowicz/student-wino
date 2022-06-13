#pragma once
#include "../consts.hpp"
#include <unistd.h>
#include <list>
#include <algorithm>

extern int* wineDemands;
extern int* goCounters;
extern int wineDemand;
extern int* winemakersClocks;
extern int* offers;
extern bool* freeStudents;
extern bool amILider;

#define TAG_WINE_DEMAND 6
#define TAG_BATON 7
#define TAG_GO 8
#define TAG_HOMEBASE 9

#define MAX_WINE_STUDENT 6

typedef struct{
    int wineDemands [STUDENTS] ;
    int goCounters [STUDENTS];
    int wineOffers [WINEMAKERS];
    int winemakersClocks [WINEMAKERS];
    bool freeStudents[STUDENTS];
}msg_s;

MPI_Datatype create_MPI_struct();
void sendToStudents(int* msg, int tag);
void determineDemand();
void allocTables(int elemSize);
void freeTables();
void goForIt(int winemaker);
void liderSection(int offersCounter, int demandsCounter, bool* freeStudents, int* wineOffers, int* winemakersClocks, MPI_Datatype mpi_lider_msg);
int studentMain();



