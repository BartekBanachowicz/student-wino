#include "winemaker_main.hpp"

void produceWine(){
    srand(time(NULL) + tid);
    wineAmount = rand() % MAX_WINE +1;

    //send message to students
    log("Wyprodukowałem " + std::to_string(wineAmount) + " wina");
    
    //IDEA: można to wysłać strukturą, ale trzeba stworzyć mpi-owy typ https://stackoverflow.com/questions/9864510/struct-serialization-in-c-and-transfer-over-mpi
    int msg [2] = {++lClock, wineAmount};
	log("Wysyłam wiadomość z ofertą");
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send( msg, 2, MPI_INT, rank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void meetStudent(int studentRank, int wineToGive){
	log("Oddaję "+ std::to_string(wineToGive)+"wina");

	wineAmount -= wineToGive;
	int msg[2] = {++lClock, wineAmount}; //send how much wine left

	MPI_Send(msg, 2, MPI_INT, studentRank, TAG_FREE, MPI_COMM_WORLD);

	for (int rank = 0; rank < WINEMAKERS; rank++){
		MPI_Send(msg, 2, MPI_INT, rank, TAG_FREE, MPI_COMM_WORLD);
	}	
	demand = false;

	safePlaces -= wmakersAfterMe;
	wmakersAfterMe = 0;
	
	if (wineAmount <= 0){
		srand(time(NULL));
		int sleepTime = rand() % MAX_SLEEP;
		log("Śpię " + std::to_string(sleepTime));
		sleep(sleepTime);

		produceWine();
	}

}

void askForSafePlace(){
	log("Żądam miejsca");

	demand = true;
	acksLeft = WINEMAKERS - 1 - (SAFE_PLACES - safePlaces); //excluding me and winemakers before me

	int msg[2] = {++lClock, wineAmount};
	
	for (int rank = 0; rank < WINEMAKERS; rank++){
		if (rank != tid){
			//TODO: niektórzy nam nie odpowiedzą, więc po co wysyłać
        	MPI_Send(msg, 2, MPI_INT, rank, TAG_DEMAND, MPI_COMM_WORLD);
		}
    }
}

int main(int argc, char** argv){

    MPI_Init(&argc, &argv); 
	MPI_Comm_rank( MPI_COMM_WORLD, &tid);
    
   	log("Winiarz");

	produceWine();
	
    //wait for messages
    MPI_Status status;
    int msg [2];
	int studentToMeet;
	int wineToGive;
	int oldClock;

	while (1){
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		log("Mam wiadomość");

		oldClock = lClock;
		lClock = std::max(msg[0], lClock) + 1;

		switch(status.MPI_TAG){
			case TAG_MEETING:
				askForSafePlace();
				studentToMeet = status.MPI_SOURCE;
				wineToGive = msg[1];
				break;
			
			case TAG_DEMAND:
				if (!demand || 
					msg[0] < oldClock || 
					((msg[0] == oldClock) && msg[1] > wineAmount) || 
					((msg[0] == oldClock) && (msg[1] == wineAmount) && status.MPI_SOURCE > tid)){ 

					msg[0] = ++lClock;
					msg[1] = 89; 
					MPI_Send(msg, 2, MPI_CHAR, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
					
					safePlaces--;
				}
				else{
					wmakersAfterMe++;	
				}
				break;
			
			case TAG_FREE:
				safePlaces++;
				if (demand && acksLeft == 0 && safePlaces > 0){
					meetStudent(studentToMeet, wineToGive);
				}
				break;
			
			case TAG_ACK:
				if (!demand){
					std::cerr<<"[BŁĄD] Po co to ack? Nie żądałem\n";
				}
				else{
					acksLeft--;
					if (acksLeft == 0 && safePlaces > 0){
						meetStudent(studentToMeet, wineToGive);
					}		
				}
		}
	}
		
	MPI_Finalize(); // Musi być w każdym programie na końcu

    return 0;
}