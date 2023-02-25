#include "DImageViewer.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QValueAxis>
#include <QMessageBox>
#include "DPyGeeUtils.h"
#include <QJsonArray>

DImageViewer::DImageViewer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initUI();
	bindSignalsSlots();
}

DImageViewer::~DImageViewer()
{
	QChartView* mLineChartView = nullptr;
	QChartView* mBarChartView = nullptr;
}

void DImageViewer::initUI()
{
	// add items to theme combobox
	ui.mComBoxTheme->addItem("Light", QChart::ChartThemeLight);
	ui.mComBoxTheme->addItem("Blue Cerulean", QChart::ChartThemeBlueCerulean);
	ui.mComBoxTheme->addItem("Dark", QChart::ChartThemeDark);
	ui.mComBoxTheme->addItem("Brown Sand", QChart::ChartThemeBrownSand);
	ui.mComBoxTheme->addItem("Blue NCS", QChart::ChartThemeBlueNcs);
	ui.mComBoxTheme->addItem("Blue Icy", QChart::ChartThemeBlueIcy);
	ui.mComBoxTheme->addItem("Qt", QChart::ChartThemeQt);
	ui.mComBoxTheme->setCurrentIndex(1);

	QHBoxLayout *hLayout1 = new QHBoxLayout();
	hLayout1->setContentsMargins(0, 0, 0, 0);

	// 初始化图表并添加
	QChart *chart = new QChart();
	chart->setTitle("Image Info Statitic");
	chart->setTheme(QChart::ChartThemeBlueCerulean);

	mLineChartView = new QChartView(chart);
	mLineChartView->setRenderHint(QPainter::Antialiasing, true);	// 抗锯齿
	hLayout1->addWidget(mLineChartView);

	ui.mTabLineChart->setLayout(hLayout1);
	// 初始化统计指标
	QStringList strLst;
	strLst << "Max" << "Min" << "Mean" << "StdDev" << "Sum" << "Median";
	ui.mComBoxStatCata->addItems(strLst);

	// 设置lineEdit输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditImgName->setValidator(new QRegExpValidator(regVar, this));
}


void DImageViewer::bindSignalsSlots()
{
	connect(ui.mBtnSummit, &QPushButton::clicked, this, &DImageViewer::onBtnSummitClicked);
	connect(ui.mComBoxTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DImageViewer::onComBoxThemeCurrentIndexChanged);
	connect(ui.mBtnDraw, &QPushButton::clicked, this, &DImageViewer::onBtnDrawClicked);
	connect(ui.tabWidget, &QTabWidget::currentChanged, [=] (int index){
		if (index == 0) return;
		if (!mBarChartView)
		{
			QChart *chart = new QChart();
			chart->setTitle("Image Info Statitic");
			chart->setTheme(QChart::ChartThemeBlueCerulean);

			QHBoxLayout *hLayout2 = new QHBoxLayout();
			hLayout2->setContentsMargins(0, 0, 0, 0);

			mBarChartView = new QChartView(chart);
			mBarChartView->setRenderHint(QPainter::Antialiasing, true);	// 抗锯齿
			hLayout2->addWidget(mBarChartView);

			ui.mTabBarChart->setLayout(hLayout2);
		}
	});
}


void DImageViewer::onComBoxThemeCurrentIndexChanged(int index)
{
	// get the current selected theme
	QChart::ChartTheme theme = static_cast<QChart::ChartTheme>(
		ui.mComBoxTheme->itemData(ui.mComBoxTheme->currentIndex()).toInt());

	mLineChartView->chart()->setTheme(theme);
	QPalette pal = window()->palette();
	if (theme == QChart::ChartThemeLight) {
		pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	else if (theme == QChart::ChartThemeDark) {
		pal.setColor(QPalette::Window, QRgb(0x121218));
		pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
	}
	else if (theme == QChart::ChartThemeBlueCerulean) {
		pal.setColor(QPalette::Window, QRgb(0x40434a));
		pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
	}
	else if (theme == QChart::ChartThemeBrownSand) {
		pal.setColor(QPalette::Window, QRgb(0x9e8965));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	else if (theme == QChart::ChartThemeBlueNcs) {
		pal.setColor(QPalette::Window, QRgb(0x018bba));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	else if (theme == QChart::ChartThemeBlueIcy) {
		pal.setColor(QPalette::Window, QRgb(0xcee7f0));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	else {
		pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	window()->setPalette(pal);
}


void DImageViewer::onBtnSummitClicked()
{
	QString imgName = ui.mLineEditImgName->text();
	if (imgName.isEmpty()) return;

	bool resImport = DPyGeeUtils::runString(QString("from EEPlugin import DUtils"));
	if (!resImport)
	{
		QMessageBox::critical(this, "Summit Error", "Fail to run >>> from EEPlugin import DUtils", QMessageBox::Ok);
		return;
	}

	QString typeObj = DPyGeeUtils::getEEObjType(imgName);
	if (typeObj.compare("Image") != 0)
	{
		QMessageBox::critical(this, "Summit Error", "Only ee.Image Object can be summited", QMessageBox::Ok);
		return;
	}

	QString enviCommand = QString("print(%1.bandNames().size().getInfo())");
	enviCommand = enviCommand.arg(imgName);
	bool enviTest = DPyGeeUtils::runString(enviCommand);
	if (!enviTest)
	{
		QString errorMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Summit Error", errorMsg, QMessageBox::Ok);
		return;
	}
	DPyGeeUtils::getLatestRunOut(true);

	QString roiName = ui.mLineEditROIName->text();
	bool resRoi = DPyGeeUtils::runString(QString("print(%1)").arg(roiName));
	if (!resRoi)
	{
		QString errorMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Summit Error", errorMsg, QMessageBox::Ok);
		return;
	}

	// 提交成功，并设置提交按钮颜色为绿色
	mSummitFlag = true;
	mCurrentImglName = imgName;
	mCurrentROIName = ui.mLineEditROIName->text();
}


void DImageViewer::onBtnDrawClicked()
{
	if (!mSummitFlag) return;
	QString scale = ui.mLineEditScale->text();
	if (scale.isEmpty()) {
		QMessageBox::critical(this, "Draw Error", "Scale is empty", QMessageBox::Ok);
		return;
	}
	double scaleNum = scale.toDouble();
	if (scaleNum == 0)
	{
		QMessageBox::critical(this, "Draw Error", "Scale format is incorrect", QMessageBox::Ok);
		return;
	}

	QString roi = mCurrentROIName.isEmpty() ? "None" : mCurrentROIName;
	QString option = ui.mComBoxStatCata->currentText();
	QString optionStr = QString("\"%1\"").arg(option);
	// getImageStatisticLst(eeObject, option, scale, roi=None):
	QString commandStatic("print(DUtils.getImageStatisticLst(%1,%2,%3,%4))");
	commandStatic = commandStatic.arg(mCurrentImglName, optionStr, scale, roi);
	bool okToCalcul = DPyGeeUtils::runString(commandStatic);
	if (!okToCalcul)
	{
		QString errorMes = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Draw Error", errorMes, QMessageBox::Ok);
		return;
	}
	QString calculRes = DPyGeeUtils::getLatestRunOut(true);
	if (calculRes.compare("[]") == 0)
	{
		QMessageBox::critical(this, "Draw Error", "The calculate result is empty", QMessageBox::Ok);
		return;
	}
	QStringList strLst = calculRes.split("&");
	QList<double> indexDoubleLst;

	for (int i = 0; i < strLst.count(); i++)
	{
		double valuee = strLst.at(i).toDouble();
		indexDoubleLst << valuee;
	}
	addLineSeries(indexDoubleLst, option);
}


void DImageViewer::addLineSeries(QList<double> datas, QString seriesName)
{
	QChart *chart = mLineChartView->chart();
	QLineSeries *series = new QLineSeries(chart);
	series->setName(seriesName);
	chart->addSeries(series);

	double maxValue = 0;
	double minValue = 0;

	for (int i = 0; i < datas.count(); i++)
	{
		double data = datas.at(i);
		QPointF temP(i, data);
		series->append(temP);
		if (data > maxValue)
			maxValue = data;
		if (data < minValue)
			minValue = data;
	}

	chart->createDefaultAxes();
	chart->axes(Qt::Horizontal).first()->setRange(0, datas.count());
	chart->axes(Qt::Vertical).first()->setRange(minValue, maxValue);

	// Add space to label to add space between labels and axis
	QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
	Q_ASSERT(axisY);
	axisY->setLabelFormat("%.2f ");
}


void DImageViewer::updateBarChart(QList<double> datas, QString seriesName)
{
	QChart *chart = mBarChartView->chart();
	QBarSeries *series = new QBarSeries(chart);
	series->setName(seriesName);
	chart->addSeries(series);

	double maxValue = 0;
	double minValue = 0;

	QBarSet *set = new QBarSet(seriesName);
	for (int i = 0; i < datas.count(); i++)
	{
		double data = datas.at(i);
		*set << data;
		if (data > maxValue)
			maxValue = data;
		if (data < minValue)
			minValue = data;
	}
	series->append(set);

	chart->createDefaultAxes();
	chart->axes(Qt::Horizontal).first()->setRange(0, datas.count());
	chart->axes(Qt::Vertical).first()->setRange(minValue, maxValue);

	// Add space to label to add space between labels and axis
	QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
	Q_ASSERT(axisY);
	axisY->setLabelFormat("%.2f ");
}

void DImageViewer::setImageName(QString& name)
{
	ui.mLineEditImgName->setText(name);
}

void DImageViewer::setScale(QString& name)
{
	ui.mLineEditScale->setText(name);
}

void DImageViewer::setROI(QString& name)
{
	ui.mLineEditROIName->setText(name);
}

//if (!alignment) {
//	for (QChartView *chartView : charts)
//		chartView->chart()->legend()->hide();
//}
//else {
//	for (QChartView *chartView : charts) {
//		chartView->chart()->legend()->setAlignment(alignment);
//		chartView->chart()->legend()->show();
//	}
//}