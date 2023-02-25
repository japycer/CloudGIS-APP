#include "DTemporyDrawerManager.h"
#include"qcolordialog.h"

DTemporyDrawerManager::DTemporyDrawerManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	connect(ui.mBtnSelectColor, &QPushButton::clicked, this, &DTemporyDrawerManager::onSelectColorBtnClicked);
	connect(ui.mBtnOk, &QPushButton::clicked, this, &DTemporyDrawerManager::onOkClicked);
	connect(ui.mMainTreeWidget, &QTreeWidget::itemDoubleClicked, this, &DTemporyDrawerManager::onItemDoubleClicked);
	connect(ui.mBtnDeleteSelection, &QPushButton::clicked, this, &DTemporyDrawerManager::onBtnDeleteClicked);
}


DTemporyDrawerManager::~DTemporyDrawerManager()
{
	delete topItem1;
	delete topItem2;
	delete topItem3;
	delete topItem4;
}


void DTemporyDrawerManager::initGui()
{
	// 初始化combox
	ui.mComboxDrawType->addItem("Point");
	ui.mComboxDrawType->addItem("LineString");
	ui.mComboxDrawType->addItem("MultiPolygon");
	ui.mComboxDrawType->addItem("Rectangle");

	// 初始化treeWidget
	topItem1 = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItem1->setText(0, "Point");
	topItem1->setCheckState(0, Qt::Checked);
	ui.mMainTreeWidget->addTopLevelItem(topItem1);

	topItem2 = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItem2->setText(0, "LineString");
	topItem2->setCheckState(0, Qt::Checked);
	ui.mMainTreeWidget->addTopLevelItem(topItem2);

	topItem3 = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItem3->setText(0, "MultiPolygon");
	topItem3->setCheckState(0, Qt::Checked);
	ui.mMainTreeWidget->addTopLevelItem(topItem3);

	topItem4 = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItem4->setText(0, "Rectangle");
	topItem4->setCheckState(0, Qt::Checked);
	ui.mMainTreeWidget->addTopLevelItem(topItem4);

	ui.mBtnSelectColor->setStyleSheet("background-color: red");
	ui.mMainTreeWidget->setHeaderHidden(true);	// 首行隐藏
	ui.mMainTreeWidget->setExpandsOnDoubleClick(false);	// 双击不展开
	ui.mMainTreeWidget->expandAll();
}


void DTemporyDrawerManager::onOkClicked()
{
	currentDrawType = judgetCurrentDrawType();
	emit drawStyleChanged(currentDrawType, currentSelectionColor);
}


DTemporyDrawerManager::DrawType DTemporyDrawerManager::judgetCurrentDrawType()
{
	DTemporyDrawerManager::DrawType drawType;
	QString strOption = ui.mComboxDrawType->currentText();
	int compareRes = -1;

	compareRes = QString::compare(strOption, "Point", Qt::CaseInsensitive);
	if (compareRes == 0)
		return DTemporyDrawerManager::DrawType::Point;
	compareRes = QString::compare(strOption, "LineString", Qt::CaseInsensitive);
	if (compareRes == 0)
		return DTemporyDrawerManager::DrawType::LineString;
	compareRes = QString::compare(strOption, "MultiPolygon", Qt::CaseInsensitive);
	if (compareRes == 0)
		return DTemporyDrawerManager::DrawType::MultiPolygon;
	compareRes = QString::compare(strOption, "Rectangle", Qt::CaseInsensitive);
	if (compareRes == 0)
		return DTemporyDrawerManager::DrawType::Rectangle;

	return DTemporyDrawerManager::DrawType::UnKnown;
}


void DTemporyDrawerManager::onSelectColorBtnClicked()
{
	currentSelectionColor = QColorDialog::getColor(Qt::black, this, "select draw color");
	QString strColor = currentSelectionColor.name();
	QString styleColor = "background-color:" + strColor;
	if (strColor.compare("#000000") == 0)
		return;
	ui.mBtnSelectColor->setStyleSheet(styleColor);
}


void DTemporyDrawerManager::insertAItemToTree(QString &name)
{
	QTreeWidgetItem * item = new QTreeWidgetItem();
	item->setText(0, name);
	item->setCheckState(0, Qt::Checked);
	switch (currentDrawType)
	{
	case DTemporyDrawerManager::Point:
		topItem1->addChild(item);
		break;
	case DTemporyDrawerManager::LineString:
		topItem2->addChild(item);
		break;
	case DTemporyDrawerManager::MultiPolygon:
		topItem3->addChild(item);
		break;
	case DTemporyDrawerManager::Rectangle:
		topItem4->addChild(item);
		break;
	case DTemporyDrawerManager::UnKnown:
	default:
		break;
	}
}


// 当父节点隐藏/显示，子节点隐藏/显示
void DTemporyDrawerManager::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	QList<QString> toHidenNames;
	bool isChecked = false;

	Qt::CheckState currentItemState = item->checkState(0);
	Qt::CheckState toState;
	if (currentItemState == Qt::CheckState::Checked)
	{
		toState = Qt::CheckState::Unchecked;
		isChecked = false;
	}

	if (currentItemState == Qt::CheckState::Unchecked)
	{
		toState = Qt::CheckState::Checked;
		isChecked = true;
	}
	item->setCheckState(0, toState);
	for (int i = 0; i < item->childCount(); i++)
	{
		QTreeWidgetItem *pChildItem = item->child(i);
		pChildItem->setCheckState(0, toState);
		QString name = pChildItem->text(0);
		toHidenNames.append(name);
	}

	if(toHidenNames.count() > 0)
		emit itemsVisibleChanged(toHidenNames, isChecked);
}


void DTemporyDrawerManager::onBtnDeleteClicked()
{
	QList<QString> toDeleteNames;

	QTreeWidgetItem *item = ui.mMainTreeWidget->currentItem();
	// 判断是否是根节点
	if (item->parent() == nullptr)
	{
		for (int i = 0; i < item->childCount(); i++)
		{
			QTreeWidgetItem *pChildItem = item->child(i);
			QString name = pChildItem->text(0);
			toDeleteNames.append(name);
		}
		// 一次性删除子节点
		qDeleteAll(item->takeChildren());
	}
	else
	{
		toDeleteNames.append(item->text(0));
		delete item->parent()->takeChild(ui.mMainTreeWidget->currentIndex().row());
	}
		
	if (toDeleteNames.count() > 0)
		emit toDeleteAGeometry(toDeleteNames);
}