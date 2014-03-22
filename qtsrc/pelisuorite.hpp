#ifndef PELISUORITE_HPP
#define PELISUORITE_HPP

#include <string>

#include "apuvalineet.hpp"

class pelisuorite {
    int pelaaja_id;
    int taso;
    int konemaara;
    int porrastusvirheet;
    int muut_virheet;
    int pisteet;
public:
    pelisuorite(int pelaaja_id, int taso, int konemaara, int porrastusvirheet, int muut_virheet, int pisteet);
    std::string anna_suorite();
    int laske_tiiviste();
    bool tarkista(int tarkiste);
};

#endif // PELISUORITE_HPP