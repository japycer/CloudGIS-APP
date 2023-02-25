#pragma once

#include <QWidget>
#include "ui_DLandsatCalibration.h"

class DEELayerShowOptionWin;

class DLandsatCalibration : public QWidget
{
	Q_OBJECT

public:
	DLandsatCalibration(QWidget *parent = Q_NULLPTR);
	~DLandsatCalibration();

private:
	DEELayerShowOptionWin* mEELayerSetParamsWin = nullptr;
	QString mCurrentShowParams;
	QString mCurrentResultName;

private:
	Ui::DLandsatCalibration ui;
	void initGui();
	void binsSignalsAndSlots();
	void onBtnLoadClicked();
	void onBtnQuitClicked();
	void onBtnCalCulateClicked();
	QString transformDatasToString();
};
