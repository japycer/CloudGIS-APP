#include "DDarkPixelAtomosphereCalibration.h"
#include "DEELayerShowOptionWin.h"
#include <QMessageBox>
#include "DPyGeeUtils.h"
#include <QStandardItemModel>
#include "DAlgorithm.h"

DDarkPixelAtomosphereCalibration::DDarkPixelAtomosphereCalibration(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	binsSignalsAndSlots();
}


DDarkPixelAtomosphereCalibration::~DDarkPixelAtomosphereCalibration()
{
	delete mEELayerSetParamsWin;
	delete ui.mMainTableView->model();
}


void DDarkPixelAtomosphereCalibration::initGui()
{
	// 设置lineEdit输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditImgName->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditOutName->setValidator(new QRegExpValidator(regVar, this));

	ui.mComboBoxSensorType->addItem(QStringLiteral("Landsat5"));

	// 初始化变化矩阵信息
	QStandardItemModel* viewModel = new QStandardItemModel();
	viewModel->setColumnCount(6);
	viewModel->setRowCount(7);
	viewModel->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Band1")));
	viewModel->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Band2")));
	viewModel->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Band3")));
	viewModel->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Band4")));
	viewModel->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Band5")));
	viewModel->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Band7")));

	viewModel->setVerticalHeaderItem(0, new QStandardItem(QObject::tr("value")));
	viewModel->setVerticalHeaderItem(1, new QStandardItem(QObject::tr("a1")));
	viewModel->setVerticalHeaderItem(2, new QStandardItem(QObject::tr("b1")));
	viewModel->setVerticalHeaderItem(3, new QStandardItem(QObject::tr("a2")));
	viewModel->setVerticalHeaderItem(4, new QStandardItem(QObject::tr("b2")));
	viewModel->setVerticalHeaderItem(5, new QStandardItem(QObject::tr("a3")));
	viewModel->setVerticalHeaderItem(6, new QStandardItem(QObject::tr("b3")));

	ui.mMainTableView->setModel(viewModel);

	//ui.mMainTableView->verticalHeader()->hide();
	ui.mMainTableView->setColumnWidth(0, 80);
	ui.mMainTableView->setColumnWidth(1, 80);
	ui.mMainTableView->setColumnWidth(2, 80);
	ui.mMainTableView->setColumnWidth(3, 80);
	ui.mMainTableView->setColumnWidth(4, 80);
	ui.mMainTableView->setColumnWidth(5, 80);

	ui.mMainTableView->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);	// 双击编辑
	ui.mMainTableView->setAlternatingRowColors(true);
}


void DDarkPixelAtomosphereCalibration::binsSignalsAndSlots()
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
	connect(ui.mBtnLoad, &QPushButton::clicked, this, &DDarkPixelAtomosphereCalibration::onBtnLoadClicked);
	connect(ui.mBtnQuit, &QPushButton::clicked, this, &DDarkPixelAtomosphereCalibration::onBtnQuitClicked);
	connect(ui.mBtnCalculate, &QPushButton::clicked, this, &DDarkPixelAtomosphereCalibration::onBtnCalCulateClicked);
	connect(ui.mBtnSummitTableEdit, &QPushButton::clicked, this, &DDarkPixelAtomosphereCalibration::onBtnSummitTableEditClicked);
}


void DDarkPixelAtomosphereCalibration::onBtnLoadClicked()
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


void DDarkPixelAtomosphereCalibration::onBtnQuitClicked()
{
	if (this->parent())
		qobject_cast<QWidget*>(this->parent())->close();
	else
		this->close();
}


void DDarkPixelAtomosphereCalibration::onBtnCalCulateClicked()
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

	bool okImport = DPyGeeUtils::runString(QStringLiteral("from EEPlugin import DUtils"));
	if (!okImport)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to calculate", errMeg, QMessageBox::Ok);
		return;
	}

	//def darkPixelAtomosphereCalibration(eeImage, datas) :
	QString caliData = transformDatasToString();
	if (caliData.compare("[[]]") == 0)
	{
		QMessageBox::critical(this, "Fail to calculate", "Please calculate calibretion data at first !", QMessageBox::Ok);
		return;
	}

	QString command("%1=DUtils.darkPixelAtomosphereCalibration(%2,%3)");
	command = command.arg(outputName, inputImgName, caliData);
	bool okRun = DPyGeeUtils::runString(command);
	if (!okRun)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to calculate", errMeg, QMessageBox::Ok);
		return;
	}
	mCurrentResultName = outputName;
}


void DDarkPixelAtomosphereCalibration::onBtnSummitTableEditClicked()
{
	QAbstractItemModel* model = ui.mMainTableView->model();
	int columnCount = model->columnCount();
	QList<float> bandValues;

	for (int j = 0; j < columnCount; j++)
	{
		QModelIndex index = model->index(0, j, QModelIndex());
		bandValues << index.data().toFloat();
	}

	QList<QList<float>> res2D = DAlgorithm::darkPixelCalculate(bandValues);

	QStandardItemModel* standardModel = qobject_cast<QStandardItemModel*>(model);

	for (int i = 1; i < 7; i++)
	{
		for (int j = 0; j < columnCount; j++)
		{
			float data = res2D.at(i-1).at(j);
			QString content = QString::asprintf("%.5f", data);
			standardModel->setItem(i, j, new QStandardItem(content));
		}
	}
}


QString DDarkPixelAtomosphereCalibration::transformDatasToString()
{
	QAbstractItemModel* model = ui.mMainTableView->model();
	int columnCount = model->columnCount();

	QStringList resLst;
	for (int i = 5; i < 7; i++)
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