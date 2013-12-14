// ohjelma.cpp
#include "ohjelma.hpp"
#include "kuvavarasto.hpp"
#include <iostream>
#include <stdexcept>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>

namespace ohjelma {
	// Staattisia, siis vain t�m�n tiedoston k�ytt��n.
	static SDL_Surface *ruutu;
	static void piirra_kuva(SDL_Surface *kuva, int x, int y, bool keskikohta = false);
	static void piirra_koneet();
	static void piirra_navipisteet();
	static void piirra_lentokentta();
	static void piirra_tilanne();
	static int fontin_koko = 12;
	static void piirra_ohje(std::string ohje);
	static void piirra_metar();
	static void piirra_odottavat();
	std::map <std::string, double> asetukset;

	static TTF_Font* fontti;
	static SDL_Color vari = {50, 50, 50};

	namespace versio {
		int paa = 2;
		int kehitys = 0;
		int rakennus = 1;
	}
}

std::string ohjelma::versio::anna_versio() {
	return apuvalineet::tekstiksi(paa) + "." + apuvalineet::tekstiksi(kehitys) + "." + apuvalineet::tekstiksi(rakennus);
}

// Alustusfunktio.
void ohjelma::alku() {
	std::clog << "ohjelma::alku()" << std::endl;
	lataa_asetukset("data/asetukset.ini");

	// Alustetaan SDL tai heitet��n virhe.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error(SDL_GetError());
	}
	// Avataan ikkuna tai heitet��n virhe.
	ruutu = SDL_SetVideoMode(anna_asetus("ruutu_leveys"), anna_asetus("ruutu_korkeus"), 32, SDL_DOUBLEBUF);

	if (!ruutu) {
		throw std::runtime_error(SDL_GetError());
	}

	// Asetetaan otsikko.
	SDL_WM_SetCaption("ATC-radar", "ATC-radar");

	// Nollataan sekuntilaskuri.
	sekunnit(true);

	if (TTF_Init() != 0) {
		throw std::runtime_error(TTF_GetError());
	}

	fontti = TTF_OpenFont("fontit/arial.ttf", fontin_koko);
	if(fontti == NULL) {
		throw std::runtime_error("Cannot open font");
	}
	SDL_EnableUNICODE(SDL_ENABLE);
}

// Lopetusfunktio.
void ohjelma::loppu() {
	std::clog << "ohjelma::loppu()" << std::endl;

	// Suljetaan SDL.
	SDL_Quit();
}

// Kertoo nollauksesta kuluneiden sekuntien m��r�n.
float ohjelma::sekunnit(bool nollaa) {
	static Uint32 alku;
	Uint32 nyt = SDL_GetTicks();
	if (nollaa) {
		alku = nyt;
	}
	return (nyt - alku) / 1000.0f;
}

// Odottaa lyhyen ajan.
void ohjelma::odota(double ms) {
	SDL_Delay(ms);
}

// Lukee seuraavan napinpainalluksen.
ohjelma::nappi ohjelma::odota_nappi() {
	// Odotellaan, kunnes tulee napinpainallus.
	SDL_Event e;
	while (SDL_WaitEvent(&e)) {
		if (e.type != SDL_KEYDOWN) continue;
		switch (e.key.keysym.sym) {
			case SDLK_ESCAPE: return NAPPI_ESCAPE;
			case SDLK_RETURN: return NAPPI_ENTER;
			case SDLK_LEFT: return NAPPI_VASEN;
			case SDLK_RIGHT: return NAPPI_OIKEA;
			default: return NAPPI_MUU;
		}
	}
	// Jokin meni pieleen!
	throw std::runtime_error(SDL_GetError());
}

// Kertoo napin nykytilan.
bool ohjelma::lue_nappi(nappi n) {
	// K�sket��n SDL:n hoitaa viestit, jolloin sen tieto napeista p�ivittyy.
	SDL_PumpEvents();

	// Tarkistetaan pyydetty nappi.
	Uint8 *napit = SDL_GetKeyState(0);

	if(napit == NULL)
	{
		return false;
	}

	switch (n) {
		case NAPPI_VASEN: return napit[SDLK_LEFT];
		case NAPPI_OIKEA: return napit[SDLK_RIGHT];
		case NAPPI_ENTER: return napit[SDLK_RETURN];
		case NAPPI_ESCAPE: return napit[SDLK_ESCAPE];
		case NAPPI_F5: return napit[SDLK_F5];
		case NAPPI_F7: return napit[SDLK_F7];
		case NAPPI_F8: return napit[SDLK_F8];
		case NAPPI_I: return napit[SDLK_i];
		default: return false;
	}
}

// Tyhjent�� sy�tepuskurin.
void ohjelma::tyhjenna_syote() {
	SDL_Event e;
	while (SDL_PollEvent(&e));
}

// Piirt�� yhden kuvan.
static void ohjelma::piirra_kuva(SDL_Surface *kuva, int x, int y, bool keskikohta) {
	SDL_Rect r = {(Sint16)x, (Sint16)y};

	if (keskikohta) {
		r.x -= kuva->w / 2;
		r.y -= kuva->h / 2;
	}

	SDL_BlitSurface(kuva, 0, ruutu, &r);
}

// Piirt�� valikon.
void ohjelma::piirra_valikko(int pelin_tulos, valikko::valinta valittu) {
	std::clog << "ohjelma::piirra_valikko(tulos, valittu)" << std::endl;

	// Valitaan oikeat kuvat.
	SDL_Surface *kuva_peli 		= image_cache::common().get("kuvat/valikko_peli.png");
	SDL_Surface *kuva_lopetus 	= image_cache::common().get("kuvat/valikko_lopetus.png");

	switch (valittu) {
		case valikko::PELI:
			kuva_peli = image_cache::common().get("kuvat/valikko_peli_valittu.png");
			break;
		case valikko::LOPETUS:
			kuva_lopetus = image_cache::common().get("kuvat/valikko_lopetus_valittu.png");
			break;
	}

	int x = 20, y = 16;
	piirra_kuva(image_cache::common().get("kuvat/tausta_valikko.png"), 0, 0);

	piirra_kuva(kuva_peli, x, y);
	piirra_kuva(kuva_lopetus, x, y + kuva_peli->h + 5);

	// Laitetaan piirustukset esille.
	SDL_Flip(ruutu);
}

// Piirt�� pelin.
void ohjelma::piirra_peli() {
	piirra_kuva(image_cache::common().get("kuvat/tausta_peli.png"), 0, 0);
	piirra_tilanne();

	piirra_koneet();

	piirra_navipisteet();

	piirra_lentokentta();
	piirra_ohje(peli::ohje);
	piirra_odottavat();

	kirjoita_tekstia(peli::syote, 50, 70);

	piirra_metar();

	SDL_Flip(ruutu);
}

void ohjelma::piirra_koneet() {
	Uint32 vari;

	double lentokorkeus;
	double selvityskorkeus;

	Uint32 ok = 0x000000FF;
	Uint32 ng = 0xFF0000FF;
	int t_y = 90;
	for (unsigned int i = 0; i < peli::koneet.size(); ++i) {
		if (peli::koneet[i].odotus == false) {
			apuvalineet::piste loppupiste = apuvalineet::uusi_paikka(peli::koneet[i].paikka, peli::koneet[i].suunta, peli::koneet[i].nopeus * (60.0 / 3600.0));
			apuvalineet::piste hiiri = anna_hiiri();

			if (peli::koneet[i].onko_porrastus) {
				vari = ok;
			} else {
				vari = ng;
			}

			rectangleColor(ruutu, peli::koneet[i].paikka.x-3, peli::koneet[i].paikka.y-3, peli::koneet[i].paikka.x+3, peli::koneet[i].paikka.y+3, vari);
			lineColor(ruutu, peli::koneet[i].paikka.x, peli::koneet[i].paikka.y, loppupiste.x, loppupiste.y, vari);
			circleColor(ruutu, peli::koneet[i].paikka.x, peli::koneet[i].paikka.y, apuvalineet::nm2px(1.5), vari);

			kirjoita_tekstia(peli::koneet[i].kutsutunnus, peli::koneet[i].paikka.x, peli::koneet[i].paikka.y);
			
			kirjoita_tekstia(peli::koneet[i].kutsutunnus, 30, t_y); 
			
			if (peli::koneet[i].reitti.size() == 0) {
				kirjoita_tekstia(apuvalineet::tekstiksi(peli::koneet[i].selvityssuunta), 100, t_y);
			} else {
				kirjoita_tekstia(peli::koneet[i].kohde.nimi, 100, t_y);
			}
			
			kirjoita_tekstia(apuvalineet::tekstiksi(peli::koneet[i].selvityskorkeus), 160, t_y);
			kirjoita_tekstia(apuvalineet::tekstiksi(peli::koneet[i].selvitysnopeus), 210, t_y);
			t_y += 20;

			if (peli::koneet[i].tyyppi == peli::SAAPUVA) {
				//std::clog << "Tulee" << std::endl;
				if ((peli::koneet[i].korkeus / 100) < peli::atis::siirtopinta) {
					lentokorkeus = std::floor(peli::koneet[i].korkeus);
				} else {
					lentokorkeus = std::floor(peli::koneet[i].korkeus / 100);
				}

				if ((peli::koneet[i].selvityskorkeus / 100) < peli::atis::siirtopinta) {
					selvityskorkeus = std::floor(peli::koneet[i].selvityskorkeus);
				} else {
					selvityskorkeus = std::floor(peli::koneet[i].selvityskorkeus / 100);
				}

			} else {
				//std::clog << "Menee" << std::endl;
				if (peli::koneet[i].korkeus < peli::atis::siirtokorkeus) {
					lentokorkeus = std::floor(peli::koneet[i].korkeus);
				} else {
					lentokorkeus = std::floor(peli::koneet[i].korkeus / 100);
				}

				if (peli::koneet[i].selvityskorkeus < peli::atis::siirtokorkeus) {
					selvityskorkeus = std::floor(peli::koneet[i].selvityskorkeus);
				} else {
					selvityskorkeus = std::floor(peli::koneet[i].selvityskorkeus / 100);
				}
			}

			kirjoita_tekstia(apuvalineet::tekstiksi(lentokorkeus) + " / " + apuvalineet::tekstiksi(selvityskorkeus), peli::koneet[i].paikka.x, peli::koneet[i].paikka.y + fontin_koko + 3);

			if (peli::koneet[i].valittu) {
				kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(peli::koneet[i].nopeus)) + " / " + apuvalineet::tekstiksi(peli::koneet[i].selvitysnopeus), peli::koneet[i].paikka.x, peli::koneet[i].paikka.y + (2 * fontin_koko) + 3);
				kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(peli::koneet[i].suunta)) + " / " + apuvalineet::tekstiksi(peli::koneet[i].selvityssuunta), peli::koneet[i].paikka.x, peli::koneet[i].paikka.y + (3 * fontin_koko) + 3);

				if (peli::koneet[i].tyyppi == peli::LAHTEVA) {
					kirjoita_tekstia(peli::koneet[i].ulosmenopiste.nimi, peli::koneet[i].paikka.x, peli::koneet[i].paikka.y + (4 * fontin_koko) + 3);
				}

				switch (peli::toiminto) {
					case peli::SUUNTA:
						kirjoita_tekstia("Anna suunta 0-360 tai navipisteen nimi", 50, 50);
						break;
					case peli::NOPEUS:
						kirjoita_tekstia("Anna nopeus lukuna", 50, 50);
						break;
					case peli::KORKEUS:
						kirjoita_tekstia("Anna korkeus numeroina", 50, 50);
						break;
				}

				lineColor(ruutu, peli::koneet[i].paikka.x, peli::koneet[i].paikka.y, hiiri.x, hiiri.y, vari);
				apuvalineet::vektori vek = apuvalineet::suunta_vektori(peli::koneet[i].paikka, hiiri);

				int x = std::abs(peli::koneet[i].paikka.x + hiiri.x) / 2;
				int y = std::abs(peli::koneet[i].paikka.y + hiiri.y) / 2;

				kirjoita_tekstia(apuvalineet::tekstiksi(std::floor(vek.pituus)) + " / " + apuvalineet::tekstiksi(std::floor(vek.suunta)), x, y);
			}
		}
	}
}

void ohjelma::piirra_navipisteet() {
	for (unsigned int i = 0; i < peli::navipisteet.size(); ++i) {
		apuvalineet::piste tmp = peli::navipisteet[i].paikka;
		kirjoita_tekstia(peli::navipisteet[i].nimi, tmp.x, tmp.y);
		trigonColor(ruutu, tmp.x-3, tmp.y+3, tmp.x+3, tmp.y+3, tmp.x, tmp.y-3, 456);
	}

	for (unsigned int i = 0; i < peli::sisapisteet.size(); ++i) {
		apuvalineet::piste tmp = peli::sisapisteet[i].paikka;
		kirjoita_tekstia(peli::sisapisteet[i].nimi, tmp.x, tmp.y);
		trigonColor(ruutu, tmp.x-3, tmp.y+3, tmp.x+3, tmp.y+3, tmp.x, tmp.y-3, 456);
	}
}

void ohjelma::lataa_asetukset(std::string nimi) {
	std::clog << "ohjelma::lataa_asetukset(" << nimi << ")" << std::endl;
	std::ifstream sisaan(nimi.c_str(), std::ios::in);

	if (!sisaan) {
		throw std::runtime_error("Tiedosto " + nimi + " ei aukea tai puuttuu");
	}

	std::string asetus_nimi;
	double asetus_arvo;

	while (sisaan >> asetus_nimi >> asetus_arvo) {
		ohjelma::asetukset[asetus_nimi] = asetus_arvo;
	}

	sisaan.close();
}

double ohjelma::anna_asetus(std::string asetus) {
	std::map <std::string, double>::iterator onko;

	onko = asetukset.find(asetus);

	if (onko == ohjelma::asetukset.end()) {
		throw std::logic_error("Asetusta " + asetus + " ei ole");
	}

	return asetukset[asetus];
}

bool ohjelma::onko_alueella(const apuvalineet::piste& a, const apuvalineet::piste& b, double sade) {
	double x = std::abs(a.x - b.x);
	double y = std::abs(a.y - b.y);

	double valimatka = std::sqrt((x*x) + (y*y));

	return valimatka < (2 * apuvalineet::nm2px(sade));
}

apuvalineet::piste ohjelma::anna_hiiri() {
	int x, y;

	SDL_GetMouseState(&x, &y);
	apuvalineet::piste tmp;

	tmp.x = x;
	tmp.y = y;

	return tmp;
}

bool ohjelma::lue_hiiri() {
	SDL_PumpEvents();

	if (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1)) {
		return true;
	}

	return false;
}

void ohjelma::piirra_lentokentta() {
	for (unsigned int i = 0; i < peli::kentta.kiitotiet.size(); ++i) {
		lineColor(ruutu, peli::kentta.kiitotiet[i].alkupiste.x, peli::kentta.kiitotiet[i].alkupiste.y, peli::kentta.kiitotiet[i].loppupiste.x, peli::kentta.kiitotiet[i].loppupiste.y, 0x223344FF);
		kirjoita_tekstia(peli::kentta.kiitotiet[i].nimi, peli::kentta.kiitotiet[i].alkupiste.x, peli::kentta.kiitotiet[i].alkupiste.y);

		circleColor(ruutu, peli::kentta.kiitotiet[i].lahestymispiste.x, peli::kentta.kiitotiet[i].lahestymispiste.y, apuvalineet::nm2px(0.5), 0xAAAAAAFF);
	}
}

void ohjelma::kirjoita_tekstia(std::string teksti, int x, int y) {
	if (teksti.length() > 0) {
    	SDL_Surface* tekstipinta = TTF_RenderText_Solid(fontti, teksti.c_str(), vari);

		if (!tekstipinta) {
			throw std::runtime_error(SDL_GetError());
		}

		piirra_kuva(tekstipinta, x, y, false);
    	SDL_FreeSurface(tekstipinta);
	}
}

void ohjelma::piirra_tilanne() {
	std::string teksti = "K�sitellyt " + apuvalineet::tekstiksi(peli::kasitellyt) + std::string("/") + apuvalineet::tekstiksi(anna_asetus("vaadittavat_kasitellyt"));
	kirjoita_tekstia(teksti, ohjelma::anna_asetus("ruutu_leveys") - anna_asetus("info_leveys"), 20);

	teksti = "porrastusvirheet " + apuvalineet::tekstiksi(peli::porrastusvirheet) + std::string("/") + apuvalineet::tekstiksi(anna_asetus("maks_porrastusvirhe"));
	kirjoita_tekstia(teksti, ohjelma::anna_asetus("ruutu_leveys") - anna_asetus("info_leveys"), 40);

	teksti =  "muut virheet " + apuvalineet::tekstiksi(peli::muut_virheet);
	kirjoita_tekstia(teksti, ohjelma::anna_asetus("ruutu_leveys") - anna_asetus("info_leveys"), 60);
}

void ohjelma::piirra_ohje(std::string ohje) {
	kirjoita_tekstia(ohje.c_str(), 50, 30);
}

void ohjelma::piirra_tilasto() {
	std::clog << "ohjelma::piirra_tilasto()" << std::endl;

	piirra_kuva(image_cache::common().get("kuvat/tausta_tilasto.png"), 0, 0);

	int y = 30;
	int x = 200;

	kirjoita_tekstia("kutsutunnus", x, y-15);
	kirjoita_tekstia("alueelle", x+100, y-15);
	kirjoita_tekstia("pois", x+200, y-15);
	kirjoita_tekstia("alueella", x+300, y-15);
	kirjoita_tekstia("selvitykset", x+400, y-15);

	for (unsigned int i = 0; i < peli::ajat.size(); ++i) {
		kirjoita_tekstia(peli::ajat[i].tunnus, x, y);
		kirjoita_tekstia(apuvalineet::tekstiksi(peli::ajat[i].sisaan).c_str(), x+100, y);
		kirjoita_tekstia(apuvalineet::tekstiksi(peli::ajat[i].pois).c_str(), x+200, y);
		kirjoita_tekstia(apuvalineet::tekstiksi(peli::ajat[i].pois - peli::ajat[i].sisaan).c_str(), x+300, y);
		kirjoita_tekstia(apuvalineet::tekstiksi(peli::ajat[i].selvitykset).c_str(), x+400, y);

		y += 15;
	}

	SDL_Flip(ruutu);
}

void ohjelma::piirra_atis(int toiminto) {
	piirra_kuva(image_cache::common().get("kuvat/tausta_atis.png"), 0, 0);

	kirjoita_tekstia(peli::syote, 50, 50);

	switch (toiminto) {
		case peli::LAHTO:
			kirjoita_tekstia("Valitse l�ht�kiitotie", 400, 30);
			break;
		case peli::LASKU:
			kirjoita_tekstia("Valitse laskukiitotie", 400, 30);
			break;
		case peli::SIIRTOPINTA:
			kirjoita_tekstia("Valitse siirtopinta", 400, 30);
			break;
	}

	int y = 70;

	for (unsigned int i = 0; i < peli::kentta.kiitotiet.size(); ++i) {
		kirjoita_tekstia(peli::kentta.kiitotiet[i].nimi, 400, y);
		y += 20;
	}

	piirra_metar();
	piirra_ohje(peli::ohje);

	kirjoita_tekstia("Anna l�ht�- ja laskukiitotie, sek� siirtopinta", 50, 70);
	kirjoita_tekstia("L�ht�kiitotie: " + apuvalineet::tekstiksi(peli::atis::lahto), 50, 90);
	kirjoita_tekstia("Laskukiitotie: " + apuvalineet::tekstiksi(peli::atis::lasku), 50, 110);
	kirjoita_tekstia("Siirtopinta: " + apuvalineet::tekstiksi(peli::atis::siirtopinta), 50, 130);

	SDL_Flip(ruutu);
}

void ohjelma::piirra_metar() {
	std::string tuuli 		= apuvalineet::tekstiksi(peli::metar::tuuli);
	std::string voimakkuus 	= apuvalineet::tekstiksi(peli::metar::voimakkuus);
	std::string paine 		= apuvalineet::tekstiksi(peli::metar::paine);
	std::string nakyvyys 	= apuvalineet::tekstiksi(peli::metar::nakyvyys);
	std::string lampotila 	= apuvalineet::tekstiksi(peli::metar::lampotila);
	std::string kastepiste 	= apuvalineet::tekstiksi(peli::metar::kastepiste);

	tuuli 		= apuvalineet::muuta_pituus(tuuli, 3);
	voimakkuus 	= apuvalineet::muuta_pituus(voimakkuus, 2);

	kirjoita_tekstia(peli::kentta.nimi + " " + tuuli + voimakkuus + "KT " + nakyvyys + " " + lampotila + " / " + kastepiste + " " + paine , 50, 10);
}

static void ohjelma::piirra_odottavat() {
	int y = 120;

	kirjoita_tekstia("Odottavat koneet", anna_asetus("ruutu_leveys") - anna_asetus("info_leveys"), y - fontin_koko - 5);

	for (unsigned int i = 0; i < peli::odottavat.size(); ++i) {
		kirjoita_tekstia(peli::odottavat[i].kutsutunnus, anna_asetus("ruutu_leveys") - 200, y);

		y += fontin_koko + 5;
	}
}
