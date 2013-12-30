#include "asetukset.h"
#include "peliview.hpp"
#include "ohjelma.hpp"
#include <iostream>
#include <stdexcept>

// Piirt�� pelin.
void PeliView::piirra() {
	piirtopinta.piirra_kuva("kuvat/tausta_peli.png", 0, 0);
	piirra_tilanne();

	piirra_koneet();

	piirra_navipisteet();

	piirra_lentokentta();
	piirra_ohje(peli.ohje);
	piirra_odottavat();

	piirtopinta.kirjoita_tekstia(peli.syote, 50, 70);

	piirra_metar();

	piirtopinta.flip();
}

void PeliView::piirra_koneet() {
	unsigned int vari;

	double lentokorkeus;
	double selvityskorkeus;

	unsigned int ok = 0x000000FF;
	unsigned int ng = 0xFF0000FF;

	int listauskorkeus = 90;

	for (unsigned int i = 0; i < peli.koneet.size(); ++i) {
		if (peli.koneet[i].odotus == false) {
			apuvalineet::piste loppupiste = apuvalineet::uusi_paikka(peli.koneet[i].paikka, peli.koneet[i].suunta, peli.koneet[i].nopeus * (60.0 / 3600.0));
			apuvalineet::piste hiiri = ohjelma.anna_hiiri();

			if (peli.koneet[i].onko_porrastus) {
				vari = ok;
			}
			else {
				vari = ng;
			}

			piirtopinta.rectangleColor(peli.koneet[i].paikka.x - 3, peli.koneet[i].paikka.y - 3, peli.koneet[i].paikka.x + 3, peli.koneet[i].paikka.y + 3, vari);
			piirtopinta.lineColor(peli.koneet[i].paikka.x, peli.koneet[i].paikka.y, loppupiste.x, loppupiste.y, vari);
			piirtopinta.circleColor(peli.koneet[i].paikka.x, peli.koneet[i].paikka.y, apuvalineet::nm2px(1.5), vari);

			piirtopinta.kirjoita_tekstia(peli.koneet[i].kutsutunnus, peli.koneet[i].paikka.x, peli.koneet[i].paikka.y);

			piirtopinta.kirjoita_tekstia(peli.koneet[i].kutsutunnus, 30, listauskorkeus);

			if (peli.koneet[i].reitti.size() == 0) {
				if (peli.koneet[i].lahestymisselvitys) {
					piirtopinta.kirjoita_tekstia("ILS", 100, listauskorkeus);
				} else if (peli.koneet[i].oikotie) {
					piirtopinta.kirjoita_tekstia(peli.koneet[i].ulosmenopiste.nimi, 100, listauskorkeus);
				}

				if (!peli.koneet[i].lahestymisselvitys && !peli.koneet[i].oikotie) {
					piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(peli.koneet[i].selvityssuunta), 100, listauskorkeus);
				}
			} else {
				piirtopinta.kirjoita_tekstia(peli.koneet[i].kohde.nimi, 100, listauskorkeus);
			}

			piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(peli.koneet[i].selvityskorkeus), 160, listauskorkeus);
			piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(peli.koneet[i].selvitysnopeus), 210, listauskorkeus);
			listauskorkeus += 20;


			if (peli.koneet[i].tyyppi == Peli::SAAPUVA) {
				//std::clog << "Tulee" << std::endl;
				if ((peli.koneet[i].korkeus / 100) < peli.atis.siirtopinta) {
					lentokorkeus = std::floor(peli.koneet[i].korkeus);
				}
				else {
					lentokorkeus = std::floor(peli.koneet[i].korkeus / 100);
				}

				if ((peli.koneet[i].selvityskorkeus / 100) < peli.atis.siirtopinta) {
					selvityskorkeus = std::floor(peli.koneet[i].selvityskorkeus);
				}
				else {
					selvityskorkeus = std::floor(peli.koneet[i].selvityskorkeus / 100);
				}

			}
			else {
				//std::clog << "Menee" << std::endl;
				if (peli.koneet[i].korkeus < peli.atis.siirtokorkeus) {
					lentokorkeus = std::floor(peli.koneet[i].korkeus);
				}
				else {
					lentokorkeus = std::floor(peli.koneet[i].korkeus / 100);
				}

				if (peli.koneet[i].selvityskorkeus < peli.atis.siirtokorkeus) {
					selvityskorkeus = std::floor(peli.koneet[i].selvityskorkeus);
				}
				else {
					selvityskorkeus = std::floor(peli.koneet[i].selvityskorkeus / 100);
				}
			}

			piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(lentokorkeus) + " / " + apuvalineet::tekstiksi(selvityskorkeus), peli.koneet[i].paikka.x, peli.koneet[i].paikka.y + piirtopinta.get_fontinkoko() + 3);

			if (peli.koneet[i].valittu) {
				piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(peli.koneet[i].nopeus)) + " / " + apuvalineet::tekstiksi(peli.koneet[i].selvitysnopeus), peli.koneet[i].paikka.x, peli.koneet[i].paikka.y + (2 * piirtopinta.get_fontinkoko()) + 3);
				piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(peli.koneet[i].suunta)) + " / " + apuvalineet::tekstiksi(peli.koneet[i].selvityssuunta), peli.koneet[i].paikka.x, peli.koneet[i].paikka.y + (3 * piirtopinta.get_fontinkoko()) + 3);

				if (peli.koneet[i].tyyppi == Peli::LAHTEVA) {
					piirtopinta.kirjoita_tekstia(peli.koneet[i].ulosmenopiste.nimi, peli.koneet[i].paikka.x, peli.koneet[i].paikka.y + (4 * piirtopinta.get_fontinkoko()) + 3);
				}

				switch (peli.toiminto) {
				case apuvalineet::SUUNTA:
					piirtopinta.kirjoita_tekstia(kieli.anna_teksti(Kieli::TEKSTI_OHJE_SUUNTA), 50, 50);
					break;
				case apuvalineet::NOPEUS:
					piirtopinta.kirjoita_tekstia(kieli.anna_teksti(Kieli::TEKSTI_OHJE_NOPEUS), 50, 50);
					break;
				case apuvalineet::KORKEUS:
					piirtopinta.kirjoita_tekstia(kieli.anna_teksti(Kieli::TEKSTI_OHJE_KORKEUS), 50, 50);
					break;
				}

				piirtopinta.lineColor(peli.koneet[i].paikka.x, peli.koneet[i].paikka.y, hiiri.x, hiiri.y, vari);
				apuvalineet::vektori vek = apuvalineet::suunta_vektori(peli.koneet[i].paikka, hiiri);

				int x = std::abs(peli.koneet[i].paikka.x + hiiri.x) / 2;
				int y = std::abs(peli.koneet[i].paikka.y + hiiri.y) / 2;

				piirtopinta.kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(vek.pituus)) + " / " + apuvalineet::tekstiksi(std::floor(vek.suunta)), x, y);
			}
		}
	}
}

void PeliView::piirra_navipisteet() {
	for (unsigned int i = 0; i < peli.navipisteet.size(); ++i) {
		apuvalineet::piste tmp = peli.navipisteet[i].paikka;
		piirtopinta.kirjoita_tekstia(peli.navipisteet[i].nimi, tmp.x, tmp.y);
		piirtopinta.trigonColor(tmp.x - 3, tmp.y + 3, tmp.x + 3, tmp.y + 3, tmp.x, tmp.y - 3, 456);
	}

	for (unsigned int i = 0; i < peli.sisapisteet.size(); ++i) {
		apuvalineet::piste tmp = peli.sisapisteet[i].paikka;
		piirtopinta.kirjoita_tekstia(peli.sisapisteet[i].nimi, tmp.x, tmp.y);
		piirtopinta.trigonColor(tmp.x - 3, tmp.y + 3, tmp.x + 3, tmp.y + 3, tmp.x, tmp.y - 3, 456);
	}
}

void PeliView::piirra_lentokentta() {
	for (unsigned int i = 0; i < peli.kentta.kiitotiet.size(); ++i) {
		piirtopinta.lineColor(peli.kentta.kiitotiet[i].alkupiste.x, peli.kentta.kiitotiet[i].alkupiste.y, peli.kentta.kiitotiet[i].loppupiste.x, peli.kentta.kiitotiet[i].loppupiste.y, 0x223344FF);
		piirtopinta.kirjoita_tekstia(peli.kentta.kiitotiet[i].nimi, peli.kentta.kiitotiet[i].alkupiste.x, peli.kentta.kiitotiet[i].alkupiste.y);

		piirtopinta.circleColor(peli.kentta.kiitotiet[i].lahestymispiste.x, peli.kentta.kiitotiet[i].lahestymispiste.y, apuvalineet::nm2px(0.5), 0xAAAAAAFF);
	}
}

void PeliView::piirra_tilanne() {
	std::string teksti = "K�sitellyt " + apuvalineet::tekstiksi(peli.kasitellyt) + std::string("/") + apuvalineet::tekstiksi(asetukset.anna_asetus("vaadittavat_kasitellyt"));
	piirtopinta.kirjoita_tekstia(teksti, asetukset.anna_asetus("ruutu_leveys") - asetukset.anna_asetus("info_leveys"), 20);

	teksti = "porrastusvirheet " + apuvalineet::tekstiksi(peli.porrastusvirheet) + std::string("/") + apuvalineet::tekstiksi(asetukset.anna_asetus("maks_porrastusvirhe"));
	piirtopinta.kirjoita_tekstia(teksti, asetukset.anna_asetus("ruutu_leveys") - asetukset.anna_asetus("info_leveys"), 40);

	teksti = "muut virheet " + apuvalineet::tekstiksi(peli.muut_virheet);
	piirtopinta.kirjoita_tekstia(teksti, asetukset.anna_asetus("ruutu_leveys") - asetukset.anna_asetus("info_leveys"), 60);
}

void PeliView::piirra_ohje(std::string ohje) {
	piirtopinta.kirjoita_tekstia(ohje.c_str(), 50, 30);
}

void PeliView::piirra_metar() {
	std::string tuuli 		= apuvalineet::tekstiksi(peli.metar.tuuli);
	std::string voimakkuus 	= apuvalineet::tekstiksi(peli.metar.voimakkuus);
	std::string paine 		= apuvalineet::tekstiksi(peli.metar.paine);
	std::string nakyvyys 	= apuvalineet::tekstiksi(peli.metar.nakyvyys);
	std::string lampotila 	= apuvalineet::tekstiksi(peli.metar.lampotila);
	std::string kastepiste 	= apuvalineet::tekstiksi(peli.metar.kastepiste);
	std::string pilvet;

	for (std::map<std::string, int>::iterator it=peli.metar.pilvet.begin(); it!=peli.metar.pilvet.end(); ++it) {
		pilvet += it->first + apuvalineet::tekstiksi(it->second) + " ";
	}

	tuuli = apuvalineet::muuta_pituus(tuuli, 3);
	voimakkuus = apuvalineet::muuta_pituus(voimakkuus, 2);

	piirtopinta.kirjoita_tekstia(peli.kentta.nimi + " " + tuuli + voimakkuus + "KT " + nakyvyys + " " + lampotila + " / " + kastepiste + " " + pilvet + " " + paine, 50, 10);
}

void PeliView::piirra_odottavat() {
	int y = 120;

	if (peli.odottavat.size()) {
		piirtopinta.kirjoita_tekstia("Odottavia koneita " + apuvalineet::tekstiksi(peli.odottavat.size()) + " kpl", asetukset.anna_asetus("ruutu_leveys") - asetukset.anna_asetus("info_leveys"), y - piirtopinta.get_fontinkoko() - 5);
	}
}
