#include "DImgCollectionViewer.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "DPyGeeUtils.h"
#include <QMessageBox>
#include <QJsonArray>
#include <QValueAxis>
#include "DGeeObjectManager.h"

DImgCollectionViewer::DImgCollectionViewer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initUI();
	bindSignalsSlots();
}

DImgCollectionViewer::~DImgCollectionViewer()
{
	delete mLineChartView;
}


void DImgCollectionViewer::initUI()
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
	
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(0,0,0,0);

	// 初始化图表并添加
	QChart *chart = new QChart();
	chart->setTitle("ImageCollection Info Statitic");
	chart->setTheme(QChart::ChartThemeBlueCerulean);

	mLineChartView = new QChartView(chart);
	mLineChartView->setRenderHint(QPainter::Antialiasing, true);	// 抗锯齿
	hLayout->addWidget(mLineChartView);

	ui.mMainWidget->setLayout(hLayout);

	// 初始化统计指标
	QStringList strLst;
	strLst << "Max" << "Min" << "Mean" << "StdDev" << "Sum" << "Median";
	ui.mComBoxToCata->addItems(strLst);
	ui.mComBoxFromCata->addItem("CloudRate");

	// 设置lineEdit输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditImgColName->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditROIName->setValidator(new QRegExpValidator(regVar, this));
}


void DImgCollectionViewer::bindSignalsSlots()
{
	connect(ui.mBtnSummit, &QPushButton::clicked, this, &DImgCollectionViewer::onBtnSummitClicked);
	connect(ui.mComBoxTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DImgCollectionViewer::onComBoxThemeCurrentIndexChanged);
	connect(ui.mBtnDraw, &QPushButton::clicked, this, &DImgCollectionViewer::onBtnDrawClicked);
	connect(ui.mBtnAddCustom, &QPushButton::clicked, this, &DImgCollectionViewer::onBtnSummitCustomIndexClecked);
}


void DImgCollectionViewer::onBtnSummitClicked()
{
	QString collectionName = ui.mLineEditImgColName->text();
	if (collectionName.isEmpty()) return;
	
	bool resImport = DPyGeeUtils::runString(QString("from EEPlugin import DUtils"));
	if (!resImport) 
	{
		QMessageBox::critical(this ,"Summit Error", "Fail to run >>> from EEPlugin import DUtils", QMessageBox::Ok);
		return;
	}

	QString strCount;
	bool resCompute = DPyGeeUtils::runString(QString("print(DUtils.getObjCount(%1))").arg(collectionName));
	if (resCompute)
	{
		strCount = DPyGeeUtils::getLatestRunOut(true);
		mCurrentImgsCount = strCount.toInt();
	}
	else
	{
		DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Summit Error", "print(DUtils.getObjCount(%1))", QMessageBox::Ok);
		return;
	}

	if (strCount.compare("0") == 0)
	{
		QMessageBox::critical(this, "Summit Error", QString("%1.size()==0").arg(collectionName), QMessageBox::Ok);
		return;
	}
	
	QString roiName = ui.mLineEditROIName->text();
	bool resRoi = DPyGeeUtils::runString(QString("print(%1)").arg(roiName));
	if (!resRoi)
	{
		QString errorMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Summit Error", errorMsg, QMessageBox::Ok);
		return;
	}
	DPyGeeUtils::getLatestRunOut(true);	//dxl
	// 提交成功，并设置提交按钮颜色为绿色
	mSummitFlag = true;
	mCurrentImgCollName = ui.mLineEditImgColName->text();
	mCurrentROICollName = ui.mLineEditROIName->text();
	//ui.mBtnSummit->setStyleSheet("background-color:Green");
}


void DImgCollectionViewer::onComBoxThemeCurrentIndexChanged(int index)
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

void DImgCollectionViewer::onBtnDrawClicked()
{
	if (!mSummitFlag)
	{
		QMessageBox::critical(this, "Draw Error", "Please clicked summit button and try again", QMessageBox::Ok);
		return;
	}

	if (mCurrentImgsCount >= 100)
	{
		bool resBool = DPyGeeUtils::runString(QString("_temp_drawImgs=%1.limit(100)").arg(mCurrentImgCollName));
		if (resBool)
		{
			mCurrentImgCollName = QString("_temp_drawImgs");
			DGeeObjectManager::Instance()->addOneRecord(QString("_temp_drawImgs"), DGeeObjectManager::GeeObjType::ImageCollection, 100);
		}
		else
		{
			QMessageBox::critical(this, "Draw Error", "The count of imagecollection is greater than 500, and fail to limit", QMessageBox::Ok);
			return;
		}
	}

	if (ui.mComBoxFromCata->currentIndex() == 0)
	{
		QString command("%1.reduceColumns(ee.Reducer.toList(), [\"CLOUD_COVER\"]).get(\"list\").getInfo()");
		command = command.arg(mCurrentImgCollName);

		bool resRunCloudRate = DPyGeeUtils::runString(QString("print(%1)").arg(command));
		if (resRunCloudRate)
		{
			// 将返回值处理为double数组
			QString resLst = DPyGeeUtils::getLatestRunOut(true);	// return like: [23.22, 45.56, 66.66]
			resLst = resLst.replace(" ", "").replace("[", "").replace("]", "");
			QStringList strLst = resLst.split(",");
			QList<double> cloudRateLst;

			double maxValue = 0;
			for (int i = 0; i < strLst.count(); i++)
			{
				double valuee = strLst.at(i).toDouble();
				cloudRateLst << valuee;
				if (valuee > maxValue)
					maxValue = valuee;
			}

			QChart *chart = mLineChartView->chart();
			QLineSeries *series = new QLineSeries(chart);
			series->setName("Cloud Cover");
			chart->addSeries(series);

			for (int i = 0; i < cloudRateLst.count(); i++)
			{
				QPointF temP(i, cloudRateLst.at(i));
				series->append(temP);
			}

			chart->createDefaultAxes();
			chart->axes(Qt::Horizontal).first()->setRange(0, cloudRateLst.count());
			chart->axes(Qt::Vertical).first()->setRange(0, maxValue*1.2);

			// Add space to label to add space between labels and axis
			QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
			Q_ASSERT(axisY);
			axisY->setLabelFormat("%.1f  ");
			return;
		}
		else
		{
			QString errorMsg = DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(this, "Draw Error", errorMsg, QMessageBox::Ok);
			return;
		}
	}
	else
	{
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

		QString command1("print(DUtils.getImgCollectionBandsCountRange(%1))");
		bool resGetBandsCount = DPyGeeUtils::runString(command1.arg(mCurrentImgCollName));
		if (!resGetBandsCount)
		{
			DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(this, "Draw Error", "Fail to get imgCollection bands count", QMessageBox::Ok);
			return;
		}
		QString minMax = DPyGeeUtils::getLatestRunOut(true);
		if (minMax.compare("0") == 0)
		{
			QMessageBox::critical(this, "Draw Error", "The imgCollection bands count == 0", QMessageBox::Ok);
			return;
		}
		QStringList temLst = minMax.split("&");
		if (temLst.length() <= 1) return;
		int min = temLst.at(0).toInt();
		int max = temLst.at(1).toInt();
		if (min == 0 && max == 0)
		{
			QMessageBox::critical(this, "Draw Error", "The imgCollection has 0 bands", QMessageBox::Ok);
			return;
		}

		// currentAlgo like: B1/B2
		QString currentAlgo = ui.mComBoxFromCata->itemData(ui.mComBoxFromCata->currentIndex()).toString().replace(" ", "");
		QRegExp rx("%\\d+%");
		int pos = 0;
		QList<int> nums;
		QStringList numsStr;

		while ((pos = rx.indexIn(currentAlgo, pos)) != -1)
		{
			QString str = rx.cap(0);
			int num = str.replace("%", "").replace("%", "").simplified().toInt();
			if (nums.length() == 0 || nums.indexOf(num) == -1) {
				nums << num;
				numsStr << QString("%1").arg(num);
			}
			currentAlgo.replace(QString("\%%1\%").arg(num), QString("B%1").arg(num));
		}

		QString functionStr = QString("\"%1\"").arg(currentAlgo);
		QString bandsLst = QString("[%1]").arg(numsStr.join(","));
		QString option = ui.mComBoxToCata->currentText();
		QString optionStr = QString("\"%1\"").arg(option);
		QString roi = mCurrentROICollName.isEmpty() ? "None" : mCurrentROICollName;

		// getImgCollectionStatisticLst(eeObject, functionStr, bandsLst, option, scale, roi=None):
		QString commandStatic("print(DUtils.getImgCollectionStatisticLst(%1,%2,%3,%4,%5,%6))");
		commandStatic = commandStatic.arg(mCurrentImgCollName, functionStr, bandsLst, optionStr, scale, roi);
		bool okToCalcul = DPyGeeUtils::runString(commandStatic);
		if (!okToCalcul)
		{
			QString errorMes = DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(this, "Draw Error", errorMes, QMessageBox::Ok);
			return;
		}

		QString calculRes = DPyGeeUtils::getLatestRunOut(true);
		if (calculRes.isEmpty())
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
}


void DImgCollectionViewer::onBtnSummitCustomIndexClecked()
{
	// input like: RVI:&4&/&3&  NDVI:(%5%-%4%)/(%4%+%5%)	ee.ImageCollection('LANDSAT/LC08/C01/T1_SR')  from EEPlugin import Map
	QString str = ui.mLineEditCustomIndex->text().simplified().replace(" ", "");
	
	if (!str.contains(":"))
	{
		QMessageBox::critical(this, "Summit Custom Index Error", "Format is Error", QMessageBox::Ok);
		return;
	}
	QStringList strLst = str.split(":");
	ui.mComBoxFromCata->addItem(strLst.at(0), strLst.at(1));
}


void DImgCollectionViewer::addLineSeries(QList<double> datas, QString seriesName)
{
	QChart *chart = mLineChartView->chart();
	QLineSeries *series = new QLineSeries(chart);
	series->setName(seriesName);
	chart->addSeries(series);

	double maxValue = 0;
	double minValue = 0;
	for (int i = 0; i < datas.count(); i++)
	{
		double valuee = datas.at(i);
		QPointF temP(i, valuee *100);
		series->append(temP);
		if (valuee > maxValue)
			maxValue = valuee;
		if (valuee < minValue)
			minValue = valuee;
	}

	chart->createDefaultAxes();
	chart->axes(Qt::Horizontal).first()->setRange(0, datas.count());
	chart->axes(Qt::Vertical).first()->setRange(minValue * 110, maxValue * 120);

	// Add space to label to add space between labels and axis
	QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
	Q_ASSERT(axisY);
	axisY->setLabelFormat("%.1f  ");
}

void DImgCollectionViewer::setImgCollName(QString& name)
{
	ui.mLineEditImgColName->setText(name);
}

void DImgCollectionViewer::setScale(QString& name)
{
	ui.mLineEditScale->setText(name);
}

void DImgCollectionViewer::setROI(QString& name)
{
	ui.mLineEditROIName->setText(name);
}