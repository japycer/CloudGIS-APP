#include "DDatabaseInterface.h"
#include"qstandarditemmodel.h"
#include"qsqlquerymodel.h"
#include <QTableWidgetItem>
#include"qheaderview.h"
#include "DDatabaseUtils.h"
#include"qsortfilterproxymodel.h"
#include"qtableview.h"
#include"qabstractscrollarea.h"
#include"qscrollbar.h"
#include"qlist.h"
#include "qdebug.h"
#include"qmessagebox.h"
#include"QtWebKitWidgets/qwebview.h"
#include"DGeeInfoBase.h"
#include "qsettings.h"
// 调用cmd，处理返回的json字符串
#include <iostream>


using namespace std;
// 描述:execmd函数执行命令，并将结果存储到result字符串数组中
// 参数:cmd表示要执行的命令
// result是执行的结果存储的字符串数组
// 函数执行成功返回1，失败返回0
int execmd(char* cmd, char* result) {
	char buffer[500]; //定义缓冲区
	FILE* pipe = _popen(cmd, "r"); //打开管道，并执行命令
	if (!pipe)
		return 0; //返回0表示运行失败
	while (!feof(pipe)) {
		if (fgets(buffer, 500, pipe)) { //将管道输出到result中
			strcat(result, buffer);
		}
	}
	_pclose(pipe); //关闭管道
	return 1; //返回1表示运行成功
}


DDatabaseInterface::DDatabaseInterface(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	init();
	initTableView();
	initModelAll();
	QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	QString keyWords = setting.value("/database/GEE/KEYWORDS").toString();
	allKeyWords = keyWords.split("&");

	for each (QString str in allKeyWords)
	{
		if(str.simplified().length()>2)
			ui.mComboKeyw->addItem(str);
	}

	connect(ui.mBtnSearch, &QPushButton::clicked, this, &DDatabaseInterface::onBtnSearchClicked);
	connect(ui.mBtnSearchOnline, &QPushButton::clicked, [&] {
		onBtnsearchDataOnlineClicked();
	});

	// 当用户点击更新搜索内容至数据库，则更新数据库+写入配置文件+刷新combox
	connect(ui.mBtnUpdateToDatabase, &QPushButton::clicked, [&] {
		QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
		QString keyWords = setting.value("/database/GEE/KEYWORDS").toString();
		keyWords = keyWords + "&" + currentKeyword;
		setting.setValue("/database/GEE/KEYWORDS", keyWords);
		updateResultToDatabase(); });


	connect(ui.mComboKeyw, &QComboBox::currentTextChanged, [&](QString keyw) {onKeywordComboChangedText(keyw); });
	connect(ui.mBtnGo2Select, &QPushButton::clicked, this, &DDatabaseInterface::onBtnShowCurrentItemClicked);
	
	// 待测试
	connect(ui.mBtnUpdateAll, &QPushButton::clicked, [&] {updateDatabase(); });
}

//DDatabaseInterface::~DDatabaseInterface()
//{
//}

void DDatabaseInterface::init()
{
	mdbRunner = DDatabaseUtils::Instance();

	mdbRunner->connectDatabase(QString("geeAssets.db"));
	if (!mdbRunner->openDatabase())
		return;
	QString command = QString("create table if not exists assets(ee_id_snippet text primary key, title text, start_date text, end_date text, asset_url text, thumbnail_url text, keyword text,description text)");
	mdbRunner->createTable(command);

	insertCmdComplate = QString("INSERT INTO assets VALUES(\"%1\", \"%2\", \"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"\")");
	mdbRunner->closeDatabase();

	mModelSearchOnline = new QStandardItemModel(this);
	// set table's header
	QString header("ee_id_snippet$title$start_date$end_date$asset_url$thumbnail_url$keyword$description");
	QStringList headerList = header.split("$");
	mModelSearchOnline->setHorizontalHeaderLabels(headerList);
	mShowInfoTemplate = QString(
		"<html><body><h3>asset_title</h3><h4>Dataset Availability</h4><p style = 'margin-left: 40px'>asset_dates</p><h4>Earth Engine Snippet</h4><p style = 'margin-left: 40px'>ee_id_snippet</p><h4>Earth Engine Data Catalog</h4><p style = 'margin-left: 40px'><a href = 'asset_url' target = '_blank'>asset_id</a></p></body></html>");

	// 初始化信息可视乎webview
	mMainWebview = new QWebView();
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);

	mInfoPanel = new DGeeInfoBase();
	layout->addWidget(mInfoPanel);
	layout->addWidget(mMainWebview);
	ui.mWidgetPreview->setLayout(layout);
}


// 在线检索按钮点击
void  DDatabaseInterface::onBtnsearchDataOnlineClicked()
{
	QString target = ui.mLineEditKeywords->text().simplified();
	if (target.isEmpty() || target.length() < 2)
		return;
	for each (QString str in allKeyWords)
	{
		int res = QString::compare(target, str, Qt::CaseInsensitive);
		if (res == 0)
		{
			int ok = QMessageBox::warning(this, "keyword has existed", "To show the result ?", QMessageBox::Yes | QMessageBox::No);
			if (ok == QMessageBox::No) return;
			if (ok)
			{
				QString sqlCmd = QString("Select * from assets where kwyword = %1").arg(target);
				if (!mdbRunner->openDatabase())
					return;

				mModelAll->setQuery(sqlCmd, mdbRunner->getDatabase());
				mdbRunner->closeDatabase();
				return;
			}
			else return;
		}
	}

	startToSearchOnline(target);
	if (mItemList.count() > 0) {
		currentKeyword = target;
	}
	else return;
	hasUpdateToDb = false;

	int option = QMessageBox::warning(this, "Loading", "To load searchinig data ...", QMessageBox::Yes | QMessageBox::No);
	if (option == QMessageBox::Yes)
		showOnlineSearchRes();
}


// 开始去在线搜索
void DDatabaseInterface::startToSearchOnline(QString &target)
{
	char* resArray = new char[1024 * 40];	// 开创一个50k的空间，大约能100个在线搜索结果

	try
	{
		QString cmd = QString("conda activate gee && geeadd search --keywords \"%1\"").arg(target);
		QByteArray byteCmd = cmd.toLatin1();
		if (1 == execmd(byteCmd.data(), resArray)) {
			QString resALl(resArray);
			int start_index = resALl.indexOf("[");
			QString resMainContent = resALl.mid(start_index);

			QJsonParseError error;
			QJsonDocument document = QJsonDocument::fromJson(resMainContent.toLatin1(), &error);

			if (error.error != QJsonParseError::NoError)
			{
				QMessageBox::warning(this, "Parse json error", "Click yes to qiut !", QMessageBox::Yes);
				return;
			}

			if (document.isNull() || document.isEmpty())
			{
				QMessageBox::warning(this, "Json null or empty", "Click yes to qiut !", QMessageBox::Yes);
				return;
			}

			mItemList = document.toVariant().toList();
			delete[] resArray;
		}
		else {
			QMessageBox::warning(this, "Searching ERROR", "Click yes to qiut !", QMessageBox::Yes);
			return;
		}
	}
	catch (const std::exception&)
	{
		return;
	}
}


// 展示在线收索结果
void DDatabaseInterface::showOnlineSearchRes()
{
	//number of search result's row, the first row is title
	int rowCnt = mItemList.count();
	if (rowCnt < 1)
	{
		QMessageBox::warning(this, "Searching result is empty", "Click yes to qiut !", QMessageBox::Yes);
		return;
	}

	//the actually mounts
	mModelSearchOnline->setRowCount(rowCnt - 1);

	// input  mainly data
	QStandardItem *item = nullptr;
	try
	{
		for (int i = 1; i < rowCnt; i++)
		{
			// 获取数据区的一行
			QVariantMap map = mItemList[i].toMap();
			// 获取每一个属性对应的值
			// ee_id_snippet,title,start_date,end_date, asset_url,thumbnail_utl,keyword,description
			item = new QStandardItem(map["ee_id_snippet"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 0, item);
			item = new QStandardItem(map["title"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 1, item);
			item = new QStandardItem(map["start_date"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 2, item);
			item = new QStandardItem(map["end_date"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 3, item);
			item = new QStandardItem(map["asset_url"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 4, item);
			item = new QStandardItem(map["thumbnail_url"].toString().simplified());
			mModelSearchOnline->setItem(i - 1, 5, item);
			item = new QStandardItem(ui.mLineEditKeywords->text().simplified());
			mModelSearchOnline->setItem(i - 1, 6, item);
			mModelSearchOnline->setItem(i - 1, 7, new QStandardItem(""));
		}
	}
	catch (const std::exception& e)
	{
		qDebug() << QString(e.what());
	}
	ui.mTableView->setModel(mModelSearchOnline);
}

// combo切换事件
void DDatabaseInterface::onKeywordComboChangedText(QString &currentText)
{
	if (!mdbRunner->openDatabase())
		return;

	QString sqlCmd = QString("Select * from assets where keyword = \"%1\"").arg(currentText);
	mModelAll->setQuery(sqlCmd, mdbRunner->getDatabase());
	if (ui.mTableView->model() != mModelAll)
		ui.mTableView->setModel(mModelAll);
	mdbRunner->closeDatabase();
}

void DDatabaseInterface::initTableView()
{
	ui.splitter->setStretchFactor(0, 8);
	ui.splitter->setStretchFactor(0, 2);
	ui.mWidgetSearch->setFixedHeight(40);
	ui.mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);		// 行选中
	ui.mTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);		// 不能编辑
	ui.mTableView->setSelectionMode(QAbstractItemView::SingleSelection);	//只能选中一行
	//ui.mTableView->setShowGrid(true);		//是否展示边框线
	//ui.mTableView->setGridStyle(Qt::DotLine);			// 边框线的格式
	//ui.mTableView->setSortingEnabled(true);	//是否启用排序，本程序中不效果，在于gee数据无数值型
	ui.mTableView->setAlternatingRowColors(true);	// 交替行颜色
	ui.mTableView->resizeColumnsToContents();	// 行宽根据内容调整
	//ui.mTableView->installEventFilter(this);	// 。。。

	// 设置垂直、水平scroll bar的格式
	ui.mTableView->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{"        //垂直滑块整体
		"background:#FFFFFF;"  //背景色
		"padding-top:20px;"    //上预留位置（放置向上箭头）
		"padding-bottom:20px;" //下预留位置（放置向下箭头）
		"padding-left:3px;"    //左预留位置（美观）
		"padding-right:3px;"   //右预留位置（美观）
		"border-left:1px solid #d7d7d7;}"//左分割线
		"QScrollBar::handle:vertical{"//滑块样式
		"background:#dbdbdb;"  //滑块颜色
		"border-radius:6px;"   //边角圆润
		"min-height:80px;}"    //滑块最小高度
		"QScrollBar::handle:vertical:hover{"//鼠标触及滑块样式
		"background:#d0d0d0;}" //滑块颜色
		"QScrollBar::add-line:vertical{"//向下箭头样式
		"background:url(:/images/resource/images/checkout/down.png) center no-repeat;}"
		"QScrollBar::sub-line:vertical{"//向上箭头样式
		"background:url(:/images/resource/images/checkout/up.png) center no-repeat;}");

	ui.mTableView->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal{"
		"background:#FFFFFF;"
		"padding-top:3px;"
		"padding-bottom:3px;"
		"padding-left:20px;"
		"padding-right:20px;}"
		"QScrollBar::handle:horizontal{"
		"background:#dbdbdb;"
		"border-radius:6px;"
		"min-width:80px;}"
		"QScrollBar::handle:horizontal:hover{"
		"background:#d0d0d0;}"
		"QScrollBar::add-line:horizontal{"
		"background:url(:/images/resource/images/checkout/right.png) center no-repeat;}"
		"QScrollBar::sub-line:horizontal{"
		"background:url(:/images/resource/images/checkout/left.png) center no-repeat;}");
}

void DDatabaseInterface::onBtnSearchClicked(bool checked)
{
	QString sqlCmd;
	if (ui.mLIneEditSql->text().isEmpty())
		sqlCmd = QString("Select * from assets");
	else
		sqlCmd = ui.mLIneEditSql->text().simplified();
	if (!mdbRunner->openDatabase())
		return;

	mModelAll->setQuery(sqlCmd, mdbRunner->getDatabase());
	if (ui.mTableView->model() != mModelAll)
		ui.mTableView->setModel(mModelAll);
	mdbRunner->closeDatabase();
}

void DDatabaseInterface::initModelAll()
{
	mModelAll = new QSqlQueryModel(ui.mTableView);
	mProxyModel = new QSortFilterProxyModel(this);
	mProxyModel->setSourceModel(mModelAll);
	if (!mdbRunner->openDatabase())
		return;

	mModelAll->setQuery("SELECT * FROM assets", mdbRunner->getDatabase());
	//QStringList headerLabels = QObject::trUtf8("ee_id_snippet,title,start_date,end_date, asset_url,thumbnail_utl,keyword,description").simplified().split(",");
	mModelAll->setHeaderData(0, Qt::Horizontal, tr("ee_id_snippet"));
	mModelAll->setHeaderData(1, Qt::Horizontal, tr("title"));
	mModelAll->setHeaderData(2, Qt::Horizontal, tr("start_date"));
	mModelAll->setHeaderData(3, Qt::Horizontal, tr("end_date"));
	mModelAll->setHeaderData(4, Qt::Horizontal, tr("asset_url"));
	mModelAll->setHeaderData(5, Qt::Horizontal, tr("thumbnail_utl"));
	mModelAll->setHeaderData(6, Qt::Horizontal, tr("keyword"));
	mModelAll->setHeaderData(7, Qt::Horizontal, tr("description"));

	ui.mTableView->setModel(mModelAll);
	ui.mTableView->setColumnWidth(0, 400);
	ui.mTableView->setColumnWidth(1, 450);
	ui.mTableView->setColumnWidth(2, 150);
	ui.mTableView->setColumnWidth(3, 150);
	ui.mTableView->setColumnWidth(4, 500);
	ui.mTableView->setColumnWidth(5, 500);
	ui.mTableView->setColumnWidth(6, 100);
	ui.mTableView->setColumnWidth(7, 100);
	ui.mTableView->show();
	mdbRunner->closeDatabase();
}


void DDatabaseInterface::exit()
{
	mdbRunner->closeDatabase();
}

void DDatabaseInterface::insert(QVariantMap& res)
{
	if (!mdbRunner->openDatabase())
		return;
	QString cmd = insertCmdComplate.arg(res["ee_id_snippet"].toString().simplified())
		.arg(res["title"].toString().simplified())
		.arg(res["start_date"].toString().simplified())
		.arg(res["end_date"].toString().simplified())
		.arg(res["asset_url"].toString().simplified())
		.arg(res["thumbnail_url"].toString().simplified())
		.arg(currentKeyword);
	mdbRunner->insertData(cmd);
	mdbRunner->closeDatabase();
}


void DDatabaseInterface::updateDatabase()
{
	int ok = QMessageBox::warning(this, "Tips", "This process maybe take much time. \n Assure to do it ?", QMessageBox::Yes | QMessageBox::No);
	if (ok == QMessageBox::No) return;
	try
	{
		for each (QString keyw in allKeyWords)
		{
			hasUpdateToDb = false;
			startToSearchOnline(keyw);
			currentKeyword = keyw;
			updateResultToDatabase(false);
		}
	}
	catch (const std::exception& e)
	{
		qDebug() << QString(e.what());
	}
}


void DDatabaseInterface::updateResultToDatabase(bool isSingle)
{
	if (isSingle)
	{
		if (hasUpdateToDb)
		{
			QMessageBox::critical(this, "repeated operation", "Current data has updated to database !");
			return;
		}
		if (mItemList.count() < 1)
			return;
	}

	if (!mdbRunner->openDatabase())
		return;
	int nums = mItemList.count();
	try
	{
		for (size_t i = 0; i < nums; i++)
		{
			QVariantMap map = mItemList[i].toMap();
			if (map["title"].toString().simplified().length() < 1)
				break;

			insert(map);
		}
		hasUpdateToDb = true;
	}
	catch (const std::exception& e)
	{
		qDebug() << QString(e.what());
	}
	if (isSingle)
		ui.mComboKeyw->addItem(currentKeyword);
	mdbRunner->closeDatabase();
}


// 展示信息按钮点击
void DDatabaseInterface::onBtnShowCurrentItemClicked()
{
	int row = ui.mTableView->currentIndex().row();	//选中的行
	if (row < 1) return;
	QAbstractItemModel *model = ui.mTableView->model();
	//ee_id_snippet, title, start_date , end_date, asset_url, thumbnail_url, keyword ,description
	QString ee_id_snippet = model->data(model->index(row, 0)).toString();
	QString title = model->data(model->index(row, 1)).toString();
	QString start_date = model->data(model->index(row, 2)).toString();
	QString end_date = model->data(model->index(row, 3)).toString();
	QString asset_url = model->data(model->index(row, 4)).toString();
	QString thumbnail_url = model->data(model->index(row, 5)).toString();
	QString dateAvailibility = start_date + "  " + end_date;

	mInfoPanel->setLabelContent(title, dateAvailibility, ee_id_snippet, asset_url);
	mMainWebview->load(QUrl(thumbnail_url)); //< h4 > Dataset Thumbnail< / h4><img src = 'thumbnail_url'>
}
