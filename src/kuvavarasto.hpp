// Otsikot pit�� sis�llytt�� vain kerran.
// T�m� varmistetaan esik��nt�j�ll�.
#ifndef IMAGE_CACHE_HPP
#define IMAGE_CACHE_HPP
#define USE_SDL_IMAGE 1

// K�ytet��n joitakin standardikirjaston luokkia.
#include <string>
#include <map>
#include <stdexcept>

// Tehd��n ensin yleinen cache-luokan malli, jossa ovat yleisp�tev�t funktiot.
// T: s�il�tt�v�st� tyypist� luotu rakenne, jossa ovat staattiset j�senet
//    - typedef  ???  type;       // s�il�tt�v� tyyppi,
//    - void free(type& t);       // funktio arvon vapauttamiseen,
//    - type null();              // funktio, joka palauttaa tyhj�n arvon,
//    - void null(type const& t); // funktio, joka tarkistaa, onko arvo tyhj�.
// K: avaimen tyyppi; oletuksena std::string
// C: tietorakenne tallennukseen; oletuksena std::map<K, T::type>.
template <typename T, typename K = std::string, typename C = std::map<K, typename T::type> > class cache {
    // S�il�j� ei kuulu kopioida, joten kopiomuodostin on yksityinen.
    cache(const cache& t) {
        throw std::logic_error("cache(const cache& t)");
    }
protected:
    // Tyypit: avain, s�il�tt�v� objekti, tietorakenne,
    // tietorakenteen iteraattori sek� t�m� luokka itse.
    typedef K key;
    typedef typename T::type object;
    typedef C container;
    typedef typename container::iterator iterator;
    typedef cache<T, K, C> cache_type;

    // Data aiemmin m��r�tyss� tietorakenteessa (map)
    container data;

    // Muodostin on tyhj�.
    cache() {}
public:
    // J�senen asetus: poistetaan vanha, lis�t��n uusi, jos se ei ole tyhj�.
    void set(const key& k, object t) {
        erase(k);
        if (T::is_null(t)) {
            data[k] = t;
        }
    }
    // Haku: etsit��n ja palautetaan, jos l�ytyi; muuten palautetaan tyhj�.
    object find(const key& name) {
        iterator i = data.find(name);
        if (i == data.end()) {
            return T::null();
        }
        return i->second;
    }
    // Tyhjennys: vapautetaan ja poistetaan kaikki.
    void clear() {
        for (iterator i = data.begin(); i != data.end(); ++i) {
            T::free(i->second);
        }
        data.clear();
    }
    // Poisto nimen perusteella: etsit��n, vapautetaan ja poistetaan.
    // Toisella parametrilla voi kielt�� vapauttamisen, jos jostain syyst�
    // haluaa vain ottaa arvon pois s�il�st� ja huolehtia itse lopusta.
    bool erase(const std::string& key, bool free_data = true) {
        iterator i = data.find(key);
        if (i == data.end()) {
            return false;
        }
        if (free_data) {
            T::free(i->second);
        }
        data.erase(i);
        return true;
    }
    // Poisto datan perusteella: k�yd��n listaa l�pi, poistetaan osuma.
    // Toinen parametri sama kuin yll�.
    bool erase(object t, bool free_data = true) {
        for (iterator i = data.begin(); i != data.end(); ++i) {
            // Jos l�ytyi, vapautetaan ja poistetaan.
            if (i->second == t) {
                if (free_data) {
                    T::free(i->second);
                }
                data.erase(i);
                return true;
            }
        }
        // Ei l�ytynyt.
        return false;
    }
    // Tuhoaja: tyhjennet��n sis�lt�.
    ~cache() {
        clear();
    }
};

// Varsinainen SDL-osuus alkaa t�st�:

#include <SDL/SDL.h>
#ifdef USE_SDL_IMAGE
    // Tuki muillekin kuin BMP-kuville saadaan SDL_image-kirjastosta:
    #include <SDL/SDL_image.h>
#endif

// Toteutetaan SDL_Surface*-tyypille cache-mallin tarvitsema rakenne.
struct image_cache_data_type {
    // Tyyppi: SDL_Surface *.
    typedef SDL_Surface* type;

    // Vapautus: SDL_FreeSurface.
    static void free(type& t) {
        SDL_FreeSurface(t);
    }

    // Tyhj� arvo: 0-osoitin (NULL-osoitin).
    static type null() {
        return 0;
    }

    // Tyhj�n tarkistus: vertailu nollaan.
    static bool is_null(type const& t) {
        return t == 0;
    }
};

// Tehd��n mallista t�m�n tyypin avulla luokka kuville.
class image_cache: public cache<image_cache_data_type> {
    // T�t�k��n ei kuulu kopioida, joten kopiomuodostin on yksityinen.
    image_cache(const image_cache& t): cache_type() {
        throw std::logic_error("image_cache(const image_cache& t)");
    }
public:
    // Muodostin on tyhj�.
    image_cache() {}

    // Latausfunktio: haetaan vanha kuva tai ladataan tiedostosta.
    SDL_Surface *get(const std::string& file) {
        // Etsit��n vanhaa kuvaa, ja jos l�ytyi, palautetaan se.
        SDL_Surface *tmp = find(file);
        if (tmp) {
            return tmp;
        }
        // Ladataan kuva, ja jos ep�onnistui, heitet��n virhe.
        #ifdef USE_SDL_IMAGE
            // SDL_image-kirjastossa on tuki monelle formaatille.
            tmp = IMG_Load(file.c_str());
        #else
            tmp = SDL_LoadBMP(file.c_str());
        #endif
        if (!tmp) {
            throw std::runtime_error(SDL_GetError());
        }
        // Laitetaan kuva s�il��n ja palautetaan se.
        data[file] = tmp;
        return tmp;
    }
    // Staattinen funktio, jossa on staattinen image_cache-olio.
    // Staattinen olio luodaan ohjelman alussa ja tuhotaan lopussa aivan
    // automaattisesti, ja olion tuhoajafunktiossa tuhotaan my�s sen
    // sis�lt�m�t kuvat (~cache => clear => free => SDL_FreeSurface).
    // Funktio palauttaa viittauksen olioon, joten kaikki p��sev�t
    // k�ytt�m��n samaa image_cache-oliota.
    static image_cache& common() {
        static image_cache c;
        return c;
    }
};

#endif
