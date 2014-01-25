#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

#include <fstream>
#include <iostream>

#include "levelwidget.hpp"
#include "AtisWidget.h"
#include "PeliView.h"
#include "Metar.hpp"
#include "PeliWidget.h"
#include "peli.hpp"
#include "kieli.hpp"
#include "asetukset.h"
#include "pelicontroller.hpp"
#include "QpiirtoPinta.h"

class MainWindow : public QDialog {
	Q_OBJECT
public:
    MainWindow(Kieli& k) : kieli(k) {
        atis = new Atis;
        metar = new Metar;
        asetukset = new Asetukset;

        peli = new Peli(*asetukset, kieli, std::string("EFRO.txt"), *atis, *metar);

        peliView = new PeliView(*peli, kieli, *asetukset, *atis);
        dummyPinta = new QPainterPiirtoPinta;
        peliController = new PeliController(*peli, *asetukset, *dummyPinta, *atis);
        peliWidget = new PeliWidget(*peliView, *peliController, *peli);

        levelMenu = new LevelMenu();
        atisWidget = new AtisWidget(*metar, *atis, *peli);

		stack = new QStackedWidget();

        stack->addWidget(levelMenu);
        stack->addWidget(atisWidget);
        stack->addWidget(peliWidget);

		QVBoxLayout *layout = new QVBoxLayout;
		layout->addWidget(stack);
		setLayout(layout);

		connect(levelMenu, SIGNAL(levelSelected(int)), this, SLOT(OnLevelSelected(int)));
        connect(atisWidget, SIGNAL(atisDone()), this, SLOT(OnAtisDone()));
	}

    void resizeEvent() {
        if (this->width() < 700 || this->height() < 600) {
            this->resize(700, 600);
        }

        asetukset->muuta_asetusta("ruutu_leveys", this->width());
        asetukset->muuta_asetusta("ruutu_korkeus", this->height());
    }

	public slots:
    void OnLevelSelected(int level) {
        stack->setCurrentIndex(1);
        atisWidget->setLevel(level);
        peli->setLevel(level);
	}

    void OnAtisDone() {
        stack->setCurrentIndex(2);
        peli->luo_kone();
        peli->luo_kone();
        peli->luo_kone();
	}

private:
	LevelMenu* levelMenu;
    AtisWidget* atisWidget;
    Peli* peli;
    Atis* atis;

    Metar* metar;
    Asetukset* asetukset;

    Kieli& kieli;

    QStackedWidget* stack;

    PeliView* peliView;
    QPainterPiirtoPinta* dummyPinta;
    PeliController *peliController;
    PeliWidget* peliWidget;
};
