#include "DAddDataset.h"
#include "DTitleBar.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qcalendarwidget.h"
#include "qjsondocument.h"
#include "DPyGeeUtils.h"
#include <QPlainTextEdit>
#include <QTextBrowser>
#include "DEELayerShowOptionWin.h"
#include "DImageViewer.h"
#include "DImgCollectionViewer.h"
#include "DGeeObjectManager.h"

DAddDataset::DAddDataset(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initGui();
	bindSignalsSlots();
}


DAddDataset::~DAddDataset()
{
	delete mCalendar;
	delete mShowImgCollectionDetailsWind;
	delete mImageViewer;
	delete mImageCollectionViewer;
	delete mEELayerShowOptionWin;
}

// 初始化GUI界面的布局、和向combo里面添加一些内容
void DAddDataset::initGui()
{
	// 初始化4个添加数据界面的布局
	ui.mVLayoutPage1->setContentsMargins(8, 10, 8, 0);
	ui.mPageAddVector->setLayout(ui.mVLayoutPage1);
	
	ui.mVLayoutPage2->setContentsMargins(8, 10, 8, 0);
	ui.mPageAddRaster->setLayout(ui.mVLayoutPage2);

	ui.mVLayoutPage3->setContentsMargins(8, 10, 8, 0);
	ui.mPageAddWmsLayer->setLayout(ui.mVLayoutPage3);

	ui.mVLayoutPage4->setContentsMargins(8, 10, 8, 0);
	ui.mPageAddGeeLayer->setLayout(ui.mVLayoutPage4);

	QVBoxLayout *vbLaout = new QVBoxLayout();
	vbLaout->addWidget(ui.mListWidgetBaseMap);
	vbLaout->setContentsMargins(8, 10, 8, 0);
	ui.mPageAddBaseMap->setLayout(vbLaout);

	ui.mComboxPage1Encode->addItem(QString("GBK"));
	ui.mComboxPage1Encode->addItem(QString("UTF-8"));
	ui.mComboxPage1Encode->addItem(QString("windows-936"));
	ui.mComboxPage1Encode->setCurrentIndex(2);

	QStringList filterBounds = DGeeObjectManager::Instance()->getGeometrys();
	for each (QString str in filterBounds)
		ui.mComboxBounds->addItem(str);

	// 初始化后处理功能
	QStringList postOperationLst;
	postOperationLst << "NoOperation" << "First" << "Mean" << "Median" << "Mosaic";
	ui.mComboxPostOperation->addItems(postOperationLst);
	ui.mComboxPostOperation->setCurrentIndex(0);
	// 设置snippetID输入规范
	QRegExp reg("ee.Image(Collection)?\\([\"'].*[\"']\\)");
	ui.mLineEditSnippetID->setValidator(new QRegExpValidator(reg, this));
	// 设置eeObj变量名输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditCustomEEFileterbounds->setValidator(new QRegExpValidator(regVar, this));
	// layerName输入规范
	ui.mLineEditEELayerName->setValidator(new QRegExpValidator(regVar, this));
	// 初始化日期编辑框不可编辑
	ui.mListWidget->setCurrentRow(0);
	ui.mMianStackedWidget->setCurrentIndex(0);
	// 后处理框架默认选中第一个
	ui.mRadioBtnPre->setChecked(true);
	// filterBound默认为None
	ui.mComboxBounds->addItem("None");
	ui.mComboxBounds->setCurrentIndex(0);
	// 设置mTab2AddGeeVisual的垂直布局
	ui.mTab2AddGeeVisual->setLayout(ui.mVLayoutVisualize);
}

void DAddDataset::bindSignalsSlots()
{
	// 界面随ListWidget的点击切换
	connect(ui.mListWidget, &QListWidget::currentRowChanged, [=](int index) {
		if (!hasInitPage4AddBaseMap && index == 4)
		{
			this->initPage4AddBaseMap();
			hasInitPage4AddBaseMap = true;
		}
		ui.mMianStackedWidget->setCurrentIndex(index); });
	// 浏览矢量数据
	connect(ui.mBtnBrower1, &QPushButton::clicked, [=] {
		QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "", "*.shp;*.geojson;;*.*");
		ui.mLineEditVectorFileName->setText(filename);
	});
	//浏览栅格数据
	connect(ui.mBtnBrowser2, &QPushButton::clicked, [=] {
		QString filename = QFileDialog::getOpenFileName(this, tr("open vector"), "", "*.tif;*.png;*.jpg;*.TIFF;;*.*");
		ui.mLineEditRaster2->setText(filename);
	});
	// 关闭安妮
	connect(ui.mBtnClose, &QPushButton::clicked, [=] {
		if (this->parent())
		{
			QWidget *ptrParent = qobject_cast<QWidget *>(this->parent());
			ptrParent->close();
		}
	});
	// 添加数据
	connect(ui.mBtnAdd, &QPushButton::clicked, this, &DAddDataset::onAddBtnClicked);
	connect(ui.mBtnLoad, &QPushButton::clicked, ui.mBtnAdd, &QPushButton::clicked);
	// 测试链接的合法性
	connect(ui.mBtnTestLink, &QPushButton::clicked, [=] {
		QString url = ui.mLIneEditWmsURl->text();
		QString layerName = ui.mLineEditWmsName->text();
		bool res;
		bool *ptrRes = &res;
		emit testCurrentLink(url, layerName, ptrRes);
		if (*ptrRes)
			QMessageBox::warning(nullptr, "succeed", "Link is Valid", QMessageBox::Ok);
		else
			QMessageBox::warning(nullptr, "error", "Link is invalid", QMessageBox::Ok);
	});
	// 打开日历控件
	connect(ui.mBtnSetStartTime, &QPushButton::clicked, [=] {
		if (!mCalendar) {
			mCalendar = new QCalendarWidget();
			mCalendar->setStyleSheet("QCalendarWidget QSpinBox{argin:2px;}QCalendarWidget QToolButton::menu - indicator{image:None;}\
		QCalendarWidget QTableView{border - width:0px;}QCalendarWidget QWidget{border:1px solid #242424;border - width:1px 1px 0px 1px;\
		background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #484848,stop:1 #383838);}");
			connect(mCalendar, &QCalendarWidget::selectionChanged, [=] {writeSelectionDate(); });
		}
		mIsLeft = true;
		mCalendar->show();

	});
	connect(ui.mBtnSetEndTime, &QPushButton::clicked, [=] {
		if (!mCalendar) {
			mCalendar = new QCalendarWidget();
			mCalendar->setStyleSheet("QCalendarWidget QSpinBox{argin:2px;}QCalendarWidget QToolButton::menu - indicator{image:None;}\
		QCalendarWidget QTableView{border - width:0px;}QCalendarWidget QWidget{border:1px solid #242424;border - width:1px 1px 0px 1px;\
		background:qlineargradient(spread : pad,x1 : 0,y1 : 0,x2 : 0,y2 : 1,stop : 0 #484848,stop:1 #383838);}");
			connect(mCalendar, &QCalendarWidget::selectionChanged, [=] {writeSelectionDate(); });
		}
		mIsLeft = false;
		mCalendar->show();

	});
	// ee
	//connect(ui.mLineEditSnippetID, &QLineEdit::editingFinished, this, &DAddDataset::slotInputEESnippetIDFinished);
	connect(ui.mBtnPageGeeSummit, &QPushButton::clicked, this, &DAddDataset::onGeeSummitBtnClicked);
	connect(ui.mBtnPage3GeeBandDetails, &QPushButton::clicked, this, &DAddDataset::onShowBandDetailsBtnClicked);
	connect(ui.mBtnPageGeeSummitFilter, &QPushButton::clicked, this, &DAddDataset::onSummitFilterBtnClicked);
	// band select
	connect(ui.mComboxSelectedBands, QOverload<int>::of(&QComboBox::activated), [=](int index) {
		QString bandName = ui.mComboxSelectedBands->itemText(index);
		ui.mComboxSelectedBands->removeItem(index);
		ui.mComboxSelectBands->addItem(bandName);
	});
	connect(ui.mComboxSelectBands, QOverload<int>::of(&QComboBox::activated), [=](int index) {
		QString bandName = ui.mComboxSelectBands->itemText(index);
		ui.mComboxSelectBands->removeItem(index);
		ui.mComboxSelectedBands->addItem(bandName);
	});
	// set show params
	connect(ui.mBtnSetShowParams, &QPushButton::clicked, [=] {
		if (!mEELayerShowOptionWin)
		{
			//if (mCurrentShowParams.isEmpty()) runPostOperation();

			delete mEELayerShowOptionWin;
			mEELayerShowOptionWin = new DEELayerShowOptionWin();

			// 构造波段名
			int temCount = ui.mComboxSelectedBands->count();
			QStringList selectedNames;
			for (int i = 0; i < temCount; i++)
				selectedNames << ui.mComboxSelectedBands->itemText(i);

			QString tem = selectedNames.join("&");
			mEELayerShowOptionWin->initParams(tem, QString("_dxl_temp_postOperation"));
			//mPage4ShowParams
			connect(mEELayerShowOptionWin, &DEELayerShowOptionWin::showParamsCreated, [=](QString&params) {
				mCurrentShowParams = params;
				ui.mPage4ShowParams->setText(ui.mPage4ShowParams->text() + "@");	// 如果已经设置了参数，label加上一个@标志
			});
		}

		mEELayerShowOptionWin->show();
	});
	// post operation
	connect(ui.mComboxPostOperation, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] {
		this->runPostOperation();
	});

	// switch to visualize interface
	connect(ui.mTabWidgetAddGee, &QTabWidget::currentChanged, [=](int index) {
		if (index == 0) return;
		if (!mImageCollectionViewer)
		{
			mImageCollectionViewer = new DImgCollectionViewer();
			mImageCollectionViewer->setImgCollName(QString("_dxl_temp_collection"));
			QVBoxLayout* temLayout = new QVBoxLayout();
			temLayout->addWidget(mImageCollectionViewer);
			ui.mVisualizePageLineChart->setLayout(temLayout);
		}
	});

	// switch to post operation visualize
	connect(ui.mRadioBtnPostOperation, &QRadioButton::clicked, [=] {
		if (!mImageViewer)
		{
			mImageViewer = new DImageViewer();
			mImageViewer->setImageName(QString("_dxl_temp_postOperation"));
			QVBoxLayout* temLayout = new QVBoxLayout();
			temLayout->addWidget(mImageViewer);
			ui.mVisualizePageBarChart->setLayout(temLayout);
		}
		ui.mStackedWidgetVisualize->setCurrentIndex(1);
	});
	// switch to pre operation visualize
	connect(ui.mRadioBtnPre, &QRadioButton::clicked, [=] {
		ui.mStackedWidgetVisualize->setCurrentIndex(0);

	});
	connect(ui.mRadioBtnPostFilter, &QRadioButton::clicked, [=] {
		ui.mStackedWidgetVisualize->setCurrentIndex(0);
		mImageCollectionViewer->setImgCollName(QString("_dxl_temp_filterCol"));
	});
}


void DAddDataset::slotInputEESnippetIDFinished()
{
	QString snippetID = ui.mLineEditSnippetID->text();

}

// 由于Page4涉及较多图片的加载，只在用户切换到该界面时加载
void DAddDataset::initPage4AddBaseMap()
{
	// 初始化baseMap list
	QString exeRootDir = QCoreApplication::applicationDirPath();	//exe所在路径
	QString fullName = exeRootDir + "/data/imagery.json";
	QFile file(fullName);
	QString content;
	if (file.open(QFile::ReadOnly | QFile::Text))
		content = file.readAll();
	else
	{
		file.close();
		return;
	}
	file.close();
	QJsonParseError error;
	QJsonDocument document = QJsonDocument::fromJson(content.toLatin1(), &error);
	mItemList = document.toVariant().toList();

	//number of search result's row, the first row is title
	ui.mListWidgetBaseMap->setSelectionMode(QAbstractItemView::SingleSelection);	// 只能选中一个
	ui.mListWidgetBaseMap->setViewMode(QListWidget::IconMode);//设置内容为图片
	ui.mListWidgetBaseMap->setIconSize(QSize(142, 95));//设置图片的大小
	ui.mListWidgetBaseMap->setSpacing(10);//设置每个item之间的间隔大小
	ui.mListWidgetBaseMap->setMovement(QListWidget::Static);//设置每个item不可拖动
	ui.mListWidgetBaseMap->setResizeMode(QListWidget::Adjust);//设置拖动时可以重新排列

	int rowCnt = mItemList.count();
	for (size_t i = 0; i < rowCnt; i++)
	{
		QVariantMap map = mItemList[i].toMap();
		QString name = map["name"].toString().simplified();
		QString url = map["url"].toString().simplified();

		QString imgPath = ":/images/imagery/" + name + ".png";
		QPixmap pixmap = QPixmap::fromImage(QImage(imgPath));
		QListWidgetItem* item = new QListWidgetItem(QIcon(pixmap), name);//从图片构造item
		item->setSizeHint(QSize(142, 115));//设置每个item的大小
		item->setTextAlignment(Qt::AlignCenter);//设置item居中对齐
		ui.mListWidgetBaseMap->addItem(item);
	}
}

void DAddDataset::writeSelectionDate()
{
	QDate date = mCalendar->selectedDate();
	QString strDate = date.toString("yyyy-MM-dd");
	if (mIsLeft)
		ui.mLineEditFromTime->setText(strDate);
	else
		ui.mLineEditEndTime->setText(strDate);
	
	mCalendar->hide();
}

// 点击添加图层按钮点击响应函数
void DAddDataset::onAddBtnClicked()
{
	int currentIndex = ui.mListWidget->currentIndex().row();

	// 根据当前所处于的界面，判断即将添加图层的类型
	switch (currentIndex)
	{
	// 矢量图层
	case 0:
	{
		QString fileName = ui.mLineEditVectorFileName->text();
		QString dataEncode = ui.mComboxPage1Encode->currentText();
		emit addVectorLayer(fileName, dataEncode, "ogr");
	}
	break;
	// 栅格图层
	case 1:
	{
		QString fileName = ui.mLineEditRaster2->text();
		emit addRasterLayer(fileName);
	}
	break;
	// wms图层
	case 2:
	{
		QString url = ui.mLIneEditWmsURl->text();
		QString layerName = ui.mLineEditWmsName->text();
		bool res;
		bool *ptrRes = &res;
		emit testCurrentLink(url, layerName, ptrRes);
		if (*ptrRes)
			emit addWmsLayer();
	}
	break;
	// eeLayer图层
	case 3:
	{
		if (ui.mLineEditEELayerName->text().isEmpty())
		{
			QMessageBox::critical(this, "Load Error", "Layer Name is empty", QMessageBox::Ok);
			return;
		}

		if (!mSummitGeeFlag) return;
		if (ui.mComboxSelectedBands->count() == 0) return;
		if (!succeedSummitFilters)
		{
			DPyGeeUtils::runString(QString("_dxl_temp_postOperation=_dxl_temp_collection"));
			DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_postOperation"));
		}


		bool ok = DPyGeeUtils::runString(QString("print(_dxl_temp_filterCol.size().getInfo())"));
		if (!ok)
			QMessageBox::critical(this, "Fail to add Layer", "Fail to run print(_dxl_temp_filterCol.size().getInfo()) in DaddDataset.cpp", QMessageBox::Ok);
		else
		{
			QString sizeCount = DPyGeeUtils::getLatestRunOut(true).toInt();
			if (sizeCount == 0)
			{
				QMessageBox::critical(this, "Fail to add Layer", "imgCollection.size()=0, please modify filter parameters to try again", QMessageBox::Ok);
				return;
			}
		}
		QString layerName = QString("\"%1\"").arg(ui.mLineEditEELayerName->text());
		if (layerName.isEmpty())
		{
			QMessageBox::critical(this, "Fail to add Layer", "LayerName is empty", QMessageBox::Ok);
			return;
		}

		if (mCurrentShowParams.isEmpty())
			mCurrentShowParams = QString("{}");

		QString command = QString("Map.addLayer(%1,%2,%3)").arg(QString("_dxl_temp_postOperation"), mCurrentShowParams, layerName);
		bool okAdd = DPyGeeUtils::runString(command);
		if (!okAdd)
			QMessageBox::critical(this, "Fail to add Layer", "Fail to run Map.addLayer in DaddDataset.cpp", QMessageBox::Ok);
		else
		{
			DPyGeeUtils::runString(QString("%1=_dxl_temp_postOperation").arg(ui.mLineEditEELayerName->text()));
			DGeeObjectManager::Instance()->addOneRecord(QString(ui.mLineEditEELayerName->text()));
		}
	}
	break;
	// base map
	case 4:
	{
		int selectedRow = ui.mListWidgetBaseMap->currentIndex().row();
		if (selectedRow < 0)return;
		QVariantMap map = mItemList[selectedRow].toMap();
		QString name = map["name"].toString().simplified();
		QString url = map["url"].toString().simplified();
		bool res;
		bool *ptrRes = &res;
		emit testCurrentLink(url, name, ptrRes);
		if (*ptrRes)
			emit addWmsLayer();
	}
	break;
	default:
		break;
	}
}


void DAddDataset::setCurrentIndex(int index)
{
	ui.mListWidget->setCurrentRow(index);
}


bool DAddDataset::compareDateBetween()
{
	QString strStartTime = ui.mLineEditFromTime->text();
	QString strEndTime = ui.mLineEditEndTime->text();
	if (strStartTime.isEmpty() || strEndTime.isEmpty())
		return false;
	QStringList startTimeLst = strStartTime.split('-');
	int startyear = startTimeLst.at(0).toInt();
	int startmonth = startTimeLst.at(1).toInt();
	int startday = startTimeLst.at(2).toInt();
	QDate startDate(startyear, startmonth, startday);

	QStringList endTimeLst = strEndTime.split('-');
	int endyear = endTimeLst.at(0).toInt();
	int endmonth = endTimeLst.at(1).toInt();
	int endday = endTimeLst.at(2).toInt();
	QDate endDate(endyear, endmonth, endday);

	int datGap = startDate.daysTo(endDate);
	if (datGap > 0)
		return true;
	return false;
}

void DAddDataset::onGeeSummitBtnClicked()
{
	QString eeSnippetID = ui.mLineEditSnippetID->text();
	QString command = QString("_dxl_temp_collection=%1").arg(eeSnippetID);
	bool ok = DPyGeeUtils::runString(command);

	if (!ok) return;
	DPyGeeUtils::setTempVarName(QString("_dxl_temp_collection"));

	bool ok2 = false;
	QString dateRange = DPyGeeUtils::getImgCollectionDateBetween(QString("_dxl_temp_collection"), &ok2);
	if (ok2)
	{
		mSummitGeeFlag = true;
		ui.mLabelDuration->setText(dateRange);
	}
	else
		QMessageBox::critical(this,"Summie ee.ImageCollection", dateRange, QMessageBox::Ok);

	if (!mSummitGeeFlag) return;

	QString res = DPyGeeUtils::getImgCollectionBandsNames();
	if (res.compare("ERROR") == 0)
		return;
	mLastBandsNames = res;
	QStringList resLst = res.split('&');
	for each (QString str in resLst)
	{
		ui.mComboxSelectedBands->addItem(str);
	}
	//DPyGeeUtils::Instance()
}

void DAddDataset::onShowBandDetailsBtnClicked()
{
	if (!mSummitGeeFlag) 
		QMessageBox::critical(this, "Summie ee.ImageCollection", "Fail to summit ee.ImageCollection", QMessageBox::Ok);

	QString description = DPyGeeUtils::getImgCollectionHtmlDescription();
	if (!mShowImgCollectionDetailsWind)
		mShowImgCollectionDetailsWind = new QTextBrowser();
	mShowImgCollectionDetailsWind->setHtml(description);
	mShowImgCollectionDetailsWind->show();
}


void DAddDataset::onSummitFilterBtnClicked()
{
	if (!mSummitGeeFlag) {
		QMessageBox::critical(this, "Summie ee.ImageCollection", "Fail to summit ee.ImageCollection", QMessageBox::Ok);
		return;
	}
	// filterDate
	bool filterDate = compareDateBetween();
	// filterBounds
	bool filterBounds = false;
	if (ui.mComboxBounds->currentIndex() > 0 || !ui.mLineEditCustomEEFileterbounds->text().isEmpty())
		filterBounds = true;
	// filterCloud
	QString strCloudRate = ui.mLineEditCloudRate->text();

	bool filterCloud = strCloudRate.toFloat()!=0;

	bool selectBands = ui.mComboxSelectBands->count() > 0;

	QString command;
	if (filterDate)
	{
		QString strStartTime = ui.mLineEditFromTime->text();
		QString strEndTime = ui.mLineEditEndTime->text();
		command = command.append(".filterDate('%1','%2')").arg(strStartTime, strEndTime);
	}
	if (filterBounds)
	{
		QString boundVarName;
		if (ui.mLineEditCustomEEFileterbounds->text().isEmpty())
			boundVarName = ui.mComboxBounds->currentText();
		else
			boundVarName = ui.mLineEditCustomEEFileterbounds->text();

		command = command.append(".filterBounds(%1)").arg(boundVarName);
	}
	if (filterCloud)
	{
		float cloudRate = strCloudRate.toFloat();

		QString cloudRateFormat = QString::asprintf("%.2f", cloudRate);
		command = command.append(QString(".filterMetadata('CLOUD_COVER', 'not_greater_than', %1)").arg(cloudRateFormat));
	}
	if (selectBands)
	{
		int temCount = ui.mComboxSelectedBands->count();
		QStringList selectedNames;

		for (int i = 0; i < temCount; i++)
			selectedNames << QString("'%1'").arg(ui.mComboxSelectedBands->itemText(i));

		if(!selectedNames.isEmpty())
			command = command.append(".select([%1])").arg(selectedNames.join(','));
		//QString selectedNames = ui.
	}
	if (command.isEmpty()) return;

	command = QString("_dxl_temp_filterCol=_dxl_temp_collection%1").arg(command);
	succeedSummitFilters = DPyGeeUtils::runString(command);
	int countt = -1;
	if (!succeedSummitFilters)
	{
		DPyGeeUtils::runString(QString("_dxl_temp_filterCol=_dxl_temp_collection"));
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_filterCol"), DGeeObjectManager::GeeObjType::ImageCollection, countt);
		return;
	}

	bool recCount = DPyGeeUtils::runString(QString("print(_dxl_temp_filterCol.size().getInfo())"));
	if (recCount)
	{
		int countt = DPyGeeUtils::getLatestRunOut(true).toInt();
		QMessageBox::information(this, "Filter imgs successful", QString("The images after filter has %1 pieces").arg(countt), QMessageBox::Ok);
	}
	else
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::warning(this, "Summit Warning", errMeg, QMessageBox::Ok);
	}

	DPyGeeUtils::runString(QString("_dxl_temp_postOperation=_dxl_temp_filterCol"));
	DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_filterCol"), DGeeObjectManager::GeeObjType::ImageCollection, countt);
	DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_postOperation"), DGeeObjectManager::GeeObjType::ImageCollection, countt);
}


// 进行数据集后处理
void DAddDataset::runPostOperation()
{
	if(!mSummitGeeFlag)
	{
		QMessageBox::critical(this, "Summie ee.ImageCollection", "Fail to summit ee.ImageCollection", QMessageBox::Ok);
		return;
	}

	int postOperationIndex = ui.mComboxPostOperation->currentIndex();
	QString postCommand = QString("_dxl_temp_postOperation=_dxl_temp_filterCol%1");
	switch (postOperationIndex)
	{
	// "NoOperation" << "First" << "Mean" << "Median" << "Mosaic";
	case 0:
		postCommand = postCommand.arg("");
		break;
	case 1:
		postCommand = postCommand.arg(".first()");
		break;
	case 2:
		postCommand = postCommand.arg(".mean()");
		break;
	case 3:
		postCommand = postCommand.arg(".median()");
		break;
	case 4:
		postCommand = postCommand.arg(".mosaic()");
	default:
		postCommand = postCommand.arg("");
		break;
	}

	succeedPostOperation = DPyGeeUtils::runString(postCommand);
	if (!succeedPostOperation)
	{
		DPyGeeUtils::runString(QString("_dxl_temp_postOperation=_dxl_temp_filterCol"));
		DGeeObjectManager::Instance()->addOneRecord(QString("_dxl_temp_postOperation"));
	}
}