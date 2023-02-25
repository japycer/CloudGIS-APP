#include "DTasselledCapTransformation.h"
#include "DEELayerShowOptionWin.h"
#include <QMessageBox>
#include "DPyGeeUtils.h"
#include <QStandardItemModel>


DTasselledCapTransformation::DTasselledCapTransformation(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	binsSignalsAndSlots();
}


DTasselledCapTransformation::~DTasselledCapTransformation()
{
	delete mEELayerSetParamsWin;
	delete ui.mMainTableView->model();
}


void DTasselledCapTransformation::initGui()
{
	// 设置lineEdit输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditImgName->setValidator(new QRegExpValidator(regVar, this));
	ui.mLineEditOutName->setValidator(new QRegExpValidator(regVar, this));
	
	ui.mComboBoxSensorType->addItem(QStringLiteral("Landsat5"));

	// 初始化变化矩阵信息
	QStandardItemModel* viewModel = new QStandardItemModel();
	viewModel->setColumnCount(6);
	viewModel->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Bannd1")));
	viewModel->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Bannd2")));
	viewModel->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Bannd3")));
	viewModel->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Bannd4")));
	viewModel->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Bannd5")));
	viewModel->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Bannd6")));

	// band1
	viewModel->setItem(0, 0, new QStandardItem("0.3037"));
	viewModel->setItem(0, 1, new QStandardItem("0.2793"));
	viewModel->setItem(0, 2, new QStandardItem("0.4743"));
	viewModel->setItem(0, 3, new QStandardItem("0.5585"));
	viewModel->setItem(0, 4, new QStandardItem("0.5082"));
	viewModel->setItem(0, 5, new QStandardItem("0.1863"));

	// band2
	viewModel->setItem(1, 0, new QStandardItem("-0.2848"));
	viewModel->setItem(1, 1, new QStandardItem("-0.2435"));
	viewModel->setItem(1, 2, new QStandardItem("-0.5436"));
	viewModel->setItem(1, 3, new QStandardItem("0.7243"));
	viewModel->setItem(1, 4, new QStandardItem("0.0840"));
	viewModel->setItem(1, 5, new QStandardItem("-0.1800"));
	
	// band3
	viewModel->setItem(2, 0, new QStandardItem("0.1509"));
	viewModel->setItem(2, 1, new QStandardItem("0.1973"));
	viewModel->setItem(2, 2, new QStandardItem("0.3279"));
	viewModel->setItem(2, 3, new QStandardItem("0.3406"));
	viewModel->setItem(2, 4, new QStandardItem("-0.7112"));
	viewModel->setItem(2, 5, new QStandardItem("-0.4572"));

	// band4
	viewModel->setItem(3, 0, new QStandardItem("-0.8242"));
	viewModel->setItem(3, 1, new QStandardItem("0.0849"));
	viewModel->setItem(3, 2, new QStandardItem("0.4392"));
	viewModel->setItem(3, 3, new QStandardItem("-0.0580"));
	viewModel->setItem(3, 4, new QStandardItem("0.2012"));
	viewModel->setItem(3, 5, new QStandardItem("-0.2768"));

	// band5
	viewModel->setItem(4, 0, new QStandardItem("-0.3280"));
	viewModel->setItem(4, 1, new QStandardItem("0.0549"));
	viewModel->setItem(4, 2, new QStandardItem("0.1075"));
	viewModel->setItem(4, 3, new QStandardItem("0.1855"));
	viewModel->setItem(4, 4, new QStandardItem("-0.4357"));
	viewModel->setItem(4, 5, new QStandardItem("0.8085"));

	// band6
	viewModel->setItem(5, 0, new QStandardItem("0.1084"));
	viewModel->setItem(5, 1, new QStandardItem("-0.9022"));
	viewModel->setItem(5, 2, new QStandardItem("0.4120"));
	viewModel->setItem(5, 3, new QStandardItem("0.0573"));
	viewModel->setItem(5, 4, new QStandardItem("-0.0251"));
	viewModel->setItem(5, 5, new QStandardItem("0.0238"));

	ui.mMainTableView->setModel(viewModel);
	
	ui.mMainTableView->verticalHeader()->hide();
	ui.mMainTableView->setColumnWidth(0, 80);
	ui.mMainTableView->setColumnWidth(1, 80);
	ui.mMainTableView->setColumnWidth(2, 80);
	ui.mMainTableView->setColumnWidth(3, 80);
	ui.mMainTableView->setColumnWidth(4, 80);
	ui.mMainTableView->setColumnWidth(4, 80);

	ui.mMainTableView->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);	// 双击编辑
	ui.mMainTableView->setAlternatingRowColors(true);
}


void DTasselledCapTransformation::binsSignalsAndSlots()
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
	connect(ui.mBtnLoad, &QPushButton::clicked, this, &DTasselledCapTransformation::onBtnLoadClicked);
	connect(ui.mBtnQuit, &QPushButton::clicked, this, &DTasselledCapTransformation::onBtnQuitClicked);
	connect(ui.mBtnCalculate, &QPushButton::clicked, this, &DTasselledCapTransformation::onBtnCalCulateClicked);
}


void DTasselledCapTransformation::onBtnCalCulateClicked()
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

	//DUtils.TasselledCapTransformate(eeImage, coefficients=None)
	bool okImport = DPyGeeUtils::runString(QStringLiteral("from EEPlugin import DUtils"));
	if (!okImport)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to calculate", errMeg, QMessageBox::Ok);
		return;
	}

	QString coefficients;
	if (ui.mComboBoxSensorType->currentText().compare("Landsat5") != 0)
		coefficients = transformDatasToString();

	QString command("%1=DUtils.TasselledCapTransformate(%2,%3)");
	command = command.arg(outputName, inputImgName, coefficients);
	bool okRun = DPyGeeUtils::runString(command);
	if (!okRun)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to calculate", errMeg, QMessageBox::Ok);
		return;
	}

	//int ok = QMessageBox::warning(this, "Succeed to calculate", "To show the result ?", QMessageBox::Yes | QMessageBox::No);
	//if (ok == QMessageBox::No) return;
	//if (ok)
	mCurrentResultName = outputName;
}


void DTasselledCapTransformation::onBtnLoadClicked()
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
		QMessageBox::critical(this, "Fail to Load", errMeg, QMessageBox::Ok);
		return;
	}

}


void DTasselledCapTransformation::onBtnQuitClicked()
{
	if (this->parent())
		qobject_cast<QWidget*>(this->parent())->close();
	else
		this->close();
}


// 将所有数据转换为python二维数组字符串形式
QString DTasselledCapTransformation::transformDatasToString()
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