#include "DCustomWindow.h"

DCustomWindow::DCustomWindow(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::CustomizeWindowHint);
	setStyleSheet("background-color:#3a3a3a");
}

DCustomWindow::~DCustomWindow()
{
	delete mTitleBar;
	delete mMainW;
	delete mMainLayout;
}

void DCustomWindow::addMainWidget(QWidget *w)
{
	mMainW = w;
	mMainLayout = new QVBoxLayout();
	mMainLayout->setContentsMargins(0, 0, 0, 0);

	mTitleBar = new DTitleBar();
	mTitleBar->setMaximumHeight(35);
	mTitleBar->setMinimumHeight(35);
	this->installEventFilter(mTitleBar);

	mMainLayout->addWidget(mTitleBar);
	mMainLayout->addWidget(w);
	this->setLayout(mMainLayout);
	this->resize(500,500);
}

void DCustomWindow::setCustomTitle(const QString& title)
{
	if(mTitleBar)
		mTitleBar->setCustomTitle(title);
}