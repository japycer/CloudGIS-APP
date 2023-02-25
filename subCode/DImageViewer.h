#pragma once

#include <QWidget>
#include <QtCharts/QChartGlobal>
#include "ui_DImageViewer.h"

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE
class DImageViewer : public QWidget
{
	Q_OBJECT

public:
	DImageViewer(QWidget *parent = Q_NULLPTR);
	~DImageViewer();
	void setImageName(QString&);
	void setScale(QString&);
	void setROI(QString&);

private:
	Ui::DImageViewer ui;
	QChartView* mLineChartView = nullptr;
	QChartView* mBarChartView = nullptr;
	bool mSummitFlag = false;
	QString mCurrentImglName;
	QString mCurrentROIName;

private:
	void initUI();
	void bindSignalsSlots();
	void onComBoxThemeCurrentIndexChanged(int index);
	void onBtnSummitClicked();
	void onBtnDrawClicked();
	void addLineSeries(QList<double>, QString);
	void updateBarChart(QList<double>, QString);
};
