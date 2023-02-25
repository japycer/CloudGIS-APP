#pragma once

#include <QWidget>
#include "ui_DTasselledCapTransformation.h"

class DEELayerShowOptionWin;

class DTasselledCapTransformation : public QWidget
{
	Q_OBJECT

public:
	DTasselledCapTransformation(QWidget *parent = Q_NULLPTR);
	~DTasselledCapTransformation();

private:
	Ui::DTasselledCapTransformation ui;
	DEELayerShowOptionWin* mEELayerSetParamsWin = nullptr;
	QString mCurrentShowParams;
	QString mCurrentResultName;

private:
	void initGui();
	void binsSignalsAndSlots();
	void onBtnLoadClicked();
	void onBtnQuitClicked();
	void onBtnCalCulateClicked();
	QString transformDatasToString();
};
