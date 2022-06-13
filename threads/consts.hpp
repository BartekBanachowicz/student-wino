#pragma once

#include <iostream>
#include <mpi.h>

#define DE

#define STUDENTS 3
#define OFFSET WINEMAKERS
#define WINEMAKERS 5

#define TAG_OFFER 1 //I have wine
#define TAG_MEETING 2 //Student wants my wine!
#define TAG_FREE 4 //I left safe place, student is free
#define TAG_ACK 5 //Aggrement to enter safe place

extern int tid;
extern int lClock; //lamport clock


//in msg first value is lamport clock, next is message
//void log(std::string msg){
//    std::cout<<"[LOG] "<<tid<<" "<<lClock<<" "<<msg<<std::endl;
//}

//
//#ifdef DE
//#define debug(FORMAT,...) printf("%c[%d;%dm [%d %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(tid/7))%2, 31+(6+tid)%7, tid, lClock, ##__VA_ARGS__, 27,0,37);
//#else
//#define debug(...) ;
//#endif
//
//#define P_WHITE printf("%c[%d;%dm",27,1,37);
//#define P_BLACK printf("%c[%d;%dm",27,1,30);
//#define P_RED printf("%c[%d;%dm",27,1,31);
//#define P_GREEN printf("%c[%d;%dm",27,1,33);
//#define P_BLUE printf("%c[%d;%dm",27,1,34);
//#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
//#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
//#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
//#define P_CLR printf("%c[%d;%dm",27,0,37);
//
///* printf ale z kolorkami i automatycznym wyświetlaniem tid. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
//#define println(FORMAT, ...) printf("%c[%d;%dm [%d %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(tid/7))%2, 31+(6+tid)%7, tid, lamport_clock, ##__VA_ARGS__, 27,0,37);
