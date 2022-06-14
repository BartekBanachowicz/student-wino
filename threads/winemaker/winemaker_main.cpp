#include "winemaker_main.hpp"

int wineAmount, wmakersAfterMe, acksLeft;
int safePlaces = SAFE_PLACES;
bool demand = false;


void produceWine(){
    srand(time(NULL) + rank);
    wineAmount = rand() % MAX_WINE_WINEMAKER +1;

    //send message to students
    debug("Wyprodukowałem %d wina", wineAmount);
    
    int msg [2] = {++lClock, wineAmount};
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send( msg, 2, MPI_INT, rank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void meetStudent(int studentRank, int wineToGive){
	debug("Oddaję %d wina", wineToGive);

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
		debug("Śpię %d", sleepTime);
		sleep(sleepTime);

		produceWine();
	}

}

bool askForSafePlace(){
	debug("Żądam miejsca");

	demand = true;
	
	acksLeft = std::max(WINEMAKERS - 1 - (SAFE_PLACES - safePlaces), 0); //excluding me and winemakers before me
	debug("będę potrzebował %d acków", acksLeft);
	
	if (acksLeft == 0)
		return 1;

	int msg[2] = {++lClock, wineAmount};
	
	for (int i = 0; i < WINEMAKERS; i++)
	{
		if (i != rank)
		{
			//WARNING: zwiększa koszt komunikacyjny - możliwa optymalizacja (niektórzy i tak nam nie odpiszą)
        	MPI_Send(msg, 2, MPI_INT, i, TAG_SAFE_PLACE_DEMAND, MPI_COMM_WORLD);
		}
    }
	return 0;
}

int winemakerMain()
{
    debug("Dzień dobry, jestem winiarzem");
    produceWine();
	
    MPI_Status status;
    int msg [2];
	int studentToMeet;
	int wineToGive;
	int oldClock;
	bool res;

	while (1){
    	//wait for messages
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		oldClock = lClock;
		lClock = std::max(msg[0], lClock) + 1;
	
		switch(status.MPI_TAG){
			case TAG_MEETING: // student asks for meeting
				debug("Wiadomość – spotkanie z %d", msg[0]);
				res = askForSafePlace();
				studentToMeet = msg[0];
				wineToGive = msg[1];
				// if I dont need acks (many safe places or nobody will give me)
				if (res && safePlaces > 0){
					meetStudent(studentToMeet, wineToGive);
				}		
			break;
			
			case TAG_SAFE_PLACE_DEMAND:
				// I don't want to enter or I have lower priority – aggree
				if (!demand || 
					msg[0] < oldClock || 
					((msg[0] == oldClock) && msg[1] > wineAmount) || 
					((msg[0] == oldClock) && (msg[1] == wineAmount) && status.MPI_SOURCE > rank)){ 
					debug("Wiadomość – ktoś chce miejsce – Zgadzam się");
					msg[0] = ++lClock;
					msg[1] = 89; 
					MPI_Send(msg, 2, MPI_CHAR, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
					
					safePlaces--;
				}
				else{
					debug("Wiadomość – ktoś chce miejsce, ale mu nie dam");
					// don't agree
					wmakersAfterMe++;	
				}
				break;
			
			case TAG_FREE:
				debug("Wiadomość – ktoś zwolnił miejsce");
				safePlaces++;
				if (demand && acksLeft == 0 && safePlaces > 0){
					meetStudent(studentToMeet, wineToGive);
				}
				break;
			
			case TAG_ACK:
				debug("Wiadomość – %d mi pozwala iść", status.MPI_SOURCE);
				if (!demand){
					std::cerr<<"[BŁĄD] Po co to ack? Nie żądałem\n";
				}
				else{
					acksLeft--;
					debug("Brakuje mi jeszcze %d ack", acksLeft);
					if (acksLeft <= 0 && safePlaces > 0){
						meetStudent(studentToMeet, wineToGive);
					}		
				}
		}
	}
    return 0;
}
