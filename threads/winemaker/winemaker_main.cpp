#include "winemaker_main.hpp"

int wineAmount, safePlaces, wmakersAfterMe, acksLeft;
bool demand = false;


void produceWine(){
    srand(time(NULL) + rank);
    wineAmount = rand() % MAX_WINE_WINEMAKER +1;

    //send message to students
    //log("Wyprodukowałem " + std::to_string(wineAmount) + " wina");
    
    int msg [2] = {++lClock, wineAmount};
	//log("Wysyłam wiadomość z ofertą");
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send( msg, 2, MPI_INT, rank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void meetStudent(int studentRank, int wineToGive){
	//log("Oddaję "+ std::to_string(wineToGive)+"wina");

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
		//log("Śpię " + std::to_string(sleepTime));
		sleep(sleepTime);

		produceWine();
	}

}

void askForSafePlace(){
	//log("Żądam miejsca");

	demand = true;
	acksLeft = WINEMAKERS - 1 - (SAFE_PLACES - safePlaces); //excluding me and winemakers before me

	int msg[2] = {++lClock, wineAmount};
	
	for (int i = 0; i < WINEMAKERS; i++)
	{
		if (i != rank)
		{
			//WARNING: zwiększa koszt komunikacyjny - możliwa optymalizacja (niektórzy i tak nam nie odpiszą)
        	MPI_Send(msg, 2, MPI_INT, i, TAG_SAFE_PLACE_DEMAND, MPI_COMM_WORLD);
		}
    }
}

int winemakerMain()
{
   	//log("Winiarz");
    debug("Dzien dobry, jestem winiarzem");
    produceWine();
    debug("Wyprodukowalem %d wina", wineAmount);
	
    //wait for messages
    MPI_Status status;
    int msg [2];
	int studentToMeet;
	int wineToGive;
	int oldClock;

	while (1){
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		debug("Dostalem wiadomosc");
		//log("Mam wiadomość");

		oldClock = lClock;
		lClock = std::max(msg[0], lClock) + 1;

		switch(status.MPI_TAG){
			case TAG_MEETING:
				askForSafePlace();
				studentToMeet = status.MPI_SOURCE;
				wineToGive = msg[1];
				break;
			
			case TAG_SAFE_PLACE_DEMAND:
				if (!demand || 
					msg[0] < oldClock || 
					((msg[0] == oldClock) && msg[1] > wineAmount) || 
					((msg[0] == oldClock) && (msg[1] == wineAmount) && status.MPI_SOURCE > rank)){ 

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
    return 0;
}
