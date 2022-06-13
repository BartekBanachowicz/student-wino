#pragma once
#include "student_main.hpp"
#include "winemaker_main.hpp"

//do rozwiązywania problemów
//-mca btl tcp
//-mca pml ^ucx

//pthread conf wait, signal

//QUESTION: jak dzielimy studentów i winiarzy? (na razie zrobiłam najpierw 5 procesów winiarzy, a pozostałe 3 - studentów)
//IDEA: można to wysłać strukturą, ale trzeba stworzyć mpi-owy typ https://stackoverflow.com/questions/9864510/struct-serialization-in-c-and-transfer-over-mpi
