#include "pelisuorite.hpp"

pelisuorite::pelisuorite(int pelaaja_id, int taso, int konemaara, int porrastusvirheet, int muut_virheet, int pisteet, int tarkiste) {
    this->pelaaja_id = pelaaja_id;
    this->taso = taso;
    this->konemaara = konemaara;
    this->porrastusvirheet = porrastusvirheet;
    this->muut_virheet = muut_virheet;
    this->pisteet = pisteet;
    this->tarkiste = tarkiste;
}

pelisuorite::pelisuorite(int pelaaja_id, int taso, int konemaara, int porrastusvirheet, int muut_virheet, int pisteet) {
    this->pelaaja_id = pelaaja_id;
    this->taso = taso;
    this->konemaara = konemaara;
    this->porrastusvirheet = porrastusvirheet;
    this->muut_virheet = muut_virheet;
    this->pisteet = pisteet;
}

std::string pelisuorite::anna_suorite() {
    return  apuvalineet::tekstiksi(this->pelaaja_id) + "|" +
            apuvalineet::tekstiksi(this->taso) + "|" +
            apuvalineet::tekstiksi(this->konemaara) + "|" +
            apuvalineet::tekstiksi(this->porrastusvirheet) + "|" +
            apuvalineet::tekstiksi(this->muut_virheet) + "|" +
            apuvalineet::tekstiksi(this->pisteet) + "|" +
            apuvalineet::tekstiksi(this->laske_tiiviste()) + "|";
}

int pelisuorite::laske_tiiviste() {
    int summa = this->pelaaja_id * 7 + this->taso * 5 + this->konemaara * 3 + this->porrastusvirheet * 7 + this->muut_virheet * 5 + this->pisteet * 3;

    int tarkiste = summa % 31;

    return apuvalineet::luvuksi<int>(apuvalineet::tekstiksi(summa) + apuvalineet::tekstiksi(tarkiste));
}

bool pelisuorite::tarkista(int tarkiste) {
    return tarkiste == this->laske_tiiviste();
}

int pelisuorite::anna_id() {
    return this->pelaaja_id;
}

int pelisuorite::anna_pisteet() {
    return this->pisteet;
}

int pelisuorite::anna_konemaara() {
    return this->konemaara;
}

int pelisuorite::anna_porrastusvirheet() {
    return this->porrastusvirheet;
}

int pelisuorite::anna_virheet() {
    return this->muut_virheet;
}

int pelisuorite::anna_taso() {
    return this->taso;
}

int pelisuorite::anna_tarkiste() {
    return this->tarkiste;
}
