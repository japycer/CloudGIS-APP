#pragma once

#include <QWidget>
#include <QtCharts/QChartGlobal>
#include "ui_DImgCollectionViewer.h"

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class DImgCollectionViewer : public QWidget
{
	Q_OBJECT

public:
	DImgCollectionViewer(QWidget *parent = Q_NULLPTR);
	~DImgCollectionViewer();
	void setImgCollName(QString&);
	void setScale(QString&);
	void setROI(QString&);

private:
	Ui::DImgCollectionViewer ui;
	QChartView * mLineChartView = nullptr;
	bool mSummitFlag = false;
	QString mCurrentImgCollName;
	QString mCurrentROICollName;
	int mCurrentImgsCount = 0;

private:
	void initUI();
	void bindSignalsSlots();
	void onComBoxThemeCurrentIndexChanged(int index);
	void onBtnSummitClicked();
	void onBtnDrawClicked();
	void onBtnSummitCustomIndexClecked();
	void addLineSeries(QList<double>, QString);
};
