#include "student_main.hpp"

void sendToStudents(int* msg, int tag){
    log("Wysyłam wiadomość");
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send( &msg, 2, MPI_INT, rank, tag, MPI_COMM_WORLD);
    }
}


void determineDemand(){
    srand(time(NULL) + tid);
    demand = rand() % MAX_WINE +1;

    //send message to students
    log("Żądam " + std::to_string(wineAmount) + " wina");
    
    int msg [2] = {++lClock, wineAmount};
	sendToStudents(msg, TAG_WINE_DEMAND);
}


void allocTables(int elemSize){
    wineDemands = malloc(STUDENTS * elemSize);
    offers = malloc(WINEMAKERS * elemSize);
}

void freeTables(){
    free(wineDemands);
    free(offers);
}

// void meetStudent(int studentRank, int wineToGive){
// 	log("Oddaję "+ std::to_string(wineToGive)+"wina");

// 	wineAmount -= wineToGive;
// 	int msg[2] = {++lClock, wineAmount}; //send how much wine left

// 	MPI_Send(msg, 2, MPI_INT, studentRank, TAG_FREE, MPI_COMM_WORLD);

// 	for (int rank = 0; rank < WINEMAKERS; rank++){
// 		MPI_Send(msg, 2, MPI_INT, rank, TAG_FREE, MPI_COMM_WORLD);
// 	}	
// 	demand = false;

// 	safePlaces -= wmakersAfterMe;
// 	wmakersAfterMe = 0;
	
// 	if (wineAmount <= 0){
// 		srand(time(NULL));
// 		int sleepTime = rand() % MAX_SLEEP;
// 		log("Śpię " + std::to_string(sleepTime));
// 		sleep(sleepTime);

// 		produceWine();
// 	}

// }

void goForIt(int winemaker){
    MPI_Recv(msg, 2, MPI_INT, winemaker, TAG_FREE, MPI_COMM_WORLD, &status);
    offers[winemaker] = msg[1];

    wineDemand -= min(offers[winemaker], wineDemand);

    msg[0] = ++lClock;
    //TODO: send nr winiarza + oferta + nasze 
    sendToStudents(msg, TAG_HOMEBASE);

    if (wineDemand == 0){

        //TODO: co jak jestem liderem?
        sleep(rand() % MAX_SLEEP);
        determineDemand();
        //TODO: śpij
    }
}

int main(int argc, char** argv){

    MPI_Init(&argc, &argv); 
	MPI_Comm_rank( MPI_COMM_WORLD, &tid);
    
   	log("Student");

    //wait for messages
    MPI_Status status;
    int msg [2];
	int oldClock;
    offersCounter = 0;
    
	determineDemand();
	allocTables(sizeof(msg));

	while (1){
		MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		log("Mam wiadomość");

		// oldClock = lClock;
		lClock = std::max(msg[0], lClock) + 1;

		switch(status.MPI_TAG){
            case TAG_WINE_DEMAND:
                wineDemands[status.MPI_SOURCE] = msg;
                if (amILider && wineDemand == 0){
                    //TODO: send winemakers' offers list
                    msg[0] = ++lClock;
                    MPI_send(msg, 2, MPI_INT, status.MPI_SOURCE, TAG_BATON, MPI_COMM_WORLD)
                }
                break;

            case TAG_OFFER:
                offers[status.MPI_SOURCE] = msg;
                offersCounter++;
                if (amILider && offersCounter){
                    goForIt(status.MPI_SOURCE);
                }
                break; 

            case TAG_GO:
                goForIt(msg[1]);

                break;
        }

        

	}
		
	MPI_Finalize(); // Musi być w każdym programie na końcu

    return 0;
}
