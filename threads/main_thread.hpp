#pragma once

#include <iostream>
#include <mpi.h>

#define STUDENTS 3
#define WINEMAKERS 5

#define TAG_OFFER 1 //I have wine
#define TAG_MEETING 2 //Student wants my wine!
#define TAG_ACK 5 //Aggrement to enter safe place

int tid;
int lClock = 0; //lamport clock


//in msg first value is lamport clock, next is message
void log(std::string msg){
	std::cout<<"[LOG] "<<tid<<" "<<lClock<<" "<<msg<<std::endl;
}

//do rozwiązywania problemów
//-mca btl tcp
//-mca pml ^ucx