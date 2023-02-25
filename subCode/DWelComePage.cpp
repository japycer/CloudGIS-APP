#include "DWelComePage.h"
#include <QSettings>
#include <QDialog>
#include <QMessageBox>
#include <QDateTime>
#include "DPyGeeUtils.h"
#include <QClipboard>

DWelComePage::DWelComePage(DEnums::User* usr, QWidget *parent)
	: QWidget(parent), mCurrentUsr(usr)
{
	ui.setupUi(this);
	initGui();
	bingSignalsSlots();
}

DWelComePage::~DWelComePage()
{
}


void DWelComePage::initGui()
{
	initUsrInfos();
	initUsrAssets();
	initLatestProjects();
	initUploadTask();
	initLatestUpdate();
}

void DWelComePage::bingSignalsSlots()
{
	// itemDoubleClicked(QListWidgetItem *item)
	connect(ui.mListWidgetLatestProjs, &QListWidget::itemDoubleClicked, this, &DWelComePage::onProjectsItemDoubleClicked);
	connect(ui.mBtnContactMe, &QPushButton::clicked, this, &DWelComePage::onBtnContactMeclicked);
	connect(ui.mToolBtnRefreshAssets, &QToolButton::clicked, this, &DWelComePage::onUpdateAssets);
}


void DWelComePage::initUsrInfos()
{
	// 设置用户ICON
	QString iconPath = QString(":/images/profiles/%1.png").arg(mCurrentUsr->strIcon);
	QPixmap pixmap(iconPath);
	ui.mLabelUserProfile->setPixmap(pixmap.scaled(50,50));
	// 设置用户名
	ui.mLineEditUsrName->setText(mCurrentUsr->strName);
	ui.mLineEditUsrName->setEnabled(false);
	// 设置邮箱
	ui.mLineEditGMail->setText(mCurrentUsr->strGmail);
	ui.mLineEditGMail->setEnabled(false);
	if (mCurrentUsr->isTempory)
	{
		ui.mRiadioBtnTemp->setChecked(true);
		ui.mRadioBtnRegister->setCheckable(false);
	}
	else
	{
		ui.mRadioBtnRegister->setChecked(true);
		ui.mRiadioBtnTemp->setCheckable(false);
	}
	QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	QString  latestProject = setting.value("/login/previousLogin").toString();
	ui.mLabelLatestLoginTime->setText(ui.mLabelLatestLoginTime->text().append(latestProject));

	QDateTime timeNow = QDateTime::currentDateTime();
	QString time = timeNow.toString("yyyy-MM-dd hh:mm:ss");
	setting.setValue("/login/previousLogin", time);
}


void DWelComePage::initUsrAssets()
{
	// 初始化item
	ui.mTreeWidgetAssets->setColumnCount(2);
	ui.mTreeWidgetAssets->setColumnWidth(0, 200);
	ui.mTreeWidgetAssets->setColumnWidth(1, 100);
	ui.mTreeWidgetAssets->setHeaderHidden(true);

	//// 1、QTreeWidgetItem *topItemRasters;
	//topItemRasters = new QTreeWidgetItem(ui.mTreeWidgetAssets);
	//topItemRasters->setText(0, "Raster Datas");
	//ui.mTreeWidgetAssets->addTopLevelItem(topItemRasters);
	//// 第二列
	//QWidget* widgetBtn = new QWidget();
	//QHBoxLayout* hLayout = new QHBoxLayout();
	//hLayout->setContentsMargins(2, 2, 5, 2);
	//hLayout->addStretch();
	//QPushButton* btnImport = new QPushButton("Import");
	//QPushButton* btnCopyPath = new QPushButton("Copy");
	//hLayout->addWidget(btnImport);
	//hLayout->addWidget(btnCopyPath);
	//widgetBtn->setLayout(hLayout);
	//ui.mTreeWidgetAssets->setItemWidget(topItemRasters, 1, widgetBtn);


	//// 2、QTreeWidgetItem *topItemTables;
	//topItemTables = new QTreeWidgetItem(ui.mTreeWidgetAssets);
	//topItemTables->setText(0, "Table Datas");
	//ui.mTreeWidgetAssets->addTopLevelItem(topItemTables);
	//// 第二列
	//QWidget* widgetBtn2 = new QWidget();
	//QHBoxLayout* hLayout2 = new QHBoxLayout();
	//hLayout2->setContentsMargins(2, 2, 5, 2);
	//hLayout2->addStretch();
	//QPushButton* btnImport2 = new QPushButton("Import");
	//QPushButton* btnCopyPath2 = new QPushButton("Copy");
	//hLayout2->addWidget(btnImport2);
	//hLayout2->addWidget(btnCopyPath2);
	//widgetBtn2->setLayout(hLayout2);
	//ui.mTreeWidgetAssets->setItemWidget(topItemTables, 1, widgetBtn2);

	//// 3、QTreeWidgetItem *topItemOthers;
	//topItemOthers = new QTreeWidgetItem(ui.mTreeWidgetAssets);
	//topItemOthers->setText(0, "Other Datas");
	//ui.mTreeWidgetAssets->addTopLevelItem(topItemOthers);
	//// 第二列
	//QWidget* widgetBtn3 = new QWidget();
	//QHBoxLayout* hLayout3 = new QHBoxLayout();
	//hLayout3->setContentsMargins(2, 2, 5, 2);
	//hLayout3->addStretch();
	//QPushButton* btnImport3 = new QPushButton("Import");
	//QPushButton* btnCopyPath3 = new QPushButton("Copy");
	//hLayout3->addWidget(btnImport3);
	//hLayout3->addWidget(btnCopyPath3);
	//widgetBtn3->setLayout(hLayout3);
	//ui.mTreeWidgetAssets->setItemWidget(topItemOthers, 1, widgetBtn3);

	// TODO 
	// connect....
}


void DWelComePage::initLatestProjects()
{
	QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	QString  latestProject = setting.value("/files/projects").toString();
	QStringList projects = latestProject.split("&");
	QListWidgetItem* item = nullptr;
	for each (QString pathStr in projects)
	{
		item = new QListWidgetItem(QIcon(":/images/Mine/project.png"), pathStr);
		ui.mListWidgetLatestProjs->addItem(item);
	}
}


void DWelComePage::initUploadTask()
{

}


void DWelComePage::initLatestUpdate()
{
	QListWidgetItem* item = nullptr;

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("Image Visualize"));
	ui.mListWidgetLatestUpdate->addItem(item);

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("ImageCollection Visualize"));
	ui.mListWidgetLatestUpdate->addItem(item);

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("Classify Tool"));
	ui.mListWidgetLatestUpdate->addItem(item);

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("Water Pollution Calculate"));
	ui.mListWidgetLatestUpdate->addItem(item);

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("Tasselled Cap Transformation"));
	ui.mListWidgetLatestUpdate->addItem(item);

	item = new QListWidgetItem(QIcon(":/images/Mine/updates.png"), QStringLiteral("Dark Pixel Atomosphere Calibartion"));
	ui.mListWidgetLatestUpdate->addItem(item);
}


void DWelComePage::onProjectsItemDoubleClicked(QListWidgetItem *item)
{
	QString itemName = item->text();
	if(itemName.endsWith("qgs")|| itemName.endsWith("qgz")||itemName.endsWith("QGS")|| itemName.endsWith("QGZ"))
		emit toLoadProject(itemName);
	else
		QMessageBox::critical(this, "Fail to open project", "The file name is invalid ! Only support *.qgs/qgz/QGS/QGZ",QMessageBox::Ok);
}


void DWelComePage::addOneTaskRecord(QString& taskName)
{
	QListWidgetItem* item = new QListWidgetItem(QIcon(":/images/Mine/project.png"), taskName);
	ui.mListWidgetLatestUpdate->addItem(item);
}


void DWelComePage::onBtnContactMeclicked()
{
	QDialog diag(this);
	QVBoxLayout* vLayout = new QVBoxLayout();
	vLayout->setContentsMargins(0,0,0,0);
	vLayout->setSpacing(5);

	QHBoxLayout* hLayout1 = new QHBoxLayout();
	QLabel* nameLabel = new QLabel("QQ: 2901528129");
	hLayout1->setContentsMargins(5, 10, 5, 0);
	hLayout1->addWidget(nameLabel);
	hLayout1->addStretch();

	QHBoxLayout* hLayout2 = new QHBoxLayout();
	QLabel* emailLabel = new QLabel("EMAIL: 2901528129@qq.com");
	hLayout2->setContentsMargins(5, 10, 5, 0);
	hLayout2->addWidget(emailLabel);

	vLayout->addLayout(hLayout1);
	vLayout->addLayout(hLayout2);
	diag.setLayout(vLayout);
	diag.exec();
}


void DWelComePage::onUpdateAssets()
{
	QString importCommand("from EEPlugin import DUtils");
	bool ok1 = DPyGeeUtils::runString(importCommand);
	if (!ok1)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to update", errMeg, QMessageBox::Ok);
		return;
	}
	QString runCommand("print(DUtils.getGeeAssets())");

	bool ok2 = DPyGeeUtils::runString(runCommand);
	if (!ok2)
	{
		QString errMeg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::critical(this, "Fail to update", errMeg, QMessageBox::Ok);
		return;
	}

	QString resAssets = DPyGeeUtils::getLatestRunOut(true).replace("users/","");
	QStringList resAssetsLst = resAssets.split(":");
	// collection_list, table_list, image_list
	QList<AssetsType> types;
	types << AssetsType::ImageCollection << AssetsType::Table << AssetsType::Image;
	if (resAssetsLst.count() < 3)
	{
		QMessageBox::critical(this, "Fail to Update", "The search result is valid");
		return;
	}

	ui.mTreeWidgetAssets->clear();
	for (int i = 0; i < 3; i++)
	{
		AssetsType currentType = types.at(i);
		QString itemCol = resAssetsLst.at(i);
		if (itemCol.isEmpty())
			continue;

		QStringList itemLst = itemCol.split("&");
		for each (QString item in itemLst)
		{
			addOneAssetByFullName(item, currentType);
		}
	}
}


void DWelComePage::addOneAssetByFullName(QString& fullName, DWelComePage::AssetsType type)
{
	// fullName like: {users/}perphilozdi/Bj_station
	QStringList splitFullNameLst = fullName.split("/");
	if (splitFullNameLst.count() == 1 && splitFullNameLst.at(0).isEmpty())
		return;

	QTreeWidgetItem* item = nullptr;
	int topItemCount = ui.mTreeWidgetAssets->topLevelItemCount();
	if (topItemCount==0)
	{
		item = new QTreeWidgetItem();
		item->setText(0, splitFullNameLst.at(0));
		ui.mTreeWidgetAssets->addTopLevelItem(item);

		splitFullNameLst.takeFirst();
		if (splitFullNameLst.count() > 0)
			toInsertNodeByDiGui(item, splitFullNameLst, type);
		else
		{
			QWidget* widgetBtn = new QWidget();
			QHBoxLayout* hLayout = new QHBoxLayout();
			hLayout->setContentsMargins(2, 2, 5, 2);
			hLayout->addStretch();
			QPushButton* btnImport = new QPushButton("Import");
			QPushButton* btnCopyPath = new QPushButton("Copy");
			hLayout->addWidget(btnImport);
			hLayout->addWidget(btnCopyPath);
			widgetBtn->setLayout(hLayout);
			ui.mTreeWidgetAssets->setItemWidget(item, 1, widgetBtn);

			connect(btnImport, &QPushButton::clicked, this, &DWelComePage::onBtnImportAssetsClicked);
			connect(btnCopyPath, &QPushButton::clicked, this, &DWelComePage::onBtnCopyAssetsClicked);
			return;
		}
	}
	else
	{
		bool findIt = false;
		for (int i = 0; i < topItemCount; i++)
		{
			item = ui.mTreeWidgetAssets->topLevelItem(i);
			if (item->text(0).compare(splitFullNameLst.at(0)) == 0)
			{
				findIt = true;
				splitFullNameLst.takeFirst();
				if (splitFullNameLst.count() > 0)
				{
					toInsertNodeByDiGui(item, splitFullNameLst, type);
					return;
				}
				else
					return;
			}
		}
		if (!findIt)
		{
			item = new QTreeWidgetItem();
			item->setText(0, splitFullNameLst.at(0));
			ui.mTreeWidgetAssets->addTopLevelItem(item);
			splitFullNameLst.takeFirst();

			if (splitFullNameLst.count() > 0)
			{
				toInsertNodeByDiGui(item, splitFullNameLst, type);
				return;
			}
			else
			{
				QWidget* widgetBtn = new QWidget();
				QHBoxLayout* hLayout = new QHBoxLayout();
				hLayout->setContentsMargins(2, 2, 5, 2);
				hLayout->addStretch();
				QPushButton* btnImport = new QPushButton("Import");
				QPushButton* btnCopyPath = new QPushButton("Copy");
				hLayout->addWidget(btnImport);
				hLayout->addWidget(btnCopyPath);
				widgetBtn->setLayout(hLayout);
				ui.mTreeWidgetAssets->setItemWidget(item, 1, widgetBtn);
				connect(btnImport, &QPushButton::clicked, this, &DWelComePage::onBtnImportAssetsClicked);
				connect(btnCopyPath, &QPushButton::clicked, this, &DWelComePage::onBtnCopyAssetsClicked);
				return;
			}
		}
	}
}


void DWelComePage::toInsertNodeByDiGui(QTreeWidgetItem* item, QStringList& splitNames, AssetsType type)
{
	int childCount = item->childCount();
	if (childCount == 0)
	{
		QTreeWidgetItem* insertItem = new QTreeWidgetItem();
		insertItem->setText(0, splitNames.at(0));
		item->addChild(insertItem);

		splitNames.takeFirst();
		if (splitNames.count() > 0)
			toInsertNodeByDiGui(insertItem, splitNames, type);
		else
		{
			QWidget* widgetBtn = new QWidget();
			QHBoxLayout* hLayout = new QHBoxLayout();
			hLayout->setContentsMargins(2, 2, 5, 2);
			hLayout->addStretch();
			QPushButton* btnImport = new QPushButton("Import");
			QPushButton* btnCopyPath = new QPushButton("Copy");
			hLayout->addWidget(btnImport);
			hLayout->addWidget(btnCopyPath);
			widgetBtn->setLayout(hLayout);
			ui.mTreeWidgetAssets->setItemWidget(insertItem, 1, widgetBtn);

			switch (type)
			{
			case DWelComePage::Image:
				insertItem->setIcon(0, QIcon(":/images/Mine/imageAsset.png"));
				insertItem->setToolTip(0, "Image");
				break;
			case DWelComePage::Table:
				insertItem->setIcon(0, QIcon(":/images/Mine/tableAsset.png"));
				insertItem->setToolTip(0, "Table");
				break;
			case DWelComePage::ImageCollection:
				insertItem->setIcon(0, QIcon(":/images/Mine/imagesAsset.png"));
				insertItem->setToolTip(0, "ImageCollection");
				break;
			case DWelComePage::Undifine:
			default:
				break;
			}

			connect(btnImport, &QPushButton::clicked, this, &DWelComePage::onBtnImportAssetsClicked);
			connect(btnCopyPath, &QPushButton::clicked, this, &DWelComePage::onBtnCopyAssetsClicked);
			return;
		}
	}
	else
	{
		bool findIt = false;
		for (int i = 0; i < childCount; i++)
		{
			QTreeWidgetItem* currentItem = item->child(i);
			if (currentItem->text(0).compare(splitNames.at(0)) == 0)
			{
				findIt = true;
				splitNames.takeFirst();
				if (splitNames.count() > 0)
				{
					toInsertNodeByDiGui(currentItem, splitNames, type);
					return;
				}
				else
					return;
			}
		}
		if (!findIt)
		{
			QTreeWidgetItem* insertItem = new QTreeWidgetItem();
			insertItem->setText(0, splitNames.at(0));
			item->addChild(insertItem);

			splitNames.takeFirst();
			if (splitNames.count() > 0)
			{
				toInsertNodeByDiGui(insertItem, splitNames, type);
				return;
			}
			else
			{
				QWidget* widgetBtn = new QWidget();
				QHBoxLayout* hLayout = new QHBoxLayout();
				hLayout->setContentsMargins(2, 2, 5, 2);
				hLayout->addStretch();
				QPushButton* btnImport = new QPushButton("Import");
				QPushButton* btnCopyPath = new QPushButton("Copy");
				hLayout->addWidget(btnImport);
				hLayout->addWidget(btnCopyPath);
				widgetBtn->setLayout(hLayout);
				ui.mTreeWidgetAssets->setItemWidget(insertItem, 1, widgetBtn);

				switch (type)
				{
				case DWelComePage::Image:
					insertItem->setIcon(0, QIcon(":/images/Mine/imageAsset.png"));
					insertItem->setToolTip(0, "Image");
					break;
				case DWelComePage::Table:
					insertItem->setIcon(0, QIcon(":/images/Mine/tableAsset.png"));
					insertItem->setToolTip(0, "Table");
					break;
				case DWelComePage::ImageCollection:
					insertItem->setIcon(0, QIcon(":/images/Mine/imagesAsset.png"));
					insertItem->setToolTip(0, "ImageCollection");
					break;
				case DWelComePage::Undifine:
				default:
					break;
				}
				
				connect(btnImport, &QPushButton::clicked, this, &DWelComePage::onBtnImportAssetsClicked);
				connect(btnCopyPath, &QPushButton::clicked, this, &DWelComePage::onBtnCopyAssetsClicked);
				return;
				//insertItem->setIcon();
			}
		}
	}
}


void DWelComePage::onBtnImportAssetsClicked()
{
	QPushButton*btn = qobject_cast<QPushButton*>(sender());
	if (!btn) return;
	// btn => Layout => Qwidget => QTreeWidgetItem
	//QObject* bababa = btn->parent()->parent()->parent();
	//QTreeWidgetItem*item = qobject_cast<QTreeWidgetItem*>(bababa);
	QPoint clickPoint = QPoint(btn->parentWidget()->frameGeometry().x(), btn->parentWidget()->frameGeometry().y());
	QTreeWidgetItem*item = ui.mTreeWidgetAssets->itemAt(clickPoint);
	QString nodeName;
	QStringList assetPathLst;
	if (item)
	{
		nodeName = item->text(0);
		assetPathLst.append(nodeName);
		QTreeWidgetItem* parentNode = item->parent();
		while (parentNode)
		{
			assetPathLst.insert(0, parentNode->text(0));
			parentNode = parentNode->parent();
		}
	}
	else
		return;

	QString toolTip = item->toolTip(0);
	QString res = QString("users/").append(assetPathLst.join("/"));
	QString eeType("FeatureCollection");

	if (toolTip.compare("Image") == 0)
		eeType = QString("Image");
	else if (toolTip.compare("Table") == 0)
		eeType = QString("FeatureCollection");
	else if (toolTip.compare("ImageCollection") == 0)
		eeType = QString("ImageCollection");

	QString command = QString("%1=ee.%2(\"%3\")").arg(nodeName, eeType,res);
	bool ok = DPyGeeUtils::runString(command);
	if (ok)
	{
		QMessageBox::information(this, "Succeed to load asset", QString("The Viriable name is %1").arg(nodeName),QMessageBox::Ok);
		return;
	}
	else
	{
		QString errMsg = DPyGeeUtils::getLatestRunOut(false);
		QMessageBox::information(this, "Fail to load asset", errMsg, QMessageBox::Ok);
		return;
	}
}


void DWelComePage::onBtnCopyAssetsClicked()
{
	QPushButton*btn = qobject_cast<QPushButton*>(sender());
	if (!btn) return;
	QPoint clickPoint = QPoint(btn->parentWidget()->frameGeometry().x(), btn->parentWidget()->frameGeometry().y());
	QTreeWidgetItem*item = ui.mTreeWidgetAssets->itemAt(clickPoint);
	QString nodeName;
	QStringList assetPathLst;
	if (item)
	{
		nodeName = item->text(0);
		assetPathLst.append(nodeName);
		QTreeWidgetItem* parentNode = item->parent();
		while (parentNode)
		{
			assetPathLst.insert(0, parentNode->text(0));
			parentNode = parentNode->parent();
		}
	}
	else
		return;

	QString toolTip = item->toolTip(0);
	QString eeType("FeatureCollection");

	if (toolTip.compare("Image") == 0)
		eeType = QString("Image");
	else if (toolTip.compare("Table") == 0)
		eeType = QString("FeatureCollection");
	else if (toolTip.compare("ImageCollection") == 0)
		eeType = QString("ImageCollection");

	QString res = QString("users/").append(assetPathLst.join("/"));
	QString content = QString("ee.%1(\"%2\")").arg(eeType, res);
	
	QApplication::clipboard()->setText(content);
}