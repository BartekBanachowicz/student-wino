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
    for (int currRank = WINEMAKERS ; currRank < WINEMAKERS + STUDENTS; currRank++){
        MPI_Send( msg, 2, MPI_INT, currRank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void meetStudent(int studentRank, int wineToGive, bool *activeMeeting){
	debug("Oddaję %d wina. SafePlaces %d", wineToGive, safePlaces-wmakersAfterMe);

	wineAmount -= wineToGive;
	int msg[2] = {++lClock, wineAmount}; //send how much wine left

	MPI_Send(msg, 2, MPI_INT, studentRank, TAG_FREE, MPI_COMM_WORLD);

	for (int currRank = 0; currRank < WINEMAKERS; currRank++)
	{
		if (currRank != rank)
			MPI_Send(msg, 2, MPI_INT, currRank, TAG_FREE, MPI_COMM_WORLD);
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
	
	*activeMeeting = false;
}

bool askForSafePlace(){
	debug("Żądam miejsca");

	demand = true;
	
	acksLeft = std::max(WINEMAKERS - 1 - (SAFE_PLACES - safePlaces), 0); //excluding me and winemakers before me
	debug("będę potrzebował %d acków", acksLeft);
	

	int msg[2] = {++lClock, wineAmount};
	
	for (int i = 0; i < WINEMAKERS; i++)
	{
		if (i != rank)
		{
			//WARNING: zwiększa koszt komunikacyjny - możliwa optymalizacja (niektórzy i tak nam nie odpiszą)
        	MPI_Send(msg, 2, MPI_INT, i, TAG_SAFE_PLACE_DEMAND, MPI_COMM_WORLD);
		}
    }
	if (acksLeft == 0)
		return 1;
	else 
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
	bool activeMeeting = false;

	while (1)
	{
    	//wait for messages
    	
    	MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		
	
		switch(status.MPI_TAG){
			case TAG_MEETING: // student asks for meeting
				if(!activeMeeting)
				{
					MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, TAG_MEETING, MPI_COMM_WORLD, &status);
					oldClock = lClock;
					lClock = std::max(msg[0], lClock) + 1;
					
					activeMeeting = true;
		
					debug("Wiadomość – spotkanie z %d", msg[0]);
					res = askForSafePlace();
					studentToMeet = msg[0];
					wineToGive = msg[1];
					// if I dont need acks (many safe places or nobody will give me)
					if (res && safePlaces > 0){
						meetStudent(studentToMeet, wineToGive, &activeMeeting);
					}
				}
						
			break;
			
			case TAG_SAFE_PLACE_DEMAND:
			
				MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, TAG_SAFE_PLACE_DEMAND, MPI_COMM_WORLD, &status);
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				// I don't want to enter or I have lower priority – aggree
				if (!demand || 
					msg[0] < oldClock || 
					((msg[0] == oldClock) && msg[1] > wineAmount) || 
					((msg[0] == oldClock) && (msg[1] == wineAmount) && status.MPI_SOURCE > rank)){ 
					debug("Wiadomość – ktoś chce miejsce – Zgadzam się; safeplaces: %d", safePlaces-1);
					msg[0] = ++lClock;
					msg[1] = 89; 
					MPI_Send(msg, 2, MPI_INT, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
					
					safePlaces--;
				}
				else{
					debug("Wiadomość – ktoś chce miejsce, ale mu nie dam");
					// don't agree
					wmakersAfterMe++;	
				}
				break;
			
			case TAG_FREE:
				MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, TAG_FREE, MPI_COMM_WORLD, &status);
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				debug("Wiadomość – ktoś zwolnił miejsce; safeplaces: %d", safePlaces+1);
				safePlaces++;
				if (demand && acksLeft == 0 && safePlaces > 0){
					meetStudent(studentToMeet, wineToGive, &activeMeeting);
				}
				break;
			
			case TAG_ACK:
				MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, TAG_ACK, MPI_COMM_WORLD, &status);
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				debug("Wiadomość – %d mi pozwala iść", status.MPI_SOURCE);
				if (!demand){
					std::cerr<<"[BŁĄD] Po co to ack? Nie żądałem\n";
				}
				else{
					acksLeft--;
					debug("Brakuje mi jeszcze %d ack", acksLeft);
					if (acksLeft <= 0 && safePlaces > 0){
						meetStudent(studentToMeet, wineToGive, &activeMeeting);
					}		
				}
		}
	}
    return 0;
}
