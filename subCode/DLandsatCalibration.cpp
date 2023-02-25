#include "DLandsatCalibration.h"
#include "DEELayerShowOptionWin.h"
#include <QMessageBox>
#include "DPyGeeUtils.h"
#include <QStandardItemModel>

DLandsatCalibration::DLandsatCalibration(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	binsSignalsAndSlots();

	mCurrentShowParams = QString("{}");
}

DLandsatCalibration::~DLandsatCalibration()
{
	delete mEELayerSetParamsWin;
	delete ui.mMainTableView->model();
}


void DLandsatCalibration::initGui()
{
	// 设置lineEdit输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditImgName->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditOutName->setValidator(new QRegExpValidator(regVar, this));

	ui.mComboBoxTransTo->addItem(QStringLiteral("Radiance"));
	ui.mComboBoxTransTo->addItem(QStringLiteral("TOA"));

	// 初始化变化矩阵信息
	QStandardItemModel* viewModel = new QStandardItemModel();
	viewModel->setColumnCount(6);
	viewModel->setRowCount(6);
	viewModel->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Bannd1")));
	viewModel->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Bannd2")));
	viewModel->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Bannd3")));
	viewModel->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Bannd4")));
	viewModel->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Bannd5")));
	viewModel->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Bannd7")));

	ui.mMainTableView->setModel(viewModel);

	ui.mMainTableView->verticalHeader()->hide();
	ui.mMainTableView->setColumnWidth(0, 80);
	ui.mMainTableView->setColumnWidth(1, 80);
	ui.mMainTableView->setColumnWidth(2, 80);
	ui.mMainTableView->setColumnWidth(3, 80);
	ui.mMainTableView->setColumnWidth(4, 80);
	ui.mMainTableView->setColumnWidth(5, 80);

	ui.mMainTableView->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);	// 双击编辑
	ui.mMainTableView->setAlternatingRowColors(true);
}

void DLandsatCalibration::binsSignalsAndSlots()
{
	connect(ui.mBtnLoad, &QPushButton::clicked, [=] {
		if (!mEELayerSetParamsWin)
		{
			mEELayerSetParamsWin = new DEELayerShowOptionWin(this);
			connect(mEELayerSetParamsWin, &DEELayerShowOptionWin::showParamsCreated, [=](QString& params) {
				mCurrentShowParams = params;
			});
		}

		if (mEELayerSetParamsWin)
			mEELayerSetParamsWin->show();
	});
	connect(ui.mBtnLoad, &QPushButton::clicked, this, &DLandsatCalibration::onBtnLoadClicked);
	connect(ui.mBtnQuit, &QPushButton::clicked, this, &DLandsatCalibration::onBtnQuitClicked);
	connect(ui.mBtnCalculate, &QPushButton::clicked, this, &DLandsatCalibration::onBtnCalCulateClicked);
}


void DLandsatCalibration::onBtnCalCulateClicked()
{
	QString inputImgName = ui.mLineEditImgName->text();
	if (inputImgName.isEmpty())
	{
		QMessageBox::critical(this, "Fail to calculate", "The EEImage variable name is empty", QMessageBox::Ok);
		return;
	}
	QString outputName = ui.mLineEditOutName->text();
	if (outputName.isEmpty())
	{
		QMessageBox::critical(this, "Fail to calculate", "The output image variable name is empty", QMessageBox::Ok);
		return;
	}

	QString varType = DPyGeeUtils::getEEObjType(inputImgName);
	if (varType.startsWith(QStringLiteral("ERROR")))
	{
		QMessageBox::critical(this, "Fail to calculate", "Can't get variable type !", QMessageBox::Ok);
		return;
	}

	if (varType.compare(QStringLiteral("Image")) == 0)
	{
		QString transType = ui.mComboBoxTransTo->currentText();
		QString command;
		if (transType.compare(QStringLiteral("Radiance")) == 0)
			command = QString("%1=ee.Algorithms.Landsat.calibratedRadiance(%2)");
		else
			command = QString("%1=ee.Algorithms.Landsat.TOA(%2)");

		command = command.arg(outputName, inputImgName);
		bool ok = DPyGeeUtils::runString(command);
		if (!ok)
		{
			QString errMeg = DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(this, "Fail to calculate !", errMeg, QMessageBox::Ok);
			return;
		}
		mCurrentResultName = outputName;
		return;
	}

	if (varType.compare(QStringLiteral("ImageCollection")) == 0)
	{
		QString transType = ui.mComboBoxTransTo->currentText();
		QString command;
		if (transType.compare(QStringLiteral("Radiance")) == 0)
			command = QString("%1=%2.map(ee.Algorithms.Landsat.calibratedRadiance)");
		else
			command = QString("%1=%2.map(ee.Algorithms.Landsat.TOA)");

		command = command.arg(outputName, inputImgName);
		bool ok = DPyGeeUtils::runString(command);
		if (!ok)
		{
			QString errMeg = DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(this, "Fail to calculate !", errMeg, QMessageBox::Ok);
			return;
		}
		mCurrentResultName = outputName;
		return;
	}
}

void DLandsatCalibration::onBtnLoadClicked()
{
	if (mCurrentResultName.isEmpty())
	{
		QMessageBox::critical(this, "Fail to Load", "Can't find calculate result !", QMessageBox::Ok);
		return;
	}

	QString commandAdd("Map.addLayer(%1,%2,%3)");
	commandAdd = commandAdd.arg(mCurrentResultName, mCurrentShowParams, mCurrentResultName);
	bool okAdd = DPyGeeUtils::runString(commandAdd);
	if (!okAdd)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to calculate", errMeg, QMessageBox::Ok);
		return;
	}
}


void DLandsatCalibration::onBtnQuitClicked()
{
	if (this->parent())
		qobject_cast<QWidget*>(this->parent())->close();
	else
		this->close();
}


// 将所有数据转换为python二维数组字符串形式
QString DLandsatCalibration::transformDatasToString()
{
	QAbstractItemModel* model = ui.mMainTableView->model();
	int columnCount = model->columnCount();
	int rowCount = model->rowCount();

	QStringList resLst;
	for (int i = 0; i < rowCount; i++)
	{
		QStringList strLst;
		for (int j = 0; j < columnCount; j++)
		{
			QModelIndex index = model->index(i, j, QModelIndex());
			strLst << index.data().toString();
		}
		resLst << QString("[%1]").arg(strLst.join(","));
	}

	QString res = QString("[%1]").arg(resLst.join(","));
	return res;
}