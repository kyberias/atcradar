#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

#include "levelmenu.h"
#include "atisview.h"

class MainWindow : public QDialog {
	Q_OBJECT

public:
	MainWindow()
	{
		levelMenu = new LevelMenu();
        atisView = new AtisView();

		stack = new QStackedWidget();

		stack->addWidget(levelMenu);
		stack->addWidget(atisView);

		QVBoxLayout *layout = new QVBoxLayout;
		layout->addWidget(stack);
		setLayout(layout);

		connect(levelMenu, SIGNAL(levelSelected(int)), this, SLOT(OnLevelSelected(int)));
		connect(atisView, SIGNAL(atisDone()), this, SLOT(OnAtisDone()));
	}

	public slots:
	void OnLevelSelected(int level)
	{
		//close();
		stack->setCurrentIndex(1);
	}

	void OnAtisDone()
	{
		stack->setCurrentIndex(2);
		//close();
	}

private:
	LevelMenu* levelMenu;
	AtisView* atisView;
	QStackedWidget* stack;
};
