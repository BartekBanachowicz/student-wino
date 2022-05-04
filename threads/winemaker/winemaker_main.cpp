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
}

int main(int argc, char** argv){
    std::cout<<"Winiarz\n";
    produceWine(argc, argv);
	
    //wait for messages
    //TODO: w pętli
    MPI_Status status;
    int msg [2];

    MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        std::cout<<"Mam wiadomość\n";
        lClock++;
		switch(status.MPI_TAG){
			case TAG_MEETING:
                meetStudent(status.MPI_SOURCE);  
                break;
            case TAG_DEMAND:
                //TODO
                break;
            case TAG_FREE:
                //TODO
                break;

		}

	MPI_Finalize(); // Musi być w każdym programie na końcu



    return 0;
}