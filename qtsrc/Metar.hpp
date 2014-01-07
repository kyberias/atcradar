#ifndef METAR_HPP
#define METAR_HPP

#include <QString>
#include "../src/apuvalineet.hpp"
#include <vector>
#include <fstream>
#include <map>
#include <stdexcept>

class paineraja {
public:
    int alaraja;
    int ylaraja;
    int siirtopinta;

    paineraja (int ar, int yr, int sp) {
        this->alaraja = ar;
        this->ylaraja = yr;
        this->siirtopinta = sp;
    }
};

class Metar {
    int tuuli;
    int voimakkuus;
    double paine;
    int nakyvyys;
    int lampotila;
    int ilmankosteus;
    int kastepiste;
    std::string pilvet;
public:
    std::vector <paineraja> painerajat;
    Metar () {
        tuuli           = apuvalineet::pyorista(apuvalineet::arvo_luku(3, 23), 5);
        voimakkuus      = apuvalineet::arvo_luku(0, 20);
        paine           = apuvalineet::arvo_luku(940, 1060);
        nakyvyys        = apuvalineet::pyorista(apuvalineet::arvo_luku(200, 9999), 100);
        lampotila       = apuvalineet::arvo_luku(-30, 60);
        ilmankosteus    = apuvalineet::arvo_luku(50, 100);
        kastepiste      = (lampotila - (100 - ilmankosteus)) / 5;

        std::vector <std::string> pilvityypit;

        pilvityypit.push_back("SKC");
        pilvityypit.push_back("FEW");
        pilvityypit.push_back("BKN");
        pilvityypit.push_back("SCT");
        pilvityypit.push_back("OVC");

        for (unsigned int i = 0; i < apuvalineet::arvo_luku(0, 5); ++i) {
            pilvet += pilvityypit[apuvalineet::arvo_luku(0, pilvityypit.size()-1)] + apuvalineet::tekstiksi(apuvalineet::pyorista(apuvalineet::arvo_luku(1000, 8800), 100)) + " ";
        }
    }

    int anna_paine() {
        return this->paine;
    }

    QString getMessage() {
        return QString::fromStdString("EFRO " + apuvalineet::muuta_pituus(apuvalineet::tekstiksi(tuuli), 3) + apuvalineet::muuta_pituus(apuvalineet::tekstiksi(voimakkuus), 2) + "KT Q" + apuvalineet::tekstiksi(paine) + " " + apuvalineet::tekstiksi(nakyvyys) + " " + apuvalineet::tekstiksi(lampotila) + "/" + apuvalineet::tekstiksi(kastepiste) + " " + pilvet);
    }

    void downloadPrressureLimit(std::string file, int siirtokorkeus) {
        std::ifstream in(file.c_str(), std::ios::in);
        if (!in) {
            throw std::runtime_error("File " + file + " cannot be open");
        }
        std::string line;
        std::vector <std::string> words;

        while (std::getline(in, line)) {
            words = apuvalineet::pilko_rivi(line, " ");
            if (apuvalineet::luvuksi<int>(words[0]) == siirtokorkeus) {
                this->painerajat.push_back(paineraja(apuvalineet::luvuksi<int>(words[1]), apuvalineet::luvuksi<int>(words[2]), apuvalineet::luvuksi<int>(words[3])));
            }
        }

        in.close();
    }
};

#endif // METAR_HPP