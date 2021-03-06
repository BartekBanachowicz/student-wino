#include "main_thread.hpp"

/* sem_init sem_destroy sem_post sem_wait */
//#include <semaphore.h>
/* flagi dla open */
//#include <fcntl.h>

int rank;
int lClock = 0;

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE:
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
            fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
            MPI_Finalize();
            exit(-1);
            break;
        case MPI_THREAD_FUNNELED:
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
            break;
        case MPI_THREAD_SERIALIZED:
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
            break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
            break;
        default: printf("Nikt nic nie wie\n");
    }
}

void inicjuj(int argc, char **argv)
{
    // int provided;
    MPI_Init(&argc, &argv); 
    // MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);

    //std::cout<<"Tutaj"<<std::endl;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //log("jestem");
}

/* usunięcie zamkków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    /* Czekamy, aż wątek potomny się zakończy */
//    println("czekam na wątek \"komunikacyjny\"\n" );
//    pthread_join(threadKom,NULL);
//    if (rank==0) pthread_join(threadMon,NULL);
//    MPI_Type_free(&MPI_PAKIET_T); //TODO: usunąć tak u studenta
    MPI_Finalize();
}

/*int main(int argc, char **argv)
{
    #include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
 */
/**
 * @brief Illustrates how to initialise the MPI environment.
 **/
int main(int argc, char* argv[])
{
    // Initilialise MPI and check its completion
    // MPI_Init(&argc, &argv);
    inicjuj(argc, argv);
 
    // Get my rank
    //int my_rank;
    //MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
   // debug("Pomyslnie stworzono proces: %d", rank);
    
    if (rank < WINEMAKERS)
    {
        winemakerMain();
        
    } else
    {
    	studentMain();
    }
    // Tell MPI to shut down.
    MPI_Finalize();
 
    return EXIT_SUCCESS;
}
	
    /*std::cout<<"Ja tu nic nie robię\n";
    MPI_Init(&argc,&argv);
    std::cout<<"Here"<<std::endl; 
    //inicjuj(argc, argv);

    if (rank < WINEMAKERS)
    {
        winemakerMain();
    }
    else
    {
        studentMain();
    }

    finalizuj();
    return 0;
}*/

