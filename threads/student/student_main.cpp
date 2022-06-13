#include "student_main.hpp"
#include <unistd.h>
#include <list>
#include <algorithm>

MPI_Datatype create_MPI_struct()
{
     /* create a type for struct msg_s */
    const int nitems=5;
    int blocklengths[5] = {STUDENTS, STUDENTS, WINEMAKERS, WINEMAKERS, STUDENTS};
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_C_BOOL};
    MPI_Datatype mpi_lider_msg;
    MPI_Aint offsets[5];

    offsets[0] = offsetof(msg_s, wineDemands);
    offsets[1] = offsetof(msg_s, goCounters);
    offsets[2] = offsetof(msg_s, wineOffers);
    offsets[3] = offsetof(msg_s, winemakersClocks);
    offsets[4] = offsetof(msg_s, freeStudents);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_lider_msg);
    MPI_Type_commit(&mpi_lider_msg);

    return mpi_lider_msg;
}

void sendToStudents(int* msg, int tag)
{
    log("Wysyłam wiadomość");
    for (int rank = OFFSET ; rank < OFFSET + STUDENTS; rank++){
        MPI_Send(&msg, 2, MPI_INT, rank, tag, MPI_COMM_WORLD);
    }
}


void determineDemand()
{
    srand(time(NULL) + tid);
    wineDemand = rand() % MAX_WINE +1;

    //send message to students
    log("Żądam " + std::to_string(wineDemand) + " wina");
    
    int msg [2] = {lClock, wineDemand};
	sendToStudents(msg, TAG_WINE_DEMAND);
}

void allocTables(int elemSize)
{
    wineDemands = (int*)calloc(STUDENTS, elemSize);
    offers = (int*)calloc(WINEMAKERS, elemSize);
    goCounters = (int*)calloc(STUDENTS, elemSize);
    winemakersClocks = (int*)calloc(WINEMAKERS, elemSize);
    freeStudents = (bool*)calloc(STUDENTS, sizeof(bool));
}

void freeTables()
{
    free(wineDemands);
    free(offers);
    free(goCounters);
    free(winemakersClocks);
    free(freeStudents);
}

void liderSection(int* offersCounter, int* demandsCounter, bool* freeStudents, int* wineOffers, int* winemakersClocks)
{
    if (offersCounter > 0 && demandsCounter > 0)
    {
        //Sortujemy wolnych studentów insertion sortem na podstawie goCounters i wineDemands
        std::list<int> studentsQ; 
        for (int i; i<STUDENTS; i++)
        {
            if (freeStudents[i] == 1)
            {
                auto pos = studentsQ.begin();
                while (goCounters[*pos] < goCounters[i]) pos++;
                while (goCounters[*pos] == goCounters[i] && wineDemands[*pos] < wineDemands[i]) pos++; //QUESTION: rosnąco czy malejąco?
                studentsQ.insert(pos, i);                   
            } 
        }

        //dopisz ofertę do kolejki winiarzy (wg. czasu, później oferty, później rangi)
        std::list<int> winemakersQ;
        for (int i; i<WINEMAKERS; i++)
        {
            if (wineOffers[i] != 0)
            {
                auto pos = winemakersQ.begin();
                while(winemakersClocks[*pos] < winemakersClocks[i]) pos++;
                while(winemakersClocks[*pos] == winemakersClocks[i] && wineOffers[*pos] > wineOffers[i]) pos++;
                winemakersQ.insert(pos, i);
            }
        }
        // w sumie można by nawet ich ładniej dopasować
        int my_winemaker = winemakersQ.front();
        winemakersQ.pop_front();

        std::cout<<"my_winemaker"<<my_winemaker<<std::endl;

        //wyślij studentom wiadomości do których winiarzy mają iść
        auto student = studentsQ.begin();
        auto winemaker = winemakersQ.begin();

        while (student != studentsQ.end() && winemaker != winemakersQ.end())
        {
            int msg[2];
            msg[1] = *winemaker;
            std::cout<<"winemaker"<<msg[1];
            MPI_Send(msg, 2, MPI_INT, studentsQ[i] + OFFSET, TAG_GO, MPI_COMM_WORLD);
            student++;
            winemaker++;
        }

        //przekaż pałeczkę następnemu studentowi w kolejce
        if (!studentsQ.empty())
        {
            struct msg_long msg_long;
            std::copy(wineDemands, wineDemands + STUDENTS, msg_long.wineDemands);
            std::copy(goCounters, goCounters + STUDENTS , msg_long.goCounters);
            std::copy(wineOffers, wineOffers + WINEMAKERS, msg_long.wineOffers);
            std::copy(winemakersClocks, winemakersClocks + WINEMAKERS, msg_long.winemakersClocks);
            std::copy(freeStudents, freeStudents + STUDENTS, msg_long.freeStudents);

            MPI_Send(&msg_long, 2, mpi_lider_msg, studentsQ.front() + OFFSET, TAG_GO, MPI_COMM_WORLD);
            amILider = false;
        }
        else
        {
            //TODO
        }
    }
}

void goForIt(int winemaker){
    int msg[2];
    MPI_Status status;

    MPI_Recv(msg, 2, MPI_INT, winemaker, TAG_FREE, MPI_COMM_WORLD, &status);
    offers[winemaker] = msg[1];

    wineDemand -= std::min(offers[winemaker], wineDemand);

    msg[0] = ++lClock;
    msg[1] = winemaker;

    //WARNING: tu można wysłać też ofertę i żądanie, żeby zaktualizować
    sendToStudents(msg, TAG_HOMEBASE);

    if (wineDemand == 0){

        //TODO: jak jestem liderem to wątek śpi
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
    int demandsCounter = 0;
    
	determineDemand();
    // TODO: ma czekać na żądania od wszystkich
	allocTables(sizeof(msg));

	while (1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		log("Mam wiadomość");

        if (status.MPI_TAG == TAG_BATON)
        {
            MPI_Recv(&msg_long, 1, mpi_lider_msg, MPI_ANY_SOURCE, TAG_BATON, MPI_COMM_WORLD, &status);
            amILider = true;

            //zaktualizuj żądania studentów i winiarzy
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

            std::copy(freeStudents, freeStudents+STUDENTS, msg_long.freeStudents);

            liderSection(offersCounter, demandsCounter, freeStudents);
            
        }
        else
        {
            MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            switch(status.MPI_TAG)
            {
                case TAG_WINE_DEMAND:
                    demandsCounter++;
                    
                    if (msg[0] >= goCounters[&status.MPI_SOURCE]){
                        wineDemands[status.MPI_SOURCE] = msg[1];
                    }

                    if (amILider && wineDemand == 0)
                    {
                        //TODO: send winemakers' offers list
                        MPI_Send(msg, 2, MPI_INT, status.MPI_SOURCE, TAG_BATON, MPI_COMM_WORLD);
                    }
                    break;

                case TAG_OFFER:
                    offers[status.MPI_SOURCE] = msg[1];
                    offersCounter++;
                    if (amILider)
                    {
                        liderSection(offersCounter, demandsCounter, freeStudents);
                    }
                    break; 

                case TAG_GO: //od lidera że mam pójść 
                    goForIt(msg[1]);
                    break;
                
                case TAG_HOMEBASE:
                    int student = status.MPI_source;
                    goCounters[student]++;

                    int wineTaken = min(wineDemands[student], wineOffers[winemaker]);
                    wineOffers[msg[1]] -= wineTaken; 
                    wineDemands[student] -= wineTaken;

                    if (wineOffers[msg[1]] == 0)
                        offersCounter--;
                    if (wineDemands[student] == 0)
                        demandsCounter--;
                    
                    freeStudents[i] = 1;
                    break;
            }
        }
	}
		
	MPI_Finalize(); // Musi być w każdym programie na końcu

    return 0;
}
