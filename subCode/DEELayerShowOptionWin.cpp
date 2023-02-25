#include "DEELayerShowOptionWin.h"
#include <QFile>
#include "DPyGeeUtils.h"
#include <QCloseEvent>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDialog>

DEELayerShowOptionWin::DEELayerShowOptionWin(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	init();
	connectSignalSlots();
}

DEELayerShowOptionWin::~DEELayerShowOptionWin()
{
}

void DEELayerShowOptionWin::init()
{
	// init image stretch method
	QStringList strList;
	strList << "Custom" << "Stretch:1Theta" << "Stretch:2Theta" << "Stretch:3Theta" << "Stretch:98%" << "Stretch:100%";
	ui.mComBoxStretchMethod->addItems(strList);
	ui.mComBoxStretchMethod->setCurrentIndex(0);

	// set min-max value
	ui.mWidgetGammaValue->setValueRange(1, 100);
	// set default select
	ui.mRadioBtnGamma->setChecked(true);
	ui.mRadioBtnGrayScale->setChecked(true);

	// set default value
	ui.mWidgetGammaValue->setValue(10);
	ui.mWidgetOpacityValue->setValue(100);

	ui.mComBoxRGB_R->setEnabled(false);
	ui.mComBoxRGB_G->setEnabled(false);
	ui.mComBoxRGB_B->setEnabled(false);

	ui.mComBoxPalette->setEnabled(false);
	ui.mRadioBtnPalette->setEnabled(true);
	ui.mLineEditCustomPalette->setEnabled(false);

	ui.mLineEditMinValue->setEnabled(true);
	ui.mLineEditMaxValue->setEnabled(true);
	loadPalette();
}


void DEELayerShowOptionWin::connectSignalSlots()
{
	connect(ui.mRadioBtnGrayScale, &QRadioButton::clicked, [=] {
		ui.mComBoxGrayBands->setEnabled(true);

		ui.mRadioBtnPalette->setEnabled(true);
		ui.mComBoxRGB_R->setEnabled(false);
		ui.mComBoxRGB_G->setEnabled(false);
		ui.mComBoxRGB_B->setEnabled(false);
	});

	connect(ui.mRadioBtnRGB, &QRadioButton::clicked, [=] {
		ui.mComBoxGrayBands->setEnabled(false);

		ui.mRadioBtnGamma->setChecked(true);
		ui.mWidgetGammaValue->setEnable(true);
		ui.mComBoxPalette->setEnabled(false);
		ui.mLineEditCustomPalette->setEnabled(false);

		ui.mRadioBtnPalette->setEnabled(false);
		ui.mComBoxRGB_R->setEnabled(true);
		ui.mComBoxRGB_G->setEnabled(true);
		ui.mComBoxRGB_B->setEnabled(true);
	});

	connect(ui.mComBoxStretchMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		bool flagEnabled = index == 0;

		ui.mLineEditMinValue->setEnabled(flagEnabled);
		ui.mLineEditMaxValue->setEnabled(flagEnabled);
	});

	connect(ui.mRadioBtnGamma, &QRadioButton::clicked, [=] {
		ui.mWidgetGammaValue->setEnable(true);
		ui.mComBoxPalette->setEnabled(false);
		ui.mLineEditCustomPalette->setEnabled(false);
	});

	connect(ui.mRadioBtnPalette, &QRadioButton::clicked, [=] {
		ui.mWidgetGammaValue->setEnable(false);
		ui.mComBoxPalette->setEnabled(true);
		ui.mLineEditCustomPalette->setEnabled(true);
	});

	// show VisibleParams
	connect(ui.mBtnImport, &QPushButton::clicked, [=] {
		QDialog diag;
		QHBoxLayout layout;
		layout.setContentsMargins(5, 5, 5, 5);
		QLineEdit lineEdit;
		lineEdit.setText(createShowParams());
		layout.addWidget(&lineEdit);
		diag.setLayout(&layout);
		diag.exec();
	});

	connect(ui.mBtnCancel, &QPushButton::clicked, this, &QWidget::close);
	connect(ui.mBtnOk, &QPushButton::clicked, [=] {
		emit showParamsCreated(createShowParams());
		this->close();
	});

	connect(ui.mComBoxRGB_R, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
		bandSequenceModified = true;
	});
	connect(ui.mComBoxRGB_G, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
		bandSequenceModified = true;
	});
	connect(ui.mComBoxRGB_B, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
		bandSequenceModified = true;
	});
	connect(ui.mComBoxGrayBands, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
		bandSequenceModified = true;
	});

	connect(ui.mComBoxStretchMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (int index){
		if (bandSequenceModified)
			loadBandStatValues();
		this->getMinMax(index);
	});
}


// 初始化波段，并根据eeLayerName进行最大最小值拉升
void DEELayerShowOptionWin::initParams(QString &bandNames, QString eeLayerName)
{
	ui.mComBoxGrayBands->clear();
	ui.mComBoxRGB_R->clear();
	ui.mComBoxRGB_G->clear();
	ui.mComBoxRGB_B->clear();

	QStringList bandNamesLst = bandNames.split("&");
	for each (QString name in bandNamesLst)
	{
		ui.mComBoxGrayBands->addItem(name);
		ui.mComBoxRGB_R->addItem(name);
		ui.mComBoxRGB_G->addItem(name);
		ui.mComBoxRGB_B->addItem(name);
	}
	ui.mComBoxRGB_R->setCurrentIndex(0);
	ui.mComBoxRGB_G->setCurrentIndex(1);
	ui.mComBoxRGB_B->setCurrentIndex(2);

	mEELayerName = eeLayerName;

}


void DEELayerShowOptionWin::loadPalette()
{
	QString exeRootDir = QCoreApplication::applicationDirPath();

	QString fileFullName = exeRootDir + "/data/paletteInfo.txt";
	QString content;

	QFile file(fileFullName);
	if (file.open(QFile::ReadOnly | QFile::Text))
		content = file.readAll();
	file.close();

	mEEPaletteInfos = content.split(",");

	//QJsonParseError parseJsonErr;
	QString fileFullName2 = exeRootDir + "/data/palettes.json";
	QFile file2(fileFullName2);
	if (file2.open(QFile::ReadOnly | QFile::Text))
	{
		QJsonParseError parseJsonErr;
		QString contt = file2.readAll();
		QJsonDocument document = QJsonDocument::fromJson(contt.toLatin1(), &parseJsonErr);
		
		mEEPalette = document.object();
	}
	file2.close();
	
	for each (QString str in mEEPaletteInfos)
	{
		QStringList tempLst = str.split(":");
		QString pngPath = QString(":/images/pals/%1.png").arg(tempLst.at(0).mid(tempLst.at(0).indexOf(".")+1));
		
		ui.mComBoxPalette->setIconSize(QSize(200,25));
		ui.mComBoxPalette->addItem(QIcon(pngPath), QString(""));
	}
	ui.mComBoxPalette->setCurrentIndex(0);
	QStringList levelsLst = mEEPaletteInfos.at(0).split(":").at(1).split("|");
	for each (QString str2 in levelsLst)
	{
		ui.mComBoxLevels->addItem(str2);
	}
	connect(ui.mComBoxPalette, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		ui.mComBoxLevels->clear();
		QStringList tempLst = mEEPaletteInfos.at(index).split(":").at(1).split("|");
		for each (QString tempStr in tempLst)
		{
			ui.mComBoxLevels->addItem(tempStr);
		}
	});
}


QString DEELayerShowOptionWin::createShowParams()
{
	QString bands = QString("\"bands\":%1");	// eg, ["B1","B2"...]
	QString gamma = QString("\"gamma\":%1");	// range:0.1--10
	QString max = QString("\"max\":%1");	// float
	QString min = QString("\"min\":%1");	// float
	QString opacity = QString("\"opacity\":%1");	
	QString palette1 = QString("\"palette\":%1");	// eg. ["aaaaaa","bbbbbb"...]
	// TODO
	if (ui.mRadioBtnGrayScale->isChecked())
	{
		// bands name
		bands = bands.arg(QString("\"%1\"").arg(ui.mComBoxGrayBands->currentText()));
		// gamma
		if (ui.mRadioBtnGamma->isChecked())
		{
			QString gammaValue = QString::asprintf("%.1f", ui.mWidgetGammaValue->getCurrentValue() * 1.0 / 10);
			gamma = gamma.arg(gammaValue);
		}
		else
		{
			// palette
			if (!(ui.mLineEditCustomPalette->text().isEmpty()))
				palette1 = palette1.arg(ui.mLineEditCustomPalette->text());
			else
			{
				int currentIndex = ui.mComBoxPalette->currentIndex();
				QString url = mEEPaletteInfos.at(currentIndex).split(":").at(0);
				QString level = ui.mComBoxLevels->currentText();
				// 对3层嵌套的字符串json对象进行处理
				QStringList temPaletteLst = mEEPalette[url.split(".").at(0)].toObject()[url.split(".").at(1)].toObject()[level].toVariant().toStringList();
				QStringList temPaletteLst2;
				// 是否颠倒颜色顺序
				if (ui.mBtnReversePalette->isChecked())
				{
					QStringList tempPaletteLstt;
					int countt = temPaletteLst.count();
					for (int i = countt - 1; i >= 0 ; i--)
						tempPaletteLstt << temPaletteLst.at(i);

					for each (QString temStr in tempPaletteLstt)
						temPaletteLst2 << QString("\"%1\"").arg(temStr);
				}
				else
				{
					for each (QString temStr in temPaletteLst)
						temPaletteLst2 << QString("\"%1\"").arg(temStr);
				}
					
				QString temPalette = temPaletteLst2.join(",");
				palette1 = palette1.arg(QString("[%1]").arg(temPalette));
			}
		}
	}
	else
	{
		// bands name
		QString bandsParam = QString("['%1','%2','%3']").arg(ui.mComBoxRGB_R->currentText(), ui.mComBoxRGB_G->currentText(), ui.mComBoxRGB_B->currentText());
		bands = bands.arg(bandsParam);
		// gamma
		QString gammaValue = QString::asprintf("%.1f", ui.mWidgetGammaValue->getCurrentValue() * 1.0 / 10);
		gamma = gamma.arg(gammaValue);
	}
	// min-max
	bool hasMin_Max = !(ui.mLineEditMinValue->text().isEmpty()) && !(ui.mLineEditMaxValue->text().isEmpty());
	if (hasMin_Max)
	{
		QString minValue = ui.mLineEditMinValue->text();
		QString maxValue = ui.mLineEditMaxValue->text();
		//QString minValue = QString::asprintf("%.2f", ui.mLineEditMinValue->text().toFloat());
		//QString maxValue = QString::asprintf("%.2f", ui.mLineEditMaxValue->text().toFloat());
		min = min.arg(minValue);
		max = max.arg(maxValue);
	}
	// opacity
	QString opacityValue = QString::asprintf("%.2f", ui.mWidgetOpacityValue->getCurrentValue() * 1.0/100);
	if (opacityValue.compare("0.00") == 0) opacityValue = QString("0");
	opacity = opacity.arg(opacityValue);

	// start to format params
	QStringList resLst;
	resLst << opacity << bands;
	if (hasMin_Max)
		resLst << min << max;
	if (gamma.compare("\"gamma\":%1") != 0)
		resLst << gamma;
	if (palette1.compare("\"palette\":%1") != 0)
		resLst << palette1;
	
	// return eg. imageVisParam = {"opacity":1,"bands":["B5","B4","B3"],"min":4982.993149077803,"max":13924.799924515268,"gamma":1},
	QString rest = QString("{%1}").arg(resLst.join(","));
	return rest;
}


void DEELayerShowOptionWin::loadBandStatValues()
{
	if (mEELayerName.isEmpty()) return;
	if (ui.mComBoxGrayBands->count() == 0) return;
	if (!bandSequenceModified) return;
	bool ok1 = DPyGeeUtils::runString(QString("from EEPlugin.utils import getEE_minMax_stdDev_percent"));
	if (!ok1) return;

	mEELayerValueLst.clear();
	if (ui.mRadioBtnGrayScale->isChecked())
	{
		QString bandName = QString("'%1'").arg(ui.mComBoxGrayBands->currentText());
		bool ok2 = DPyGeeUtils::runString(QString("print(getEE_minMax_stdDev_percent(%1,%2))").arg(mEELayerName, bandName));
		QString runRes = DPyGeeUtils::getLatestRunOut(ok2);
		if (!ok2) return;
		QString mEELayerValueInfo = runRes.replace("'", "").replace("}", "").replace("{", "").replace(" ", "");
		QStringList valueLst = mEELayerValueInfo.split("&");
		QString bandMin = valueLst.at(0).split(",").at(0).split("=").at(1);
		QString bandMax = valueLst.at(0).split(",").at(1).split("=").at(1);
		QString bandMean = valueLst.at(1).split(",").at(0).split("=").at(1);
		QString bandPrecent = valueLst.at(2).split(",").at(0).split("=").at(1);
		QString bandStdDev = valueLst.at(3).split(",").at(0).split("=").at(1);
		QString bandPrecent2 = valueLst.at(4).split(",").at(0).split("=").at(1);

		mEELayerValueLst << bandMin << bandMax << bandMean << bandPrecent << bandStdDev << bandPrecent2;
	}
	else
	{
		int index1 = ui.mComBoxRGB_R->currentIndex();
		int index2 = ui.mComBoxRGB_G->currentIndex();
		int index3 = ui.mComBoxRGB_B->currentIndex();

		QString bandNameR = QString("'%1'").arg(ui.mComBoxRGB_R->currentText());
		QString bandNameG = QString("'%1'").arg(ui.mComBoxRGB_G->currentText());
		QString bandNameB = QString("'%1'").arg(ui.mComBoxRGB_B->currentText());

		QString bandNames = QString("[%1&&@@]").arg(bandNameR);
		int bandsCount = 0;
		if (index1 != index2) bandsCount++;
		if (index2 != index3) bandsCount++;
		if (index1 != index3) bandsCount++;

		if (index1 != index2)
			bandNames.replace("&&", QString(",%1").arg(bandNameG));
		else
			bandNames.replace("&&", "");
		if ((index2 != index3)&&(index1 != index3))
			bandNames.replace("@@", QString(",%1").arg(bandNameB));
		else
			bandNames.replace("@@", "");

		
		bool ok2 = DPyGeeUtils::runString(QString("print(getEE_minMax_stdDev_percent(%1,%2))").arg(mEELayerName, bandNames));
		QString runRes = DPyGeeUtils::getLatestRunOut(ok2);
		if (!ok2) return;
		QString mEELayerValueInfo = runRes.replace("'", "").replace("}", "").replace("{", "").replace(" ", "");

		QStringList valueLst = mEELayerValueInfo.split("&");
		QString bandMinR = valueLst.at(0).split(",").at(0).split("=").at(1);
		QString bandMaxR = valueLst.at(0).split(",").at(1).split("=").at(1);
		QString bandMeanR = valueLst.at(1).split(",").at(0).split("=").at(1);
		QString bandPrecentR = valueLst.at(2).split(",").at(0).split("=").at(1);
		QString bandStdDevR = valueLst.at(3).split(",").at(0).split("=").at(1);
		QString bandPrecent2R = valueLst.at(4).split(",").at(0).split("=").at(1);

		QString bandMinG;
		QString bandMaxG;
		QString bandMeanG;
		QString bandPrecentG;
		QString bandStdDevG;
		QString bandPrecent2G;

		QString bandMinB;
		QString bandMaxB;
		QString bandMeanB;
		QString bandPrecentB;
		QString bandStdDevB;
		QString bandPrecent2B;

		if (index1 == index2)
		{
			bandMinG = bandMinR;
			bandMaxG = bandMaxR;
			bandMeanG = bandMeanR;
			bandPrecentG = bandPrecentR;
			bandStdDevG = bandStdDevR;
			bandPrecent2G = bandPrecent2R;
		}
		else
		{
			bandMinG = valueLst.at(0).split(",").at(2).split("=").at(1);
			bandMaxG = valueLst.at(0).split(",").at(3).split("=").at(1);
			bandMeanG = valueLst.at(1).split(",").at(1).split("=").at(1);
			bandPrecentG = valueLst.at(2).split(",").at(1).split("=").at(1);
			bandStdDevG = valueLst.at(3).split(",").at(1).split("=").at(1);
			bandPrecent2G = valueLst.at(4).split(",").at(1).split("=").at(1);
		}
		if (index1 == index3)
		{
			bandMinB = bandMinR;
			bandMaxB = bandMaxR;
			bandMeanB = bandMeanR;
			bandPrecentB = bandPrecentR;
			bandStdDevB = bandStdDevR;
			bandPrecent2B = bandPrecent2R;
		}
		else if (index3 == index2)
		{
			bandMinB = bandMinG;
			bandMaxB = bandMaxG;
			bandMeanB = bandMeanG;
			bandPrecentB = bandPrecentG;
			bandStdDevB = bandStdDevG;
			bandPrecent2B = bandPrecent2G;
		}
		else
		{
			bandMinB = valueLst.at(0).split(",").at((bandsCount-1)*2 ).split("=").at(1);
			bandMaxB = valueLst.at(0).split(",").at((bandsCount - 1) * 2+1).split("=").at(1);
			bandMeanB = valueLst.at(1).split(",").at(bandsCount-1).split("=").at(1);
			bandPrecentB = valueLst.at(2).split(",").at(bandsCount - 1).split("=").at(1);
			bandStdDevB = valueLst.at(3).split(",").at(bandsCount - 1).split("=").at(1);
			bandPrecent2B = valueLst.at(4).split(",").at(bandsCount - 1).split("=").at(1);
		}
		QString bandMin = (QStringList() << bandMinR << bandMinG << bandMinB).join("&");
		QString bandMax = (QStringList() << bandMaxR << bandMaxG << bandMaxB).join("&");
		QString bandMean = (QStringList() << bandMeanR << bandMeanG << bandMeanB).join("&");
		QString bandPrecent = (QStringList() << bandPrecentR << bandPrecentG << bandPrecentB).join("&");
		QString bandStdDev = (QStringList() << bandStdDevR << bandStdDevG << bandStdDevB).join("&");
		QString bandPrecent2 = (QStringList() << bandPrecent2R << bandPrecent2G << bandPrecent2B).join("&");
		
		mEELayerValueLst << bandMin << bandMax << bandMean << bandPrecent << bandStdDev << bandPrecent2;
	}
	bandSequenceModified = false;
}


// 获得显示的最大最小值
QString DEELayerShowOptionWin::getMinMax(int& option)
{
	if (mEELayerName.isEmpty()) return QString();
	if (ui.mComBoxGrayBands->count() == 0) return QString();
	if (mEELayerValueLst.count() == 0) return QString();
	// strList << "Custom" << "Stretch:1Theta" << "Stretch:2Theta" << "Stretch:3Theta" << "Stretch:98%" << "Stretch:100%";
	// minMax,mean,precent,stdDev
	if (ui.mRadioBtnGrayScale->isChecked())
	{
		double min = 0;
		double max = 0;
		switch (option)
		{
		// mean theta Stretch
		case 1:
		case 2:
		case 3:
		{
			double mean = mEELayerValueLst.at(2).toDouble();
			double theta = mEELayerValueLst.at(4).toDouble();
			int thetaCount = ui.mComBoxStretchMethod->currentIndex();
			theta = theta * thetaCount;

			min = mean - theta;
			max = mean + theta;
		}
			break;
		// Stretch:98%
		case 4:
		{
			min = mEELayerValueLst.at(3).toDouble();
			max = mEELayerValueLst.at(5).toDouble();
		}
			break;
		case 5:
		case 0:
		{
			min = mEELayerValueLst.at(1).toDouble();
			max = mEELayerValueLst.at(2).toDouble();
		}
			break;
		default:
			break;
		}
		QString minStr = QString::asprintf("%.4f", min);
		ui.mLineEditMinValue->setText(minStr);
		QString maxStr = QString::asprintf("%.4f", max);
		ui.mLineEditMaxValue->setText(maxStr);
		return QString("%1&%2").arg(minStr, maxStr);
	}
	// 3 bands
	else
	{
		QList<double> mins;
		QList<double> maxs;
		for (int i = 0; i < 3; i++)
		{
			double min = 0;
			double max = 0;
			switch (option)
			{
				// mean theta Stretch
			case 1:
			case 2:
			case 3:
			{
				double mean = mEELayerValueLst.at(2).split("&").at(i).toDouble();
				double theta = mEELayerValueLst.at(4).split("&").at(i).toDouble();
				int thetaCount = ui.mComBoxStretchMethod->currentIndex();
				theta = theta * thetaCount;

				min = mean - theta;
				max = mean + theta;
			}
			break;
			// Stretch:98%
			case 4:
			{
				min = mEELayerValueLst.at(3).split("&").at(i).toDouble();
				max = mEELayerValueLst.at(5).split("&").at(i).toDouble();
			}
			break;
			case 5:
			case 0:
			{
				min = mEELayerValueLst.at(1).split("&").at(i).toDouble();
				max = mEELayerValueLst.at(2).split("&").at(i).toDouble();
			}
			break;
			default:
				break;
			}
			mins << min;
			maxs << max;
		}
		QStringList minLst;
		QStringList maxLst;
		for (int j = 0; j < 3; j++)
		{
			QString temStrMin = QString::asprintf("%.4f", mins.at(j));
			QString temStrMax = QString::asprintf("%.4f", maxs.at(j));
			minLst << temStrMin;
			maxLst << temStrMax;
		}

		QString minStr = QString("[%1]").arg(minLst.join(","));
		ui.mLineEditMinValue->setText(minStr);
		QString maxStr = QString("[%1]").arg(maxLst.join(","));
		ui.mLineEditMaxValue->setText(maxStr);
		return QString("%1&%2").arg(minStr, maxStr);
	}
	return QString();
}