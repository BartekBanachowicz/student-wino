//TODO: wczytywać jako parametr
const int MAX_WINE = 10;
const int STUDENTS = 3;
const int WINEMAKERS = 5;
const int SAFE_PLACES = 2;
//QUESTION: jak dzielimy studentów i winiarzy? (na razie zrobiłam najpierw 5 procesów winiarzy, a pozostałe 3 - studentów)

const int TAG_OFFER = 1; //I have wine
const int TAG_MEETING = 2; //Student wants my wine!
const int TAG_DEMAND = 3; //I need safe place
const int TAG_FREE = 4; //I left safe place
const int TAG_ACK = 5; //Aggrement to enter safe place

int tid;
int wineAmount; // amount of wine made by me - my offer
int safePlaces; // number of places in critical section
int wmakersAfterMe; // after me in queue to critical section
int lClock = 0; //lamport clock
bool demand = false; //do I need critical section
int acksLeft; // how many acks needed to enter critical section

void produceWine(int argc, char** argv);
void meetStudent(int studentRank, int wineToGive);
