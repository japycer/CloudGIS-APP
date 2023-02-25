#pragma once

#include <QWidget>
#include "ui_DEELayerShowOptionWin.h"
#include <QJsonObject>

class DEELayerShowOptionWin : public QWidget
{
	Q_OBJECT

public:
	DEELayerShowOptionWin(QWidget *parent = Q_NULLPTR);
	~DEELayerShowOptionWin();
	void initParams(QString &bandNames, QString eeLayerName);

signals:
	void showParamsCreated(QString&);

private:
	Ui::DEELayerShowOptionWin ui;
	QString mEELayerName;
	QStringList mEEPaletteInfos;
	QJsonObject mEEPalette;
	QStringList mEELayerValueLst;

private:
	void init();
	void loadPalette();
	void connectSignalSlots();
	QString createShowParams();
	void loadBandStatValues();
	QString getMinMax(int&);
	bool bandSequenceModified = false;
};
