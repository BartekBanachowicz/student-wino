# student-wino

## Algorytm

### Winiarz:
int ilość_wina = 0; </br>
int dostępnych_bezpiecznych_miejsc = b;</br>
int ilość_winiarzy_za_mną_w_kolejce = 0;</br>
int zegar = 0;

- produkuje X wina 
- wysyła wiadomość do wszystkich studentów (i zwiększ zegar o 1)
- czeka na wiadomości:
  - dostaje informację od studenta s o rezerwacji (aktualizuje zegar)
    - wysyła komunikat o żądaniu dostępu do bezpiecznego miejsca do innych winiarzy (zwiększa zegar o 1)
    - oczekuje na ACK od wszystkich, którzy wcześniej nie żądali dostępu (zwiększa zegar)
    - zajmuje bezpieczne miejsce jeżeli licznik jest większy od 0:
      - oddaje wino
      - wysyła wiadomość o zakończeniu transakcji do studenta s i winiarzy (i zwiększa zegar)
    - odejmuje od ilości dostępnych miejsc liczbę winiarzy za nim w kolejce, zeruje liczbę winiarzy za nim
    - jeżeli zostało mu wino, wraca do czekania na wiadomości, w przeciwnym przypadku do początku
  - informacja od winiarza o żądaniu dostępu do bezpiecznego miejsca:
    - jeżeli nie żąda albo ma gorszy priorytet (lamport), odsyła ack (zwiększa zegar) i dekrementuje licznik bezpiecznych miejsc
    - jeżeli żądał, inkrementuje licznik winiarzy za nim
  - informacja od winiarza o zwolnieniu bezpiecznego miejsca
    - inkrementuje licznik bezpiecznych miejsc


### Student

int zegar = 0; </br>
int żądania_studentów[S-1]; </br>
int moje_żądanie = 0; </br>
int oferty_winiarzy[W]; </br>

na początku liderem jest student o rank = 1

- moje_żądanie = Y
- wyślij je do wszystkich studentów (zegar++)
- czekaj na żądania od wszystkich studentów:
  - wpisz je do swojej kolejki (wg. czasu, później żądania, później rangi)
- czekaj na wiadomości (zaktualizuj zegar):
  - wiadomość od studenta o żądaniu:
    - wpisz je do swojej kolejki (wg. czasu, później żądania, później rangi)
    - jeżeli nie masz żądania a jesteś liderem, wyślij wiadomość (z tablicą winiarzy) do tego studenta, że jest nowym liderem
  - wiadomość od winiarza w, że oferuje X wina:
    - dopisz ofertę do kolejki winiarzy (wg. czasu, później oferty, później rangi)
    - jeżeli to jedyna oferta i jestem liderem – przejdź do sekcji **dla lidera**
  - wiadomość od lidera, że mam pójść do winiarza w:
    - idź po wino -- zmniejsz swoje żądanie i ofertę winiarza o minimum z tych dwóch
    - czekaj na wiadomość od winiarza o zakończeniu transakcji
    - wyślij wiadomość o powrocie do wszystkich studentów (z numerem winiarza)(zegar++)
    - jeżeli moje żądanie = 0:
      - idź spać
      - wyprodukuj nowe żądanie i wyślij je do wszystkich studentów
    - wiadomość od lidera, że jestem nowym liderem:
      - zaktualizuj swoją kolejkę studentów i winiarzy
      - przejdź do sekcji **dla lidera**
    - wiadomość od studenta, że wrócił z wyprawy po wino od winiarza w:
      - zaktualizuj żądanie studenta i ofertę winiarza w

**dla lidera:**
- jeżeli mam przynajmniej jedną ofertę od winiarzy i kolejka studentów nie jest pusta:
  - wyznacz (wg. swojej kolejki studentów) tylu studentów (łącznie ze sobą) ilu jest wolnych winiarzy (lub studentów, jeżeli jest ich mniej)
    - wyślij im wiadomości, do których winiarzy mają iść
    - wyślij winiarzom prośby o zarezerwowanie miejsca
  - kolejnego studenta z kolejki wyznacz na lidera
    - wyślij mu swoje kolejki studentów i winiarzy, i informację ilu studentów poszło po wino
  - jeżeli nie ma nowego lidera, wyznacz siebie na lidera
  - idź po wino 
  - czekaj na wiadomość od winiarza o zakończeniu transakcji
  -wyślij wiadomość o powrocie do wszystkich studentów

---

© Bartłomiej Banachowicz & Anna Panfil
 
