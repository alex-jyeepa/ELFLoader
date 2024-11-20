#Readme Implementare E.L.F Loader

Implementarea temei a durat 8 ore.

In functia de Init, se salveaza o copie a default handler-ului default
si in cea de executieeste initializat un vector pentru a tine
cont de paginile deja mapate. In handler, este verificat daca semnalul
primit este unul ce necesita handler-ul default sau cel nou. Se itereaza
prin segmente pana este gasit cel ce contine page-fault-ul cu adresa
in info->si_addr. Este calculata pozitia paginii in segment, este mapata
pagina si este trecuta in vector vectorul de pagini vizitate. Daca 
fault-ul se afla intre inceputul segmentului si adresa inceputului +
file_size, se citesc datele din fisier la adresa index a paginii mapate.
Daca page fault-ul se afla chiar inainte de portiunea ce trebuie zeroizata
se citeste din fisier doar o portiune de pagina. In orice caz, se seteaza
permisiunile pe un page_size cu mprotect. Daca fault-ul se afla in zona
zeroizata se apeleaza doar mprotect, zona fiind deja zeroizata de mmap.
Daca aceasta pagina a fost vizitata se apeleaza handler-ul default.