#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <map>
#include <string>
#include <sys/unistd.h>

using namespace std;

typedef struct {
    unsigned char bajt;
    int czestosc;
} Bajt;

struct Node {
    int suma_czestosci;
    bool czy_znak = false;
    unsigned char znak;
    Node *left = nullptr;
    Node *right = nullptr;
};

struct SymbolKodowy
{
    char znak;
    char kod[256];
    int dlugosc;
};
vector<SymbolKodowy *> kolektorKodow;


void zmienRozszerzenie(char *nazwaPliku, const char *noweRozszerzenie) {
    char *kropka = strchr(nazwaPliku, '.');
    if (kropka == nullptr) {
        printf("Niepoprawna nazwa pliku. Brak rozszerzenia.\n");
        return;
    }

    size_t dlugosc_br = kropka - nazwaPliku;
    nazwaPliku[dlugosc_br] = '\0';
    strcat(nazwaPliku, ".");
    strcat(nazwaPliku, noweRozszerzenie);
}

void kompresor(FILE* wy, const vector<bool>& boole) {
    // Sprawdzenie, czy liczba bitów jest podzielna przez 8
    int dopelnien = 0;
    if (boole.size() % 8 != 0) {
        dopelnien = 8 - (boole.size() % 8);
    }

    // Zapisanie liczby nieznacz¹cych bitów na pocz¹tku pliku
    fputc(dopelnien, wy);
    printf("%d\n", dopelnien);


    // Zapisanie bitów w kolejnych bajtach
    unsigned char byte = 0; // 8 bitow
    int bitIndex = 7; // 0 7
    int counter = 0;  // Licznik zapisanych bitów


    for (bool bit : boole) {

        byte |= (bit << bitIndex); // aktualizacja

        bitIndex--;

        //gdy pelny bicior
        if (bitIndex < 0) {
            fputc(byte, wy);
            byte = 0;
            bitIndex = 7;
        }
        counter++;


        // Sprawdzenie, czy zapisano wystarczaj¹c¹ liczbê bitów
        if (counter == boole.size()) {
            // Uzupe³nienie ostatniego bajtu zerami, jeœli potrzeba
            if (dopelnien > 0) {
                byte |= 0; //(symbolicznie)
                fputc(byte, wy);
                printf("Ostatni bajt (uzupelnienie): %d\n", byte);
            }
            break;
        }

    }

    // Wypisanie zakodowanego ci¹gu bitów (z dope³nieniem)
    int i = 0;
    for (bool bit : boole) {
        putchar(bit ? '1' : '0');
        i++;
        if(i%8==0){
            printf(" ");
        }
    }

    printf("\nDlugosc ciagu kodowego: %d\n", i);

    // Wypisanie liczby bitów uzupe³nionych zerami
    printf("Bitow dopelnien: %d \n", dopelnien);
}

void dekompresor(FILE* we, char nazwaDekompresji[100]) {
    //Bitowa zawartosc pliku wejsciowego


    printf("Bitowa zawartosc pliku wejsciowego:\n");
    int readByte;
    int count = 0;
    printf("Bitset\n");

    //test
    while ((readByte = fgetc(we)) != EOF) {

        //bajt
        bitset<8> bits(readByte); //obj "bits" t bitset 8 init readByte

        for (int i = 7; i >= 0; i--) {
            printf("%d", bits.test(i) ? 1 : 0); //bitset lib
            count++;
            if (count % 8 == 0) {
                printf(" ");
            }
        }
    }

    printf("\n");
    rewind(we);

    // mapa
    map<string, char> odwzorowanie;
    for (SymbolKodowy *symbol: kolektorKodow) {
        string kod(symbol->kod, symbol->dlugosc);
        odwzorowanie[kod] = symbol->znak;
    }

    // Odczytanie liczby nieznacz¹cych bitów
    int dopelnien = fgetc(we);
    printf("Odebrano %d nieznaczacych bitow\n", dopelnien);

    // Przesuniêcie wskaŸnika na pocz¹tek waznych danych
    fseek(we, 1, SEEK_SET);

    // wlasciwe wpisani bitow do buffer
    vector<bool> buffer;
    int byte;
    while ((byte = fgetc(we)) != EOF) { //z pliku kompresji
        for (int bitIndex = 7; bitIndex >= 0; bitIndex--) { //po bajcie
            bool b = (byte >> bitIndex) & 1; //spr czy bit najmniej zaczacy jest 0 czy 1
            buffer.push_back(b);
        }
    }

    printf("Buffer values: \n");
    for(auto b: buffer){
        printf("%d",(int)b);
    }

    // Otwarcie pliku do zapisu zdekompresowanych danych
    FILE *dekompresja = fopen(nazwaDekompresji, "w");
    if (dekompresja == nullptr) {
        printf("Blad otwarcia pliku do zapisu.\n");
        return;
    }

    // Dekodowanie skompresowanej wiadomoœci
    string biezacyKod;
    char znak;
    printf(" \nZdekodowane znaki: \n");

    int counter = 0;
    int licznikZnakow = 0;
    int znaczaceBity = buffer.size() - dopelnien;

    while (counter < znaczaceBity) {

        biezacyKod.push_back(buffer[counter] ? '1' : '0'); //dopisanie do kodu

        if (odwzorowanie.find(biezacyKod) != odwzorowanie.end()) { //find kodu znaku w mapie odwzorowan nie w ostatnim
            // gdy odwzorowanie.end() = 1 to nie znaleziono

            znak = odwzorowanie.at(biezacyKod); //przypisanie znaku do odpowiadajacego kodu

            printf("Counter: %d \nKod: %s\n", counter, biezacyKod.c_str());

                fwrite(&znak, sizeof(char), 1, dekompresja);
                printf("Znaleziono znak: ");
                printf("%c\n", znak);
                licznikZnakow++;


            if (counter == znaczaceBity - 1) {
                break;  // ostatni znak
            }
            biezacyKod = "";
        }
        counter++;
    }


    printf("\n");

    printf("Counter: %d\n", counter);
    fclose(dekompresja);
    fclose(we);
    printf("\n");
}

void kompresja(FILE *we, char nazwa_kopresja[100]) {
    vector<bool> boole;
    fseek(we, 0, SEEK_SET);
    int ch;

    //przepisywanie kazdego znaku z pliku do bool
    while ((ch = fgetc(we)) != EOF) {
        for(SymbolKodowy* symbol : kolektorKodow){ //na podstawie kolektora
            if(symbol->znak == ch){
                printf("%c",ch);
                for(int i=0; symbol->kod[i] != '\0'; i++){
                    if(symbol->kod[i]=='0'){
                        boole.push_back(false);
                    }else{
                        boole.push_back(true);
                    }
                }
            }
        }
    }
    char *rozsz = "kompresja";
    zmienRozszerzenie(nazwa_kopresja,rozsz);
    FILE* kompresja = fopen(nazwa_kopresja, "wb");  // Otwórz plik w trybie binarnym
    if (kompresja == nullptr) {
        cout << "B³¹d otwarcia pliku do zapisu." << endl;
        return;
    }

    kompresor(kompresja, boole);  // Wywo³anie funkcji zapisuj¹cej do pliku

    fclose(kompresja);  // Zamkniêcie pliku

}

int porownaj(const void *a, const void *b) {
    const Bajt *infoA = (const Bajt *) a;
    const Bajt *infoB = (const Bajt *) b;
    return infoB->czestosc - infoA->czestosc;
}

void wyswietlDrzewoHuffmana(FILE *wy, Node *root, int poziom = 0, char symbol = '-') {
    if (root == nullptr) {
        return;
    }

    wyswietlDrzewoHuffmana(wy, root->right, poziom + 1, '/');

    for (int i = 0; i < poziom; i++) {
        fprintf(wy, "    ");
        printf("    ");
    }

    if (poziom > 0) {
        fprintf(wy, "%c--", symbol);
        printf("%c--", symbol);
    }

    if (root->znak != '\0') {
        fprintf(wy, " %d -- krotnosc znaku %c (0x%x) : %d", root->suma_czestosci, root->znak, root->znak,
                root->suma_czestosci);
        printf(" %d -- krotnosc znaku %c (0x%x) : %d", root->suma_czestosci, root->znak, root->znak,
               root->suma_czestosci);
    } else {
        fprintf(wy, ": %d", root->suma_czestosci);
        printf(": %d", root->suma_czestosci);
    }
    fprintf(wy, "\n");
    printf("\n");

    wyswietlDrzewoHuffmana(wy, root->left, poziom + 1, '\\');
}

vector<SymbolKodowy*> utworzTabeleKodowa(FILE *wy, int level, Node *n, string kod = "") {
    static int index = 0;
    static SymbolKodowy tabela[256];

    if (n->czy_znak) {
        /*if(n->znak==0X0A && (n->left->znak==0X0D || n->right->znak==0X0D)){
            chat temp =
            fprintf();

        }*/
        //z tabeli kodowej wylaczamy nowa linie

            fprintf(wy, "|%c (0x%x) | %s\n", n->znak, n->znak, kod.c_str());
            printf("|%c (0x%x) | %s\n", n->znak, n->znak, kod.c_str());
            tabela[index].znak = n->znak;
            strcpy(tabela[index].kod, kod.c_str());
            index++;
            // Dodaj obiekt SymbolKodowy do wektora kolektorKodow
            SymbolKodowy* symbol = new SymbolKodowy;
            symbol->znak = n->znak;
            strcpy(symbol->kod, kod.c_str());
            kolektorKodow.push_back(symbol);


    }

    if (n->left != nullptr) {
        utworzTabeleKodowa(wy, level + 1, n->left, kod + "0");
    }
    if (n->right != nullptr) {
        utworzTabeleKodowa(wy, level + 1, n->right, kod + "1");
    }


    //przypadek prawid³owy
}

void wypiszKodowanie() {
    printf("\nZnaki kodowe z kolektora:\n");
    for (const SymbolKodowy* symbol : kolektorKodow) {
        //cout << "Znak: " << symbol->znak << ", Kod: " << symbol->kod << endl;
        printf("Znak: %c Hex: %02X, Kod: %s \n",symbol->znak,symbol->znak,symbol->kod );

    }

}

void utworzDrzewoHuffmana(Bajt bajty[256]) {

    vector<Node *> wszystkie;
    for (int t = 0; t != 256 && bajty[t].czestosc != 0; t++) {
        wszystkie.push_back(new Node{bajty[t].czestosc, true, bajty[t].bajt});
    }

    while (wszystkie.size() > 1) {
        //sort. malej¹co
        sort(wszystkie.begin(), wszystkie.end(), [](const void *e1, const void *e2) {
            return ((Node *) e1)->suma_czestosci > ((Node *) e2)->suma_czestosci;
        });
        //³¹czenie dwoch najmniejszych
        Node *mergen1 = wszystkie.back();
        wszystkie.pop_back();
        Node *mergen2 = wszystkie.back();
        wszystkie.pop_back();
        Node *newnode = new Node{mergen1->suma_czestosci + mergen2->suma_czestosci, false, 0, mergen1, mergen2};
        wszystkie.push_back(newnode);
    }

    FILE *wy = fopen("drzewo_tabela_kodowa.txt", "w");
    if (wy == nullptr) {
        printf("Nie mo¿na otworzyæ pliku Drzewo_i_tabela_kodowa.txt\n");
        return;
    }

    Node *root = wszystkie[0];
    wyswietlDrzewoHuffmana(wy, root);
    fprintf(wy, "\nTabela kodowa:\n");
    printf("\nTabela kodowa:\n");
    utworzTabeleKodowa(wy, 0, wszystkie[0]);

    fclose(wy);
}

void zapiszTabeleKodowa(char nazwa[100]) {
    char *rozsz = "tabelakodowa";
    zmienRozszerzenie(nazwa,rozsz);
    FILE *wy;
    wy = fopen(nazwa, "w");
    for (const SymbolKodowy* symbol : kolektorKodow) {
        /*if(symbol->znak==0x0D){
            continue;
        }*/
        fprintf(wy, "%d %s\n", symbol->znak, symbol->kod);
    }
}

void pobierzTabeleKodowa(const char* nazwaPliku) {
    kolektorKodow.clear();

    FILE* kody = fopen(nazwaPliku, "r");
    if (kody == NULL) {
        printf("B³¹d otwarcia pliku %s\n", nazwaPliku);
        return;
    }

    char linia[256];
    while (fgets(linia, sizeof(linia), kody) != NULL) {
        SymbolKodowy* symbol = new SymbolKodowy;
        int a;
        sscanf(linia, "%d %s", &a, symbol->kod);
        symbol->znak = a;
       /* if(symbol->znak==0x0D || symbol->znak==0x0A){
            continue;
        }*/
        symbol->dlugosc = static_cast<int>(strlen(symbol->kod));  // Oblicz d³ugoœæ kodu
        kolektorKodow.push_back(symbol);
    }

    fclose(kody);

    // Wyœwietlanie tablicy kodowej
    printf("Testowe wyswietlanie pobranej z pliku tabeli kodowej:\n");
    wypiszKodowanie();
}

void dekompresja(char *nazwaPliku,FILE* we){
    char *rozsz = "tabelakodowa";
    char *rozsz2 = "dekompresja";
    char *plikTabeli = nazwaPliku;
    char *plikDekompresji = nazwaPliku;
    zmienRozszerzenie(plikTabeli,rozsz);
    pobierzTabeleKodowa(plikTabeli);
    zmienRozszerzenie(plikDekompresji,rozsz2);
    dekompresor(we,nazwaPliku);

}

int main(int argc, char *argv[]) {

    FILE *we, *wy, *modelSort;
    we = fopen(argv[1], "rb"); // "r" read binary
    Bajt bajty[256] = {0};
    int licznik = 0;
    int ch;

    if (we == nullptr) { //spr czy podano argument
        printf("NIE PODANO NAZWY PLIKU JAKO ARGUMENT\a");
        return 0;
    }

    char *nazwaPliku = argv[1];

    if (strstr(nazwaPliku, ".kompresja") != nullptr) {
        dekompresja(nazwaPliku,we);

    }else{
        while ((ch = fgetc(we)) != EOF) {//liczenie czestosci wystapien bajtow
            bajty[ch].bajt = (unsigned char) ch;
            bajty[ch].czestosc++;
            licznik++;
        }

        qsort(bajty, 256, sizeof(Bajt), porownaj);

        char nazwa[100];
        char *rozszerzenie = "modelSort";

        strncpy(nazwa, argv[1], sizeof(nazwa));

        zmienRozszerzenie(nazwa, rozszerzenie);

        wy = fopen(nazwa, "w");
        fprintf(wy, "Ilosc znakow w tekscie: %d\n", licznik);
        printf("Ilosc znakow w tekscie: %d\n", licznik);
        //wy = fopen("posortowany_model.txt", "w");
        if (wy == nullptr) {
            printf("Nie mo¿na otworzyæ pliku %s\n", nazwa);
            fclose(we);
            return 0;
        }
        for (int w = 0; w < 256; w++) {
            if (bajty[w].czestosc > 0) {
                fprintf(wy, "Bajt 0x%02X ilosc wystapien %d\n", bajty[w].bajt, bajty[w].czestosc);
                printf("Bajt 0x%02X ilosc wystapien %d\n", bajty[w].bajt, bajty[w].czestosc);
            }
        }

        utworzDrzewoHuffmana(bajty);
        wypiszKodowanie();
        kompresja(we, nazwa);
        zapiszTabeleKodowa(nazwa);

        fclose(wy);
        fclose(we);


    }

    return 0;
}
