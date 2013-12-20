#include "asetukset.h"
#include "peli.hpp"
#include "pelicontroller.hpp"
#include "peliview.hpp"
#include "ohjelma.hpp"
#include "lukija.hpp"
#include "ajastin.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// Pelin p��funktio.
int PeliController::aja() {
	int taso = -1;
	peli.porrastusvirheet = 0;
	peli.muut_virheet = 0;
	peli.kasitellyt = 0;
	syotteenluku lukija;
	peli.ohje = "Valitse taso 0 - 3";
	
	while (taso < 0) {
		lukija.lue_syote();
		peli.syote = lukija.anna_viesti();
		
		if (ohjelma.lue_nappi(Ohjelma::NAPPI_ENTER)) {
			if (lukija.anna_viesti().length()) {
				taso = apuvalineet::luvuksi<int>(lukija.anna_viesti());
			}
		}
		
		view.piirra_valinta();
	}
	
	lukija.tyhjenna();
	peli.ohje = " ";
	
	switch (taso) {
		case 3:
			Asetukset::asetukset["koska_uusi_ala"] = 30;
			Asetukset::asetukset["koska_uusi_yla"] = 80;
			Asetukset::asetukset["maks_konemaara"] = 30;
			Asetukset::asetukset["vaadittavat_kasitellyt"] = 30;
			break;
		case 2:
			Asetukset::asetukset["koska_uusi_ala"] = 60;
			Asetukset::asetukset["koska_uusi_yla"] = 120;
			Asetukset::asetukset["maks_konemaara"] = 15;
			Asetukset::asetukset["vaadittavat_kasitellyt"] = 20;
			break;
		case 1:
			Asetukset::asetukset["koska_uusi_ala"] = 120;
			Asetukset::asetukset["koska_uusi_yla"] = 240;
			Asetukset::asetukset["maks_konemaara"] = 10;
			Asetukset::asetukset["vaadittavat_kasitellyt"] = 15;
			break;
		case 0:
			Asetukset::asetukset["koska_uusi_ala"] = 150;
			Asetukset::asetukset["koska_uusi_yla"] = 300;
			Asetukset::asetukset["maks_konemaara"] = 8;
			Asetukset::asetukset["vaadittavat_kasitellyt"] = 10;
			break;
	}
	
	std::clog << "pelicontroller::aja()" << std::endl;

	peli.lataa_tunnukset("data/tunnukset.txt");

	peli.lataa_kentta("EFRO");

	std::srand(std::time(NULL));

	peli.generoi_metar();
	bool loppu = false, piirretty = false;

	pyyda_atis();

	// Nollataan kello.
	float pelin_kello = ohjelma.sekunnit(true);

	ohjelma.tyhjenna_syote();

	peli.koska_metar = Asetukset::anna_asetus("koska_metar");

	std::vector <ajastin> ajastimet;

	ajastimet.push_back(ajastin("metar", Asetukset::anna_asetus("koska_metar"), 0));

	peli.luo_kone(ohjelma);
	peli.luo_kone(ohjelma);

	while (!loppu) {
		int alku = ohjelma.sekunnit();
		apuvalineet::piste hiiri = ohjelma.anna_hiiri();

		peli.valitse_kone(hiiri);

		if (peli.porrastusvirheet >= Asetukset::anna_asetus("maks_porrastusvirhe")) {
			std::clog << "Porrastusvirheet t�ynn�" << std::endl;
			loppu = true;
		}

		if (alku == peli.koska_uusi_kone) {
			if (peli.koneet.size() < Asetukset::anna_asetus("maks_konemaara")) {
				peli.luo_kone(ohjelma);
				peli.koska_uusi_kone += apuvalineet::arvo_luku(Asetukset::anna_asetus("koska_uusi_ala"), Asetukset::anna_asetus("koska_uusi_yla"));
			}

			std::clog << "Seuraava kone luodaan " << (peli.koska_uusi_kone - alku) << " sekunnin kuluttua" << std::endl;
		}

		if (alku == peli.koska_metar) {
			peli.generoi_metar();

			if (!peli.tarkista_atis()) {
				peli.atis.ok = false;
				pyyda_atis();
			}

			peli.koska_metar += Asetukset::anna_asetus("koska_metar");
		}

		// Esc-nappi lopettaa pelin.
		if (ohjelma.lue_nappi(Ohjelma::NAPPI_ESCAPE)) {
			loppu = true;
		}

		if (ohjelma.lue_nappi(Ohjelma::NAPPI_F5)) {
			peli.toiminto = Peli::SUUNTA;
		}
		else if (ohjelma.lue_nappi(Ohjelma::NAPPI_F7)) {
			peli.toiminto = Peli::NOPEUS;
		}
		else if (ohjelma.lue_nappi(Ohjelma::NAPPI_F8)) {
			peli.toiminto = Peli::KORKEUS;
		}

		peli.syote = lukija.lue_syote();

		if (ohjelma.lue_nappi(Ohjelma::NAPPI_ENTER) && peli.etsi_valittu_kone() >= 0) {
			peli.koneet[peli.etsi_valittu_kone()].reaktioaika = pelin_kello + apuvalineet::arvo_luku(4, 10);
			std::string tmp = lukija.anna_viesti();

			if (peli.toiminto == Peli::KORKEUS) {
				if (tmp.length() == 2 || tmp.length() == 3) {
					double luku = apuvalineet::luvuksi<double>(tmp) * 100;
					tmp = apuvalineet::tekstiksi(luku);
				}
			}

			if (tmp == "ILS" && peli.koneet[peli.etsi_valittu_kone()].tyyppi == Peli::SAAPUVA) {
				peli.toiminto = Peli::LAHESTYMIS;
			}
			else if (tmp == "DCT" && peli.koneet[peli.etsi_valittu_kone()].tyyppi == Peli::LAHTEVA) {
				peli.toiminto = Peli::OIKOTIE;
			}
			else if (tmp == "HOLD") {
				peli.toiminto = Peli::ODOTUS;
			}
			else if (tmp == "OFF") {
				peli.toiminto = Peli::POIS;
			}

			//peli.koneet[peli.etsi_valittu_kone()].ota_selvitys(tmp, peli.toiminto);

			Peli::selvitys tmp_selvitys;
			tmp_selvitys.kone_id = peli.etsi_valittu_kone();
			tmp_selvitys.nimi = tmp;
			tmp_selvitys.toiminto = peli.toiminto;
			tmp_selvitys.aika = ohjelma.sekunnit() + 3;

				      //koneet[etsi_valittu_kone()].ota_selvitys(tmp, toiminto);
			peli.selvitykset.push_back(tmp_selvitys);
			peli.lisaa_selvityksia();
			ohjelma.odota(150);

			lukija.tyhjenna();
			peli.toiminto = Peli::TYHJA;
			peli.virheteksti = " ";
		}

		if (peli.kasitellyt >= Asetukset::anna_asetus("vaadittavat_kasitellyt")) {
			peli.koska_uusi_kone = -1;
			if (peli.koneet.size() == 0) {
				/* Peli loppui */
				loppu = true;
			}
		}

		if (peli.etsi_valittu_kone() < 0) {
			peli.ohje = "Valitse kone klikkaamalla";
		}
		else {
			peli.ohje = "Paina toimintonappulaa F5 F6 tai F8 ja anna komento";
		}


		for (unsigned int k = 0; k < peli.selvitykset.size(); ++k) {
			if ((int)peli.selvitykset[k].aika == (int)ohjelma.sekunnit()) {
				peli.koneet[peli.selvitykset[k].kone_id].ota_selvitys(peli.selvitykset[k].nimi, peli.selvitykset[k].toiminto);
				peli.selvitykset.erase(peli.selvitykset.begin() + k);
			}
		}

		peli.tarkista_porrastus();

		while (pelin_kello + peli.ajan_muutos <= ohjelma.sekunnit()) {
			pelin_kello += peli.ajan_muutos;

			peli.hoida_koneet();

			piirretty = false;
		}
		if (piirretty) {
			ohjelma.odota();
		} else {
			view.piirra_peli();
			piirretty = true;
		}
	}

	ohjelma.tyhjenna_syote();
	view.piirra_tilasto();

	for (unsigned int i = 0; i < peli.ajat.size(); ++i) {
		std::clog << peli.ajat[i].tunnus << " " << peli.ajat[i].sisaan << " " << peli.ajat[i].pois << " " << (peli.ajat[i].pois - peli.ajat[i].sisaan) << " " << peli.ajat[i].selvitykset << std::endl;
	}

	if (loppu) {
		ohjelma.odota_nappi();
	}

	return 0;
}

void PeliController::pyyda_atis() {
	Peli::Atis &atis = peli.atis;
	syotteenluku lukija;
	atis.lue_paineet("data/painerajat.txt");
	int toiminto = Peli::LAHTO;

	while (atis.ok == false) {
		peli.syote = lukija.lue_syote();
		ohjelma.odota(20);
		
		if (ohjelma.lue_nappi(Ohjelma::NAPPI_F5)) {
			toiminto = Peli::LAHTO;
		}
		else if (ohjelma.lue_nappi(Ohjelma::NAPPI_F7)) {
			toiminto = Peli::LASKU;
		}
		else if (ohjelma.lue_nappi(Ohjelma::NAPPI_F8)) {
			toiminto = Peli::SIIRTOPINTA;
		}

		if (lukija.anna_viesti().length() > 1 && ohjelma.lue_nappi(Ohjelma::NAPPI_ENTER)) {

			std::vector <kiitotie>::iterator tmp;
			size_t index;

			switch (toiminto) {
			case Peli::LAHTO:
				tmp = std::find(peli.kentta.kiitotiet.begin(), peli.kentta.kiitotiet.end(), lukija.anna_viesti());
				index = std::distance(peli.kentta.kiitotiet.begin(), tmp);

				if (tmp != peli.kentta.kiitotiet.end()) {
					atis.lahtokiitotie = index;
					atis.lahto = lukija.anna_viesti();
					lukija.tyhjenna();
				}
				toiminto = Peli::LASKU;
				break;
			case Peli::LASKU:
				tmp = std::find(peli.kentta.kiitotiet.begin(), peli.kentta.kiitotiet.end(), lukija.anna_viesti());
				index = std::distance(peli.kentta.kiitotiet.begin(), tmp);

				if (tmp != peli.kentta.kiitotiet.end()) {
					atis.laskukiitotie = index;
					atis.lasku = lukija.anna_viesti();
					lukija.tyhjenna();
				}
				toiminto = Peli::SIIRTOPINTA;
				break;
			case Peli::SIIRTOPINTA:
				atis.siirtopinta = apuvalineet::luvuksi<int>(lukija.anna_viesti());
				lukija.tyhjenna();
				break;
			}
		}

		bool siirto_ok = false;
		bool lahto_ok = false;
		bool lasku_ok = false;

		view.piirra_atis(toiminto);

		if (atis.lahtokiitotie > -1 && atis.laskukiitotie > -1 && atis.siirtopinta > -1) {
			peli.ohje = "Tiedot sy�tetty tarkistetaan onko oikein";

			double vastakomponentti_lahto = std::cos(apuvalineet::deg2rad(peli.metar.tuuli) - apuvalineet::deg2rad(peli.kentta.kiitotiet[atis.lahtokiitotie].suunta)) * peli.metar.voimakkuus;
			double vastakomponentti_lasku = std::cos(apuvalineet::deg2rad(peli.metar.tuuli) - apuvalineet::deg2rad(peli.kentta.kiitotiet[atis.laskukiitotie].suunta)) * peli.metar.voimakkuus;

			double siirtopinta = atis.etsi_siirtopinta(peli.metar.paine);

			double max_vasta = 0;
			double vasta;

			for (unsigned int i = 0; i < peli.kentta.kiitotiet.size(); ++i) {
				vasta = std::cos(apuvalineet::deg2rad(peli.metar.tuuli) - apuvalineet::deg2rad(peli.kentta.kiitotiet[i].suunta)) * peli.metar.voimakkuus;

				if (vasta > max_vasta) {
					max_vasta = vasta;
				}
			}

			if (siirtopinta == atis.siirtopinta) {
				siirto_ok = true;
			}
			else {
				peli.ohje = "Siirtopinta v��rin tulisi olla " + apuvalineet::tekstiksi(siirtopinta);
				toiminto = Peli::SIIRTOPINTA;
			}

			if (vastakomponentti_lahto > 0) {
				lahto_ok = true;
			}
			else {
				peli.ohje = "L�ht�kiitotie valittu v��rin";
				toiminto = Peli::LAHTO;
			}

			if (vastakomponentti_lasku > 0) {
				lasku_ok = true;
			}
			else {
				peli.ohje = "Laskukiitotie valittu v��rin";
				toiminto = Peli::LASKU;
			}
		}

		if (siirto_ok && lahto_ok && lasku_ok) {
			atis.ok = true;
			atis.siirtokorkeus = 5000;
		}
	}
}
