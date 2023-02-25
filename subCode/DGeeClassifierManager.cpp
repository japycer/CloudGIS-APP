#include "DGeeClassifierManager.h"
#include <QMessageBox>
#include "DGeeObjectManager.h"
#include "DEELayerShowOptionWin.h"
#include "DPyGeeUtils.h"

DGeeClassifierManager::DGeeClassifierManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	bindSignalsSlots();
}


DGeeClassifierManager::~DGeeClassifierManager()
{
	delete mSetShowParamsWin;
}


void DGeeClassifierManager::initGui()
{
	// 设置eeObj变量名输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	// ui.mLineEditLayerName->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditLayerName_2->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditRoi->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditSamplesName->setValidator(new QRegExpValidator(regVar, this));

	// 初始化分类器
	QStringList classifierNames;
	classifierNames << "amnhMaxent" << "decisionTree" << "decisionTreeEnsemble" << "libsvm" << "minimumDistance" << "smileCart" << "smileGradientTreeBoost" << "smileNaiveBayes" << "smileRandomForest";
	ui.mComboxClassifier->addItems(classifierNames);

	// 初始化ROI
	QStringList roiNames = DGeeObjectManager::Instance()->getGeometrys();
	ui.mComBoxRois->addItems(roiNames);

	// 初始化参数
	ui.mLineEditScale->setText(QString("30"));		// scale
	// 初始化ee.image Layer列表
	bool ok = DPyGeeUtils::runString(QString("import EEPlugin"));
	DPyGeeUtils::getLatestRunOut(ok);
	if (ok) {
		bool ok2 = DPyGeeUtils::runString(QString("print(EEPlugin.utils.getEEImageLayerNames())"));
		if (ok2)
		{
			QStringList eeImgNames = DPyGeeUtils::getLatestRunOut(true).split("&");
			for each (QString str in eeImgNames)
				ui.mComBoxLayerName->addItem(str);
			if (eeImgNames.count() > 0)
				ui.mLineEditLayerName->setText(QString("<%1>").arg(eeImgNames.at(0)));
			updateBandNames();
		}
	}

}


void DGeeClassifierManager::bindSignalsSlots()
{
	// 各种按钮的信号槽连接
	connect(ui.mBtnTrain, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnTainCliecked);
	connect(ui.mBtnClassify, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnClassifyBtnCliecked);
	connect(ui.mBtnTrainAndClassify, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnTrainAndClassifyCliecked);
	connect(ui.mBtnShowParams, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnShowParamsClicked);
	connect(ui.mBtnQuit, &QPushButton::clicked, this, &DGeeClassifierManager::onQuitBtnClicked);
	connect(ui.mBtnAddToMap, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnAddCliecked);
	connect(ui.mBtnExplain, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnExplainClicked);
	connect(ui.mBtnConfusionTable, &QPushButton::clicked, this, &DGeeClassifierManager::onBtnConfusionTableClicked);
	connect(ui.mBtnSummitLayer, &QPushButton::clicked, this, &DGeeClassifierManager::updateBandNames);
	// 波段选择功能的连接
	connect(ui.mComBoxSelectBands, QOverload<int>::of(&QComboBox::activated), [=](int index) {
		QString bandName = ui.mComBoxSelectBands->itemText(index);
		ui.mComBoxSelectBands->removeItem(index);
		ui.mComboxCanbeSelectBands->addItem(bandName);
	});
	connect(ui.mComboxCanbeSelectBands, QOverload<int>::of(&QComboBox::activated), [=](int index) {
		QString bandName = ui.mComboxCanbeSelectBands->itemText(index);
		ui.mComboxCanbeSelectBands->removeItem(index);
		ui.mComBoxSelectBands->addItem(bandName);
	});
}


void DGeeClassifierManager::onQuitBtnClicked()
{
	if (this->parent() == nullptr)
		this->close();
	else
		qobject_cast<QWidget *>(this->parent())->close();
}


void DGeeClassifierManager::onBtnAddCliecked()
{
	QString layerNameToAdd = ui.mLineEditLayerName_2->text();
	if (layerNameToAdd.isEmpty())
	{
		QMessageBox::critical(this, "Add Error", "Please input layer name", QMessageBox::Ok);
		return;
	}
	// TODO
}


void DGeeClassifierManager::onBtnTainCliecked()
{
	bool ok = checkInputsValidity();
	if (!ok) return;
	// TODO
	// def eeTrainSamples(eeImage, eeFc, classfier = None, scale = 30, bands = None):
	QString eeImage = getInputLayerName();
	QString eeFc = ui.mLineEditSamplesName->text();
	QString classfier = ui.mComboxClassifier->currentText();
	if (classfier.isEmpty())
		classfier = QString("None");
	QString scale = ui.mLineEditScale->text();
	QStringList bandsNames;
	for (int i = 0; i < ui.mComBoxSelectBands->count(); i++)
		bandsNames << QString("\"%1\"").arg(ui.mComBoxSelectBands->itemText(i));
	QString bands;
	if (bandsNames.count() == 0)
		bands = QString("None");
	else
		bands = QString("[%1]").arg(bandsNames.join(","));	// bands like : ["B1","B2"...]
	
	// def samplesRegions(eeImage, eeFc, scale = 30, bands = None):
	QString commandSample = QString("_dxl_temp_samples=DUtils.samplesRegions(%1,%2,%3,%4)");
	commandSample = commandSample.arg(eeImage, eeFc, scale, bands);
	bool ok2 = DPyGeeUtils::runString(commandSample);
	if (!ok2)
	{
		QString errMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to sampleRegions", errMsg, QMessageBox::Ok);
		return;
	}
	else {
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_samples"), DGeeObjectManager::GeeObjType::FeatureCollection);
	}

	// def eeTrainSamples(samples, classfier = None) :
	QString commandTrain = QString("_dxl_temp_trained=DUtils.eeTrainSamples(%1,%2)");
	commandTrain = commandTrain.arg(QString("_dxl_temp_samples"), classfier);

	bool ok3 = DPyGeeUtils::runString(commandTrain);
	if (ok3)
	{
		ui.mLineEditTrainedModel->setText("_dxl_temp_trained");
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_trained"), DGeeObjectManager::GeeObjType::Classifier, 1);
	}
	else
	{
		QString errMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail To Train Model", errMsg, QMessageBox::Ok);
		return;
	}
}


void DGeeClassifierManager::onBtnClassifyBtnCliecked()
{
	bool ok = checkInputsValidity();
	if (!ok) return;
	
	QString trainedModel = ui.mLineEditTrainedModel->text();
	if (trainedModel.isEmpty())
	{
		QMessageBox::critical(this, "Fail to classify", "The trained model is unspicified", QMessageBox::Ok);
		return;
	}

	// def eeClassify(eeImage, eeClassifier, bands=None, roi=None):
	QString command("_dxl_classifyImg=DUtils.eeClassify(%1,%2,%3,%4)");
	QString eeImage = getInputLayerName();
	QStringList bandsNames;
	for (int i = 0; i < ui.mComBoxSelectBands->count(); i++)
		bandsNames << QString("\"%1\"").arg(ui.mComBoxSelectBands->itemText(i));
	QString bands;
	if (bandsNames.count() == 0)
		bands = QString("None");
	else
		bands = QString("[%1]").arg(bandsNames.join(","));	// bands like : ["B1","B2"...]
	QString roiName = ui.mLineEditRoi->text();
	if (roiName.isEmpty())
		roiName = QString("None");
	command = command.arg(eeImage, trainedModel, bands, roiName);

	bool ok2 = DPyGeeUtils::runString(command);
	if (ok2)
	{
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_classifyImg"), DGeeObjectManager::GeeObjType::Image, 1);
		ui.mLineEditLayerName_2->setText("_dxl_classifyImg");
		QMessageBox::information(this, "Successful to classify", "Please input layerName to load result to map", QMessageBox::Ok);
	}
	else
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to classify", errMeg, QMessageBox::Ok);
	}
}


void DGeeClassifierManager::onBtnTrainAndClassifyCliecked()
{
	bool ok = checkInputsValidity();
	if (!ok) return;
	// TODO
	// def eeTrainAndClassify(eeImage, eeFc, classfier=None, roi=None, scale=30, bands=None):
	QString command("_dxl_classifyImg=DUtils.eeTrainAndClassify(%1,%2,%3,%4,%5,%6)");
	QString eeImage = getInputLayerName();		// arg 1
	QString eeFc = ui.mLineEditSamplesName->text();	// arg 2
	QString classfier = ui.mComboxClassifier->currentText();	// arg 3
	if (classfier.isEmpty())
		classfier = QString("None");
	QString scale = ui.mLineEditScale->text();	// arg 5
	QStringList bandsNames;
	for (int i = 0; i < ui.mComBoxSelectBands->count(); i++)
		bandsNames << QString("\"%1\"").arg(ui.mComBoxSelectBands->itemText(i));
	QString bands;	// arg6
	if (bandsNames.count() == 0)
		bands = QString("None");
	else
		bands = QString("[%1]").arg(bandsNames.join(","));	// bands like : ["B1","B2"...]
	QString roiName = ui.mLineEditRoi->text();	// arg 4
	if (roiName.isEmpty())
		roiName = QString("None");

	command = command.arg(eeImage, eeFc, classfier, roiName, scale, bands);
	bool ok2 = DPyGeeUtils::runString(command);
	if (ok2)
	{
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_classifyImg"), DGeeObjectManager::GeeObjType::Image, 1);
		ui.mLineEditLayerName_2->setText("_dxl_classifyImg");
		QMessageBox::information(this, "Successful to classify", "Please input layerName to load result to map", QMessageBox::Ok);
	}
	else
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to classify", errMeg, QMessageBox::Ok);
	}
}


bool DGeeClassifierManager::checkInputsValidity()
{
	QString eeLayerName = ui.mLineEditLayerName->text();
	if (eeLayerName.isEmpty())
	{
		QMessageBox::critical(this, "Inputs Error", "Please input layer name", QMessageBox::Ok);
		return false;
	}

	QString samplesName = ui.mLineEditSamplesName->text();
	if (samplesName.isEmpty())
	{
		QMessageBox::critical(this, "Inputs Error", "Please samples variable name", QMessageBox::Ok);
		return false;
	}

	QString roiName = ui.mLineEditRoi->text();

	// TODO
	return true;
}


void DGeeClassifierManager::onBtnShowParamsClicked()
{
	if (mSetShowParamsWin == nullptr)
	{

	}
		mSetShowParamsWin = new DEELayerShowOptionWin(this);


	mSetShowParamsWin->show();
}


void DGeeClassifierManager::updateBandNames()
{
	QString varName = getInputLayerName();
	if (varName.isEmpty()) return;
	bool ok2 = DPyGeeUtils::runString(QString("print(%1.bandNames().getInfo())").arg(varName));
	if (ok2)
	{
		QStringList bandNames = DPyGeeUtils::getLatestRunOut(true).replace("[", "").replace("]", "").split(",");
		if (bandNames.count() > 0)
		{
			ui.mComboxCanbeSelectBands->clear();
			ui.mComBoxSelectBands->clear();
		}
		for each (QString bandName in bandNames)
			ui.mComBoxSelectBands->addItem(bandName);
	}
}


QString DGeeClassifierManager::getInputLayerName()
{
	QString layerName = ui.mLineEditLayerName->text();
	if (layerName.isEmpty())
		return QString("");

	QString varName = layerName;
	if (layerName.startsWith("<"))
	{
		layerName = layerName.replace("<", "").replace(">", "");
		bool ok = DPyGeeUtils::runString(QString("_dxl_temp_layer=EEPlugin.utils.get_layer_by_name(%1)").arg(layerName));
		if (ok)
			varName = QString("_dxl_temp_layer");
	}

	return varName;
}


void DGeeClassifierManager::onBtnExplainClicked()
{
	QString trainedModel = ui.mLineEditTrainedModel->text();
	if (trainedModel.isEmpty())
	{
		QMessageBox::critical(this, "Fail to explain", "The Trained Classifier is unspecified", QMessageBox::Ok);
		return;
	}
	QString command("print(%1.explain())");
	command = command.arg(trainedModel);
	bool ok = DPyGeeUtils::runString(command);
	if (ok)
	{
		QString res = DPyGeeUtils::getLatestRunOut(true);
		ui.mTextEditDetails->setText(res);
		ui.tabWidget->setCurrentIndex(1);
	}
	else
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to explain\n", errMeg, QMessageBox::Ok);
	}
}

void DGeeClassifierManager::onBtnConfusionTableClicked()
{
	// def assessment(trainging, classfier, bands):
	// return dictionary	
	// "Kappa" =>  confusionMatrix	Number
	// "accuracy" => confusionMatrix.accuracy()		2D Matrix
	// "consumerAccuracy" => confusionMatrix.consumersAccuracy()	List
	// "producersAccuracy" => confusionMatrix.producersAccuracy()	List

	QString trainedModel = ui.mLineEditTrainedModel->text();
	if (trainedModel.isEmpty())
	{
		QMessageBox::critical(this, "Fail to print confusion matrix", "Please click \"Train Button(sample and train) at first\"", QMessageBox::Ok);
		return;
	}
	QString command("print(DUtils.assessment(%1,%2,%3))");
	QString classfier = ui.mComboxClassifier->currentText();
	if (classfier.isEmpty())
		classfier = QString("None");
	else
		classfier = QString("ee.Classifier.%1()").arg(classfier);

	QStringList bandsNames;
	for (int i = 0; i < ui.mComBoxSelectBands->count(); i++)
		bandsNames << QString("\"%1\"").arg(ui.mComBoxSelectBands->itemText(i));
	QString bands;
	if (bandsNames.count() == 0)
		bands = QString("None");
	else
		bands = QString("[%1]").arg(bandsNames.join(","));	// bands like : ["B1","B2"...]
	command = command.arg(QString("_dxl_temp_samples"), classfier, bands);

	bool ok = DPyGeeUtils::runString(command);
	if (ok)
	{
		QString res = DPyGeeUtils::getLatestRunOut(true);
		// TODO
		ui.tabWidget->setCurrentIndex(0);
	}
	else
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to print confusion matrix\n", errMeg, QMessageBox::Ok);
	}
}
