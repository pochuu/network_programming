# Dns-spoof
DNS spoof, typu man in the middle attack, po wykonaniu arp spoofingu przechwytuje pakiety lib pcap'em z filtrem na "dst port 53", a następnie tworzy DNS response z podanym ip, które jest zdefiniowane w argumentach wywołania.

## Sposób kompilacji i uruchomienia
    gcc -Wall ./spoof.c -o ./spoof -lnet -lpcap -pthread
    ./spoof INTERFACE HOST DEST_IP

INTERFACE - interfejs karty sieciowej  <br>
HOST - adres bramy domyślnej, za który się podszywamy  <br/>
DEST_IP - adres ip w dns response.<br/>
Wymaga uprawnień roota.

## Uruchomienie testowe
Wywołanie sudo ./spoof ens33 192.168.1.1 212.53.172.218<br/>
## Testy
Program jednak miewa problemy z odpowiedzeniem w wystarczająco szybkim czasem, w teście poniżej klientem był pc z windowsem 10 i ustawionym statycznie adresem bramy dns na server z aktywnym spoofingiem. Po zapytaniu dns o stronę wp.pl, dostaliśmy odpowiedź, która przekierowała na plemiona.pl- "212.53.172.218"
<br/>![](https://gitlab.cs.put.poznan.pl/inf140107/programowanie-sieciowe/raw/master/projekt-zaliczeniowy/screenshot1.png)

Normalne działanie spoofingu bez ustawionego serwera dns. Utworzona fałszywa odpowiedź dns przychodzi po prawdziwym pakiecie dns z odpowiedzią.
![](https://gitlab.cs.put.poznan.pl/inf140107/programowanie-sieciowe/raw/master/projekt-zaliczeniowy/screenshot2.png)