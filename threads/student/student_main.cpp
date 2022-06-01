#include "student_main.hpp"
#include <unistd.h>

MPI_Datatype create_MPI_struct(){
     /* create a type for struct msg_s */
    const int nitems=4;
    int blocklengths[4] = {STUDENTS, STUDENTS, WINEMAKERS, WINEMAKERS};
    MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Datatype mpi_lider_msg;
    MPI_Aint offsets[4];

    offsets[0] = offsetof(msg_s, wineDemands);
    offsets[1] = offsetof(msg_s, goCounters);
    offsets[2] = offsetof(msg_s, wineOffers);
    offsets[3] = offsetof(msg_s, winemakersClocks);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_lider_msg);
    MPI_Type_commit(&mpi_lider_msg);

    return mpi_lider_msg;
}

void sendToStudents(int* msg, int tag){
    log("Wysyłam wiadomość");
    for (int rank = WINEMAKERS ; rank < WINEMAKERS + STUDENTS; rank++){
        MPI_Send(&msg, 2, MPI_INT, rank, tag, MPI_COMM_WORLD);
    }
}


void determineDemand(){
    srand(time(NULL) + tid);
    wineDemand = rand() % MAX_WINE +1;

    //send message to students
    log("Żądam " + std::to_string(wineDemand) + " wina");
    
    int msg [2] = {++lClock, wineDemand};
	sendToStudents(msg, TAG_WINE_DEMAND);
}


void allocTables(int elemSize){
    wineDemands = (int*)malloc(STUDENTS * elemSize);
    offers = (int*)malloc(WINEMAKERS * elemSize);
    goCounters = (int*)malloc(STUDENTS * elemSize);
    winemakersClocks = (int*)malloc(WINEMAKERS * elemSize);
}

void freeTables(){
    free(wineDemands);
    free(offers);
    free(goCounters);
    free(winemakersClocks);
}
/*
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
*/
void goForIt(int winemaker){
    int msg[2];
    MPI_Status status;

    MPI_Recv(msg, 2, MPI_INT, winemaker, TAG_FREE, MPI_COMM_WORLD, &status);
    offers[winemaker] = msg[1];

    wineDemand -= std::min(offers[winemaker], wineDemand);

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
    MPI_Datatype mpi_lider_msg = create_MPI_struct();
    
   	log("Student");

    //wait for messages
    MPI_Status status;
    int msg [2];
    msg_s msg_long; 
	int oldClock;
    int offersCounter = 0;
    
	determineDemand();
	allocTables(sizeof(msg));

	while (1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		log("Mam wiadomość");

        if (status.MPI_TAG == TAG_BATON)
        {
            MPI_Recv(&msg_long, 1, mpi_lider_msg, MPI_ANY_SOURCE, TAG_BATON, MPI_COMM_WORLD, &status);
            amILider = True;
            for (int i=0; i<STUDENTS; i++)
            {
                if (msg_long.goCounters[i] > goCounters[i])
                {
                    wineDemands[i] = msg_long.wineDemands[i];
                    goCounters[i] = msg_long.goCounters[i];
                }
            }

            for (int i=0; i<WINEMAKERS; i++)
            {
                if (msg_long.winemakersClocks[i] > winemakersClocks[i])
                {
                    offers[i] = msg_long.wineOffers[i];
                    winemakersClocks[i] = msg_long.winemakersClocks[i];
                }
            }

            //TODO: jak zegar to zaktualizować
        }
        else
        {
            MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            switch(status.MPI_TAG)
            {
                case TAG_WINE_DEMAND:
                    
                    if (msg[0] >= goCounters[&status.MPI_SOURCE]){
                        wineDemands[status.MPI_SOURCE] = msg[1];
                    }
                    goCounters = std::max(msg[0], lClock) + 1; //TODO: w sumie niepotrzebny



                    if (amILider && wineDemand == 0)
                    {
                        //TODO: send winemakers' offers list
                        MPI_Send(msg, 2, MPI_INT, status.MPI_SOURCE, TAG_BATON, MPI_COMM_WORLD);
                    }
                    break;

                case TAG_OFFER:
                    offers[status.MPI_SOURCE] = msg[1];
                    offersCounter++;
                    if (amILider && offersCounter)
                    {
                        goForIt(status.MPI_SOURCE);
                    }
                    break; 

                case TAG_GO:
                    goForIt(msg[1]);
                    break;
            }
        }
	}
		
	MPI_Finalize(); // Musi być w każdym programie na końcu

    return 0;
}
