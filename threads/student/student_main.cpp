#include "student_main.hpp"
#include <sstream>

int* wineDemands;
int* goCounters;
int* winemakersClocks;
int* offers;
int wineDemand;
bool* freeStudents;
bool amILider;


void printTab(int* tab, int length, std::string caption)
{
	std::stringstream ss;
	for (int i=0; i<length; i++) ss<<tab[i]<<", ";
	debug("%s: %s", caption.c_str(), ss.str().c_str());
}


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
    for (int r = OFFSET ; r < OFFSET + STUDENTS; r++){
        MPI_Send(msg, 2, MPI_INT, r, tag, MPI_COMM_WORLD);
    }
}

void sendBatonMessage(MPI_Datatype mpi_lider_msg, int newLider)
{
    msg_s msg_long;
    std::copy(wineDemands, wineDemands + STUDENTS, msg_long.wineDemands);
    std::copy(goCounters, goCounters + STUDENTS , msg_long.goCounters);
    std::copy(offers, offers + WINEMAKERS, msg_long.wineOffers);
    std::copy(winemakersClocks, winemakersClocks + WINEMAKERS, msg_long.winemakersClocks);
    std::copy(freeStudents, freeStudents + STUDENTS, msg_long.freeStudents);

	amILider = false;
    
    MPI_Send(&msg_long, 1, mpi_lider_msg, newLider, TAG_BATON, MPI_COMM_WORLD);
}

int correctWine(int student, int winemaker)
{
//	debug("Przed transakcją. Student %d ma %d, a winiarz %d - %d", student, wineDemands[student], winemaker, offers[winemaker]);
	int wineTaken = std::min(wineDemands[student], offers[winemaker]);
    offers[winemaker] -= wineTaken; 
    wineDemands[student] -= wineTaken;
	return wineTaken;
}

void determineDemand()
{
    srand(time(NULL) + rank);
    wineDemand = rand() % MAX_WINE_STUDENT +1;
	debug("Chcę wypić %d wina", wineDemand);	

    //send message to students
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

void goForIt(int winemaker)
{
    debug("Poszedlem do %d", winemaker);
    int msg[2];
    MPI_Status status;

	//czeka na wiadomość od winiarza
    MPI_Recv(msg, 2, MPI_INT, winemaker, TAG_FREE, MPI_COMM_WORLD, &status);
	offers[winemaker] = msg[1];

    wineDemand -= std::min(offers[winemaker], wineDemand);

    msg[0] = ++lClock;
    msg[1] = winemaker;

    sendToStudents(msg, TAG_HOMEBASE);

    if (wineDemand == 0){

		debug("śpię");
        sleep(rand() % MAX_SLEEP);
        determineDemand();
    }
}

void liderSection(int* offersCounter, int* demandsCounter, bool* freeStudents, int* wineOffers, int* winemakersClocks, MPI_Datatype mpi_lider_msg)
{
    debug("Jestem w sekcji dla lidera");
	
	*demandsCounter = 0;
    for(int i = 0; i<STUDENTS; i++)
    {
		freeStudents[i] = 1;
		if(wineDemands[i] > 0) (*demandsCounter)++;
    }

	
	*offersCounter = 0;
	for (int i=0; i<WINEMAKERS; i++)
	{
		if(offers[i]>0) (*offersCounter)++;
    }
	
    if (*offersCounter > 0 && *demandsCounter > 0)
    {
        //Sortujemy wolnych studentów insertion sortem na podstawie goCounters i wineDemands
        std::list<int> studentsQ;
        int newLider = -1;
		for (int i=0; i<STUDENTS; i++)
        {
            if (freeStudents[i] == 1)
            {
                auto pos = studentsQ.begin();
                if (studentsQ.size() > 0)
                {
		        	while (goCounters[*pos] < goCounters[i] && pos != studentsQ.end()){
							 pos++;
						}
		        	while (goCounters[*pos] == goCounters[i] && wineDemands[*pos] < wineDemands[i] && pos != studentsQ.end())
					{	
					 	pos++; //QUESTION: rosnąco czy malejąco?
					}
				}
                studentsQ.insert(pos, i);
            }
			else if (newLider == -1) newLider = i;
        }
        

        //dopisz ofertę do kolejki winiarzy (wg. czasu, później oferty, później rangi)
        std::list<int> winemakersQ;

        for (int i=0; i<WINEMAKERS; i++)
        {
            if (wineOffers[i] != 0)
            {
                auto pos = winemakersQ.begin();
                if(winemakersQ.size() > 0)
                {
		        	while(winemakersClocks[*pos] < winemakersClocks[i] && pos != winemakersQ.end()) pos++;
		    	    while(winemakersClocks[*pos] == winemakersClocks[i] && wineOffers[*pos] > wineOffers[i] && pos != winemakersQ.end()) pos++;
				}
                winemakersQ.insert(pos, i);
            }
        }
        
        // w sumie można by nawet ich ładniej dopasować
        int myWinemaker = -1;

        //wyślij studentom wiadomości do których winiarzy mają iść
        auto student = studentsQ.begin();
        auto winemaker = winemakersQ.begin();
		int wineAmount;
		int msg[2];

        while (student != studentsQ.end() && winemaker != winemakersQ.end())
        {
			goCounters[*student] ++;
			wineAmount = correctWine(*student, *winemaker);
	    	
			if(*student + OFFSET == rank)
		    {
				myWinemaker = *winemaker;
	   		}
		    else
		    {
				msg[1] = *winemaker;
	    		MPI_Send(msg, 2, MPI_INT, *student + OFFSET, TAG_GO, MPI_COMM_WORLD);
			}
			msg[0] = *student + OFFSET;
			msg[1] = wineAmount; 
			MPI_Send(msg, 2, MPI_INT, *winemaker, TAG_MEETING, MPI_COMM_WORLD);
	   		    
        	freeStudents[*student] = 0;
	
			studentsQ.pop_front();
        	winemakersQ.pop_front();
			(*demandsCounter)--;
			(*offersCounter)--;           

      	    student = studentsQ.begin();
            winemaker = winemakersQ.begin();
        }

        //przekaż pałeczkę następnemu studentowi w kolejce, jeżeli go nie ma, pierwszemu wolnemu
        if (!studentsQ.empty())
        {
            newLider =  studentsQ.front();
        }

		if (newLider != -1) sendBatonMessage(mpi_lider_msg, newLider + OFFSET); //jakiś student nie idzie po wino
        if (myWinemaker != -1) goForIt(myWinemaker);
    }
}

int studentMain()
{
    debug("Dzien dobry, jestem studentem");
    MPI_Datatype mpi_lider_msg = create_MPI_struct();
    
    //wait for messages
    MPI_Status status;
    int msg [2];
    msg_s msg_long; 
    int offersCounter = 0;
    int demandsCounter = 0;
    
    determineDemand();
	allocTables(sizeof(msg));

    if(rank == OFFSET)
    {
    	amILider = true;
    	debug("Jestem liderem");
    }

	while (1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_BATON)
        {
            debug("Wiadomosc - zostalem liderem");
            MPI_Recv(&msg_long, 1, mpi_lider_msg, MPI_ANY_SOURCE, TAG_BATON, MPI_COMM_WORLD, &status);
            amILider = true;

            //zaktualizuj żądania studentów i winiarzy – WYGLĄDA OK
            for (int i=0; i<STUDENTS; i++)
            {
                if (msg_long.goCounters[i] > goCounters[i])
                {
                    wineDemands[i] = msg_long.wineDemands[i];
                    goCounters[i] = msg_long.goCounters[i];
					freeStudents[i] = msg_long.freeStudents[i];
				 }
            }

            for (int i=0; i<WINEMAKERS; i++)
            {
                if (msg_long.winemakersClocks[i] >= winemakersClocks[i])
                {
                    offers[i] = msg_long.wineOffers[i];
                    winemakersClocks[i] = msg_long.winemakersClocks[i];
                }
            }

			
			offersCounter = 0;
			for (int i=0; i<WINEMAKERS; i++)
			{
				if(offers[i]>0) offersCounter++;
			}			
		
			demandsCounter = 0;
			for (int i=0; i<STUDENTS; i++)
			{
				if(wineDemands[i]>0 && freeStudents[i]) demandsCounter++;
			}			


            liderSection(&offersCounter, &demandsCounter, freeStudents, offers, winemakersClocks, mpi_lider_msg);
            
        }
        else
        {
            MPI_Recv(msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            switch(status.MPI_TAG)
            {
                case TAG_WINE_DEMAND:
                    debug("Wiadomość - student %d żąda %d wina", status.MPI_SOURCE, msg[1]);
                    demandsCounter++;
                    
                    if (msg[0] >= goCounters[status.MPI_SOURCE-OFFSET])
                    {
                        wineDemands[status.MPI_SOURCE-OFFSET] = msg[1];
						goCounters[status.MPI_SOURCE-OFFSET] = msg[0];
                    	freeStudents[status.MPI_SOURCE-OFFSET] = 1;
                    }

                    if (amILider && demandsCounter == 1)
                    {
                        sendBatonMessage(mpi_lider_msg, status.MPI_SOURCE);
                    }

                    break;

                case TAG_OFFER:
                    debug("Wiadomość - winiarz %d oferuje %d wina", status.MPI_SOURCE, msg[1]);
                    winemakersClocks[status.MPI_SOURCE] = msg[0];
					offers[status.MPI_SOURCE] = msg[1];
                    offersCounter++;
                    if (amILider)
                    {
                        liderSection(&offersCounter, &demandsCounter, freeStudents, offers, winemakersClocks, mpi_lider_msg);
                    }
                    break; 

                case TAG_GO: //od lidera że mam pójść 
                    debug("Wiadomość - zielone swiatło, idę do %d", msg[1]);
                    goForIt(msg[1]);
                    break;
                
                case TAG_HOMEBASE:
                    debug("Wiadomość - student %d wrócił", status.MPI_SOURCE);
                    int student = status.MPI_SOURCE;

                    if (msg[0] > goCounters[student])
					{
						correctWine(student-OFFSET, msg[1]);
						goCounters[student] = msg[0];
					}
                    if (offers[msg[1]] > 0)
                        offersCounter++;
                    if (wineDemands[student] > 1)
                        demandsCounter++;
                    
                    freeStudents[student] = 1;
					if(amILider) liderSection(&offersCounter, &demandsCounter, freeStudents, offers, winemakersClocks, mpi_lider_msg);
                    break;
            }
        }
	}
    return 1;
}
