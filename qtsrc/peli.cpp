#include "peli.hpp"

Peli::Peli(IAsetukset& a, Atis &at, Metar& m) : asetukset(a), atis(at), metar(m), koska_uusi_kone(50) {
	porrastusvirheet = 0;
	muut_virheet = 0;
    kasitellyt = 0;
	koska_metar = asetukset.anna_asetus("koska_metar");
	lataa_tunnukset("data/tunnukset.txt");
    lataa_pelaajat("data/pelaajat.txt");
    lataa_pisteet("data/pisteet.txt");
	generoi_metar();
    valittuKone = NULL;
    laskeutuvaKone = false;
    taso = 1;
    edellinen_kone_lahto = -1;
    jaljet_intervalli = 5;
}

void Peli::lataa_tunnukset(std::string tunnukset) {
    this->tunnukset = apuvalineet::lue_tiedosto(tunnukset);
}

void Peli::aseta_virhe(int virhe) {
    std::ofstream ulos("virhedata.txt", std::ios::app);

    switch (virhe) {
        case VIRHE_KORKEUS_ALA:
            virheteksti = "Korkeuden alaraja on " + apuvalineet::tekstiksi(asetukset.anna_asetus("selvityskorkeus_ala")) + " jalkaa";
            break;
        case VIRHE_KORKEUS_YLA:
            virheteksti = "Korkeuden yläraja on " + apuvalineet::tekstiksi(asetukset.anna_asetus("selvityskorkeus_yla")) + " jalkaa";
            break;
        case VIRHE_LAHESTYMISKORKEUS:
            virheteksti = "Lähestymisen yläraja on " + apuvalineet::tekstiksi(asetukset.anna_asetus("maks_lahestymiskorkeus")) + " jalkaa";
            break;
        case VIRHE_LAHESTYMISNOPEUS:
            virheteksti = "Lähestymisen maksiminopeus on " + apuvalineet::tekstiksi(asetukset.anna_asetus("maks_lahestymisnopeus")) + " solmua";
            break;
        case VIRHE_LAHESTYMISSUUNTA:
            virheteksti ="Koneen suunnan tulee olla kiitotien suunnasta +- " + apuvalineet::tekstiksi(asetukset.anna_asetus("lahestymiskulma"));
            break;
        case VIRHE_NOPEUS_ALA:
            virheteksti = "Nopeuden alaraja on " + apuvalineet::tekstiksi(asetukset.anna_asetus("selvitysnopeus_ala")) + " solmua";
            break;
        case VIRHE_NOPEUS_YLA:
            virheteksti = "Nopeuden yläraja on " + apuvalineet::tekstiksi(asetukset.anna_asetus("selvitysnopeus_yla")) + " solmua";
            ulos << "Annettu selvitysnopeus on suuri pieni aikaan " << this->pelin_kello << std::endl;
            break;
        case VIRHE_PORRASTUS:
            virheteksti = "Porrastuksen tulee olla pystysuuntaan " + apuvalineet::tekstiksi(asetukset.anna_asetus("porrastus_pysty")) + " jalkaa ja vaakasuuntaan " + apuvalineet::tekstiksi(asetukset.anna_asetus("porrastus_vaaka")) + " mailia";
            break;
        case VIRHE_OIKOTIE:
            virheteksti = "Oikotien saa antaa vasta " + apuvalineet::tekstiksi(asetukset.anna_asetus("oikotie")) + " jalassa";
            break;
        case VIRHE_ALUEELTA:
            virheteksti = "Kone poistui alueeltasi";
          break;
    }

    ulos << virheteksti << " " << pelin_kello << std::endl;

    if (virhe != VIRHE_PORRASTUS) {
        ++muut_virheet;
    }

    ulos.close();
}

void Peli::luo_kone() {
	int j = apuvalineet::arvo_luku(10, 100) % 2;
	apuvalineet::piste paikka;
	bool odotus;
	std::string tunnus = generoi_tunnus();

    unsigned int lahtevat_alkaa = std::distance(navipisteet.begin(), std::find(navipisteet.begin(), navipisteet.end(), Peli::LAHTEVA));
    unsigned int lahtevat_loppuu = navipisteet.size() - 1;
    unsigned int saapuvat_alkaa = 0;
    unsigned int saapuvat_loppuu = lahtevat_alkaa - 1;

    #ifdef DEBUG
        std::clog << "lahtevat_alkaa = " << navipisteet[lahtevat_alkaa].nimi << std::endl;
        std::clog << "lahtevat_loppuu = " << navipisteet[lahtevat_loppuu].nimi << std::endl;
    #endif

	if (j == LAHTEVA) {
        std::vector <kiitotie> :: iterator haku_lahto = std::find(kentta.kiitotiet.begin(), kentta.kiitotiet.end(), atis.anna_lahtokiitotie());
        double suunta = haku_lahto->suunta;

        if (!onko_vapaata() || edellinen_kone_lahto > (pelin_kello-asetukset.anna_asetus("lahtevien_porrastus")*60)) {
            paikka = haku_lahto->odotuspiste;
            odotus = true;
        } else {
            paikka = haku_lahto->alkupiste;
            odotus = false;
		}
        #ifdef DEBUG
            std::clog << "lahtevat_alkaa = " << lahtevat_alkaa << " lahtevat_loppuu = " << lahtevat_loppuu << std::endl;
        #endif

        if (lahtevat_alkaa == navipisteet.size()) {
            lahtevat_alkaa = 0;
        }

        koneet.push_back(new lentokone(tunnus, paikka, kentta.korkeus, 0.0, suunta, LAHTEVA, odotus, kentta, atis, asetukset));
        koneet.back()->aseta_ulosmenopiste(navipisteet[apuvalineet::arvo_luku(lahtevat_alkaa, lahtevat_loppuu)]);

        if (koneet.back()->anna_odotus()) {
            odottavat.push(*koneet.back());
		}

	} else {
		int i = -1;
        #ifdef DEBUG
            std::clog << "saapuvat_alkaa = " << saapuvat_alkaa << " saapuvat_loppuu = " << saapuvat_loppuu << std::endl;
        #endif
		do {
            i = apuvalineet::arvo_luku(saapuvat_alkaa, saapuvat_loppuu);
		} while (!onko_vapaata(SAAPUVA, i));

        koneet.push_front(new lentokone(tunnus, navipisteet[i].paikka, navipisteet[i].lentokorkeus, navipisteet[i].lentonopeus, navipisteet[i].lentosuunta, SAAPUVA, false, kentta, atis, asetukset));
	}

	tilasto tmp;

	tmp.tunnus = tunnus;
    tmp.sisaan = this->pelin_kello;
	tmp.pois = 0.0;
	tmp.selvitykset = 0;
    tmp.pisteet = 0;

	ajat.push_back(tmp);
}

bool jarjesta_pisteet(const navipiste& a, const navipiste& b) {
    return a.tyyppi < b.tyyppi;
}

void tulosta(navipiste& np) {
    #ifdef DEBUG
        std::clog << np.nimi << " " << np.tyyppi << std::endl;
    #endif
}

void Peli::lataa_kentta(std::string kenttaNimi) {
    #ifdef DEBUG
        std::clog << "Peli::lataa_kentta(" << kenttaNimi << ")" << std::endl;
    #endif

    std::vector <std::string> rivit = apuvalineet::lue_tiedosto("kentat/" + kenttaNimi);

    for (unsigned int i = 0; i < rivit.size(); ++i) {
        std::vector <std::string> asiat = apuvalineet::pilko_rivi(rivit[i], "|");

		if (asiat[0] == "N") {
            if (asiat[1] == "") {
                throw std::runtime_error("Kentän nimi puuttuu " + kenttaNimi + " rivillä " + apuvalineet::tekstiksi(i));
            }
			kentta.nimi = asiat[1];
		} else if (asiat[0] == "H") {
			kentta.korkeus = apuvalineet::luvuksi<double>(asiat[1]);
		} else if (asiat[0] == "P") {
			kentta.paikka.x = apuvalineet::luvuksi<double>(asiat[1]);
			kentta.paikka.y = apuvalineet::luvuksi<double>(asiat[2]);
		} else if (asiat[0] == "K") {
			apuvalineet::piste alku = apuvalineet::uusi_paikka(kentta.paikka, apuvalineet::luvuksi<double>(asiat[2]), apuvalineet::luvuksi<double>(asiat[3]));
            kiitotie tmpa(asiat[1], alku, apuvalineet::luvuksi<double>(asiat[5]), apuvalineet::luvuksi<double>(asiat[4]), apuvalineet::luvuksi<double>(asiat[6]), apuvalineet::luvuksi<double>(asiat[7]), apuvalineet::luvuksi<double>(asiat[8]), asetukset.anna_asetus("lahestymispiste"), asetukset.anna_asetus("hidastuspiste"), apuvalineet::luvuksi<double>(asiat[9]), apuvalineet::luvuksi<double>(asiat[10]));

			kentta.kiitotiet.push_back(tmpa);
		} else if (asiat[0] == "L") {
			apuvalineet::piste paikka;
			paikka = apuvalineet::uusi_paikka(kentta.paikka, apuvalineet::luvuksi<double>(asiat[2]), apuvalineet::luvuksi<double>(asiat[3]));

			navipiste tmp(apuvalineet::tekstiksi(asiat[1]), paikka);

			sisapisteet.push_back(tmp);
        } else if (asiat[0] == "U") {
            apuvalineet::piste paikka = apuvalineet::uusi_paikka(kentta.paikka, apuvalineet::luvuksi<double>(asiat[2]), apuvalineet::luvuksi<double>(asiat[3]));
            navipiste tmp_piste;
            int tyyppi = Peli::MOLEMMAT;

            if (asiat.back() == "S") {
                tyyppi = Peli::SAAPUVA;
            } else if (asiat.back() == "L") {
                tyyppi = Peli::LAHTEVA;
            }

            if (tyyppi == Peli::SAAPUVA || tyyppi == Peli::MOLEMMAT) {
                navipiste tmp(asiat[1], paikka, apuvalineet::luvuksi<double>(asiat[4]), apuvalineet::luvuksi<double>(asiat[5]), apuvalineet::luvuksi<double>(asiat[6]), tyyppi);
                tmp_piste = tmp;
            } else if (tyyppi == Peli::LAHTEVA) {
                #ifdef DEBUG
                    std::clog << "Lahteva" << std::endl;
                #endif
                navipiste tmp(asiat[1], paikka, tyyppi);
                tmp_piste = tmp;
            }

            navipisteet.push_back(tmp_piste);
		} else {
            throw std::runtime_error("Tiedosto " + kenttaNimi + " on väärässä formaatissa");
		}

		asiat.clear();
	}


    std::for_each(navipisteet.begin(), navipisteet.end(), tulosta);
    #ifdef DEBUG
        std::clog << std::endl << "KATKO" << std::endl << std::endl;
    #endif
    std::sort(navipisteet.begin(), navipisteet.end(), jarjesta_pisteet);

    std::for_each(navipisteet.begin(), navipisteet.end(), tulosta);
}

std::string Peli::generoi_tunnus() {
	std::string tunniste = tunnukset[apuvalineet::arvo_luku(0, tunnukset.size())];
	std::string luku = apuvalineet::tekstiksi(apuvalineet::arvo_luku(0, 999));

	return tunniste+luku;
}

void Peli::aseta_hiiren_paikka(const apuvalineet::piste& hiiri) {
    hiiren_paikka = hiiri;
}

void Peli::valitse_kone(const apuvalineet::piste& hiiri) {
    valittuKone = NULL;
    for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
        if (apuvalineet::onko_alueella(hiiri, (*it)->paikka, 0.5)) {
            valittuKone = *it;
            break;
		}
	}
}

void Peli::tarkista_porrastus() {
    int virheita = 0;
    std::list <lentokone*> alittuu;

    for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
        for (std::list <lentokone*> :: iterator jt = koneet.begin(); jt != koneet.end(); ++jt) {
            if (it == jt) {
				continue;
			}

            if (!(*it)->anna_odotus() && !(*jt)->anna_odotus()) {
                if ((*it)->anna_korkeus() >= 1000 && (*jt)->anna_korkeus() >= 1000) {
                    if (apuvalineet::etaisyys((*it)->paikka, (*jt)->paikka) < asetukset.anna_asetus("porrastus_vaaka")) {
                        if (std::abs((*it)->anna_korkeus() - (*jt)->anna_korkeus()) < asetukset.anna_asetus("porrastus_pysty")) {
                            alittuu.push_back((*it));
                            alittuu.push_back((*jt));

                            if ((*it)->onko_porrastus || (*jt)->onko_porrastus) {
                                ++virheita;
                            }

                            (*it)->onko_porrastus = false;
                            (*jt)->onko_porrastus = false;
                        }
                    }
                }
            }
		}
	}

    if (virheita) {
        porrastusvirheet += virheita;
        aseta_virhe(VIRHE_PORRASTUS);
    }

    for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
        (*it)->onko_porrastus = true;
    }

    for (std::list <lentokone*> :: iterator it = alittuu.begin(); it != alittuu.end(); ++it) {
        (*it)->onko_porrastus = false;
    }
}

void Peli::logita_aika(lentokone* lk) {
    std::vector <tilasto> :: iterator haku_pois = std::find(ajat.begin(), ajat.end(), lk->anna_kutsutunnus());
    haku_pois->pois = this->pelin_kello;
    haku_pois->pisteet += 100000 - apuvalineet::pyorista((haku_pois->pois - haku_pois->sisaan) * haku_pois->selvitykset, 10);
}

bool pois(lentokone* kone) {
    return kone->poistetaanko;
}

void Peli::hoida_koneet(double intervalliMs) {
    tarkista_porrastus();
    std::vector <kiitotie> :: iterator haku_lahto = std::find(kentta.kiitotiet.begin(), kentta.kiitotiet.end(), atis.anna_lahtokiitotie());
    std::vector <kiitotie> :: iterator haku_lasku = std::find(kentta.kiitotiet.begin(), kentta.kiitotiet.end(), atis.anna_laskukiitotie());

    std::list <lentokone*> :: iterator loppu;
    loppu = std::remove_if(koneet.begin(), koneet.end(), pois);
    koneet.erase(loppu, koneet.end());
    bool jaljet_muistiin = true;

    for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
        if ((*it)->anna_tyyppi() == Peli::LAHTEVA) {
            if (apuvalineet::onko_alueella((*it)->paikka, (*it)->anna_ulosmenopiste().paikka, 0.05)) {
                (*it)->poistetaanko = true;
                ++kasitellyt;
                logita_aika((*it));
            }
        } else if ((*it)->anna_tyyppi() == Peli::SAAPUVA) {
            if ((*it)->anna_nopeus() < 4.0) {
                (*it)->poistetaanko = true;
                ++kasitellyt;
                logita_aika((*it));
                laskeutuvaKone = false;
            }
        }

        if ((*it)->anna_paikka().x < 0 || (*it)->paikka.x > asetukset.anna_asetus("ruutu_leveys") || (*it)->paikka.y < 0 || (*it)->paikka.y > asetukset.anna_asetus("ruutu_korkeus")) {
            (*it)->poistetaanko = true;
            logita_aika((*it));
        }

        if ((*it)->anna_odotus()) {
            if (onko_vapaata() && (edellinen_kone_lahto < ((pelin_kello - asetukset.anna_asetus("lahtevien_porrastus") * 60)) || (edellinen_kone_lahto == -1))) {
                (*it)->paikka = haku_lahto->alkupiste;
                (*it)->aseta_odotus(false);
				odottavat.pop();
                edellinen_kone_lahto = pelin_kello;
                #ifdef DEBUG
                    std::clog << "Edellinen lahti " << edellinen_kone_lahto << std::endl;
                #endif
			}
		}

        if ((*it)->tyyppi == Peli::LAHTEVA && (*it)->anna_korkeus() < 1200 && (*it)->anna_odotus() == false) {
            if ((*it)->anna_nopeus() == 0) {
                (*it)->muuta_selvitysnopeutta(asetukset.anna_asetus("alkunopeus"));
			}

            if ((*it)->anna_nopeus() > 150) {
                (*it)->muuta_selvityskorkeutta(haku_lahto->alkunousukorkeus);
                (*it)->muuta_selvitysnopeutta(asetukset.anna_asetus("alkunousunopeus"));
                if ((*it)->anna_korkeus() >= kentta.korkeus + haku_lahto->minimi_nousukorkeus) {
                   (*it)->muuta_selvityssuuntaa(haku_lahto->alkunoususuunta);
                }
			}
		}

        if ((*it)->anna_korkeus() >= 10000 && (*it)->anna_tyyppi() == Peli::LAHTEVA) {
            (*it)->muuta_selvitysnopeutta(440);
        }

        if ((*it)->anna_korkeus() <= kentta.korkeus + haku_lasku->lahestymiskorkeus && (*it)->anna_tyyppi() == Peli::SAAPUVA) {
            if (!(*it)->anna_laskuselvitys()) {
                if (!laskeutuvaKone) {
                    (*it)->ota_selvitys(apuvalineet::LASKU);
                    laskeutuvaKone = true;
                } else {
                    (*it)->ota_selvitys(apuvalineet::KESKEYTA);
                }
            }
        }

        (*it)->liiku(intervalliMs);

        if ((int)pelin_kello >= jaljet_intervalli) {
            jalki tmp = {(*it)->anna_paikka()};
            jaljet.push_back(tmp);
            jaljet_muistiin = true;
        } else {
            jaljet_muistiin = false;
        }
	}

    if (jaljet_muistiin) {
        jaljet_intervalli += 30;
    }
}

void Peli::lisaa_selvityksia() {
    std::vector <tilasto>::iterator lisattava = std::find(ajat.begin(), ajat.end(), valittuKone->anna_kutsutunnus());
	++lisattava->selvitykset;
}

std::vector <std::string> Peli::lataa_pilvet(std::string pilvet) {
	std::ifstream sisaan(pilvet.c_str(), std::ios::in);
	std::vector <std::string> tyypit;
	std::string pilvi;

	if (!sisaan) {
		throw std::runtime_error("Tiedosto " + pilvet + " ei aukea");
    }

	while (sisaan >> pilvi) {
		tyypit.push_back(pilvi);
	}

	sisaan.close();

	return tyypit;
}

void Peli::generoi_metar() {
    metar.aseta_tuuli(apuvalineet::pyorista(apuvalineet::arvo_luku(0, 360), 5));
    metar.aseta_paine(apuvalineet::arvo_luku(asetukset.anna_asetus("ilmanpaine_ala"), asetukset.anna_asetus("ilmanpaine_yla")));

    metar.aseta_lampotila(apuvalineet::arvo_luku(asetukset.anna_asetus("lampotila_ala"), asetukset.anna_asetus("lampotila_yla")));
    metar.aseta_ilmankosteus(apuvalineet::arvo_luku(asetukset.anna_asetus("ilmankosteus_ala"), asetukset.anna_asetus("ilmankosteus_yla")));

    metar.aseta_tila();
}

bool Peli::onko_vapaata(int tyyppi, int piste) {
	if (tyyppi == LAHTEVA) {
        for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
            if ((*it)->anna_korkeus() < (kentta.korkeus + asetukset.anna_asetus("porrastus_pysty")) && (*it)->anna_odotus() == false) {
				return false;
			}
		}
	} else {
        for (std::list <lentokone*> :: iterator it = koneet.begin(); it != koneet.end(); ++it) {
            if (apuvalineet::etaisyys((*it)->paikka, navipisteet[piste].paikka) < asetukset.anna_asetus("porrastus_vaaka")) {
				return false;
			}
		}

		return true;
	}

	return true;
}

void Peli::aloita() {
    koska_uusi_kone = 15;
    pelin_kello = 0;
}

void Peli::aseta_taso(int taso) {
    this->taso = taso;
}

void Peli::aseta_pelin_kello(double aika){
    this->pelin_kello += aika;
}

double Peli::anna_pelin_kello() {
    return this->pelin_kello;
}

int Peli::anna_taso() {
    return this->taso;
}

int Peli::anna_pisteet() {
    int tmp = 0;

    for (unsigned int i = 0; i < ajat.size(); ++i) {
        tmp += ajat[i].pisteet;
    }

    return tmp;
}

void Peli::aseta_pelaaja(std::string tunnus) {
    std::vector <pelaaja> :: iterator p = std::find(pelaajat.begin(), pelaajat.end(), tunnus);

    if (p == pelaajat.end()) {
        this->pelaajat.push_back(pelaaja(this->pelaajat.size(), tunnus));
        #ifdef DEBUG
            std::clog << this->pelaajat.back().anna_tunnus() << std::endl;
        #endif
        this->peluri = pelaajat.back();
    } else {
        #ifdef DEBUG
            std::clog << p->anna_tunnus() << std::endl;
        #endif
        this->peluri = pelaajat[std::distance(pelaajat.begin(), p)];
    }

    std::ofstream ulos("data/pelaajat.txt", std::ios::out);

    for (unsigned int i = 0; i < this->pelaajat.size(); ++i) {
        #ifdef DEBUG
            std::clog << pelaajat[i].anna_id() << "|" << pelaajat[i].anna_tunnus() << std::endl;
        #endif
        ulos << pelaajat[i].anna_id() << "|" << pelaajat[i].anna_tunnus() << "|" << std::endl;
    }
}

pelaaja Peli::anna_pelaja() {
    return this->peluri;
}

void Peli::lataa_pelaajat(std::string tiedosto) {
    std::vector <std::string> rivit = apuvalineet::lue_tiedosto(tiedosto);

    for (unsigned int i = 0; i < rivit.size(); ++i) {
        #ifdef DEBUG
            std::clog << rivit[i] << std::endl;
        #endif
        std::vector <std::string> asiat = apuvalineet::pilko_rivi(rivit[i], "|");

        this->pelaajat.push_back(pelaaja(apuvalineet::luvuksi<int>(asiat[0]), asiat[1]));
    }

}

void Peli::lataa_pisteet(std::string tiedosto) {
    std::vector <std::string> rivit = apuvalineet::lue_tiedosto(tiedosto);
    std::vector <std::string> solut;
    #ifdef DEBUG
        std::clog << "peli::lataa_pisteet(" << tiedosto << ") rivien määrä " << rivit.size() << std::endl;
    #endif

    for (unsigned int i = 0; i < rivit.size(); ++i) {
        #ifdef DEBUG
            std::clog << rivit[i] << std::endl;
        #endif
        solut           = apuvalineet::pilko_rivi(rivit[i], "|");

        int id          = apuvalineet::luvuksi<int>(solut[0]);
        int taso        = apuvalineet::luvuksi<int>(solut[1]);
        int koneita     = apuvalineet::luvuksi<int>(solut[2]);
        int porrastus   = apuvalineet::luvuksi<int>(solut[3]);
        int muut        = apuvalineet::luvuksi<int>(solut[4]);
        int pisteet     = apuvalineet::luvuksi<int>(solut[5]);
        int tarkiste    = apuvalineet::luvuksi<int>(solut[6]);

        pistevektori.push_back(pelisuorite(id, taso, koneita, porrastus, muut, pisteet, tarkiste));
    }
}

void Peli::muodosta_suorite() {
    pistevektori.push_back(pelisuorite(peluri.anna_id(), taso, ajat.size(), porrastusvirheet, muut_virheet, anna_pisteet()));
}

void Peli::tallenna_pisteet() {
    std::ofstream ulos("data/pisteet.txt", std::ios::out);

    for (unsigned int i = 0; i < this->pistevektori.size(); ++i) {
        #ifdef DEBUG
            std::clog << pistevektori[i].anna_suorite() << std::endl;
        #endif
        ulos << pistevektori[i].anna_suorite() << std::endl;
    }

    ulos.close();
}

void Peli::tyhjenna_koneet() {
    this->koneet.clear();
}
