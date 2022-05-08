#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>
#include "winemaker_main.hpp"

void produceWine(int argc, char** argv){
    srand(time(NULL));
    wineAmount = rand() % MAX_WINE +1;

    //send message to students
    
    MPI_Init(&argc, &argv); 
    int tid;
	MPI_Comm_rank( MPI_COMM_WORLD, &tid);

    std::cout<<tid<<" "<<lClock<<" -- Wyprodukowałem "<<wineAmount<<" wina\n";
    
    //IDEA: można to wysłać strukturą, ale trzeba stworzyć mpi-owy typ https://stackoverflow.com/questions/9864510/struct-serialization-in-c-and-transfer-over-mpi
    int msg [2] = {++lClock, wineAmount};
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send( msg, 2, MPI_INT, rank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void meetStudent(int studentRank){
    //TODO
	demand = false;
}

void askForSafePlace(){
	demand = true;
	acksLeft = WINEMAKERS; //skoro wyśle do siebie, otrzyma od siebie ack
	for (int rank = 0; rank < WINEMAKERS; rank++){
		//NOTE: wysyła też do siebie
        MPI_Send(nullptr, 0, MPI_INT, rank, TAG_OFFER, MPI_COMM_WORLD);
    }
	
}

int main(int argc, char** argv){
    std::cout<<"Winiarz\n";
    produceWine(argc, argv);
	
    //wait for messages
    //TODO: w pętli
    MPI_Status status;
    int msg [2];
	int studentToMeet;

	MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	std::cout<<"Mam wiadomość\n";
	lClock++;
	switch(status.MPI_TAG){
		case TAG_MEETING:
			askForSafePlace();
			studentToMeet = status.MPI_SOURCE;
			break;
		
		case TAG_DEMAND:
			if (!demand){ //TODO: gorszy priorytet
				char ack = 'y';
				MPI_Send(&ack, 1, MPI_CHAR, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
				safePlaces--;
			}
			else{
				wmakersAfterMe++;	
			}
			break;
		
		case TAG_FREE:
			safePlaces++;
			if (demand && acksLeft == 0 && safePlaces > 0){
				meetStudent(studentToMeet);
			}
			break;
		
		case TAG_ACK:
			if (!demand){
				std::cout<<"BŁĄD! Po co to ack? Nie żądałem\n";
			}
			else{
				acksLeft--;
				if (acksLeft == 0 && safePlaces > 0){
					meetStudent(studentToMeet);
				}		
}
	}
		
	MPI_Finalize(); // Musi być w każdym programie na końcu

    return 0;
}
