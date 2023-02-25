#pragma once

#include <QWidget>
#include "ui_DDarkPixelAtomosphereCalibration.h"

class DEELayerShowOptionWin;

class DDarkPixelAtomosphereCalibration : public QWidget
{
	Q_OBJECT

public:
	DDarkPixelAtomosphereCalibration(QWidget *parent = Q_NULLPTR);
	~DDarkPixelAtomosphereCalibration();


private:
	Ui::DDarkPixelAtomosphereCalibration ui;
	DEELayerShowOptionWin* mEELayerSetParamsWin = nullptr;
	QString mCurrentShowParams;
	QString mCurrentResultName;

private:
	void initGui();
	void binsSignalsAndSlots();
	void onBtnLoadClicked();
	void onBtnQuitClicked();
	void onBtnCalCulateClicked();
	void onBtnSummitTableEditClicked();
	QString transformDatasToString();
};
