#include "winemaker_main.hpp"

int wineAmount, wmakersAfterMe, acksLeft;
int safePlaces = SAFE_PLACES;
bool demand = false;
int studentToMeet;
int wineToGive;
int oldClock;

std::queue<std::array<int, 2>> studentsToServe;

void produceWine(){
    srand(time(NULL) + rank);
    wineAmount = rand() % MAX_WINE_WINEMAKER +1;

    //send message to students
    debug("Wyprodukowałem %d wina", wineAmount);
    
    int msg [2] = {++lClock, wineAmount};
    for (int currRank = OFFSET ; currRank < OFFSET + STUDENTS; currRank++){
        MPI_Send( msg, 2, MPI_INT, currRank, TAG_OFFER, MPI_COMM_WORLD);
    }
}

void serveStudent(int* msg, bool* activeMeeting){
	oldClock = lClock;
	lClock = std::max(msg[0], lClock) + 1;
				
	*activeMeeting = true;
	debug("Wiadomość – spotkanie z %d", msg[0]);
	bool res = askForSafePlace();
	studentToMeet = msg[0];
	wineToGive = msg[1];
	// if I dont need acks (many safe places or nobody will give me)
	if (res && safePlaces > 0){
		meetStudent(studentToMeet, wineToGive, activeMeeting);
	}
}

void meetStudent(int studentRank, int wineToGive, bool *activeMeeting){
	debug("Oddaję %d wina", wineToGive);

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
	if (studentsToServe.size()>0)
	{
		int newStudent[2] = {studentsToServe.front()[0], studentsToServe.front()[1]};
		studentsToServe.pop();
		serveStudent(newStudent, activeMeeting);
	}
}

bool askForSafePlace(){
	debug("Żądam miejsca");

	demand = true;
	
	acksLeft = std::max(WINEMAKERS - 1 - (SAFE_PLACES - safePlaces), 0); //excluding me and winemakers before me
	
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
	bool activeMeeting = false;

	while (1)
	{
    	//wait for messages
    	
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch(status.MPI_TAG){
			case TAG_MEETING: // student asks for meeting
				if(!activeMeeting)
				{
					serveStudent(msg, &activeMeeting);
				}
				else
				{
					//WARNING: czy na pewno kopiuje?	
					std::array<int, 2> temp;			
					std::copy(msg, msg+2, std::begin(temp));
					studentsToServe.push(temp);
				}

						
			break;
			
			case TAG_SAFE_PLACE_DEMAND:
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				// I don't want to enter or I have lower priority – aggree
				//if (msg[0] <= oldClock)
				//{
					if (!demand || 
						msg[0] < oldClock || 
						((msg[0] == oldClock) && msg[1] > wineAmount) || 
						((msg[0] == oldClock) && (msg[1] == wineAmount) && status.MPI_SOURCE > rank)){ 
						debug("Wiadomość – żądanie dostępu → odsyłam ACK");

						msg[0] = ++lClock;
						msg[1] = 89; 
						MPI_Send(msg, 2, MPI_INT, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
					
						safePlaces--;
						if(demand) acksLeft--;
					}
					else{
						// don't agree
						wmakersAfterMe++;
					}	
				break;
			
			case TAG_FREE:
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				debug("Wiadomość – ktoś zwolnił miejsce");
				safePlaces++;
				if (demand && acksLeft == 0 && safePlaces > 0){
					meetStudent(studentToMeet, wineToGive, &activeMeeting);
				}
				break;
			
			case TAG_ACK:
				oldClock = lClock;
				lClock = std::max(msg[0], lClock) + 1;
				if(demand){
					acksLeft--;
					if (acksLeft <= 0 && safePlaces > 0){
						meetStudent(studentToMeet, wineToGive, &activeMeeting);
					}		
				}
		}
	}
    return 0;
}
