#include "DClassifyManager.h"
#include"qcolordialog.h"
#include"qmessagebox.h"
#include"qlabel.h"

DClassifyManager::DClassifyManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	init();
	mClassNames = new QList<QString>();

	connect(ui.mBtnClassColor, &QPushButton::clicked, this, &DClassifyManager::onBtnSelectColorClicked);
	connect(ui.mBtnNewAClass, &QPushButton::clicked, this, &DClassifyManager::onBtnNewAClassClicked);
	connect(ui.mMainTreeWidget, &QTreeWidget::itemDoubleClicked, this, &DClassifyManager::onItemDoubleClicked);
	connect(ui.mComboxClassNames, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {onCurrentTextChanged(index); });
	connect(ui.mBtnDeleteAItem, &QPushButton::clicked, this, &DClassifyManager::onDeleteBtnClicked);	// delete

	connect(ui.mBtnStartExport, &QPushButton::clicked, [=] {
		QString exportName = ui.mLineEditExportName->text().simplified();
		QString exportTo = ui.mComboxExportTo->currentText();
		emit toExportSamples(exportTo, exportName); }); // export 
}


DClassifyManager::~DClassifyManager()
{
}

void DClassifyManager::init()
{
	QList<QString>* mClassNames = new QList<QString>();

	// init GUI
	ui.mMainTreeWidget->setColumnCount(3);
	ui.mMainTreeWidget->setHeaderLabels(QStringList() << "Class Name" << "Color" << "Count");

	// 初始化treeWidget
	ui.mMainTreeWidget->setExpandsOnDoubleClick(false);	// 双击不展开
	ui.mComboxExportTo->addItems(QStringList() << "PyObj" << "Shp" << "VLayer");
	ui.mComboxExportTo->setCurrentIndex(0);

	// 设置导出文件名/变量名的输入规范
	QRegExp regVar("[a-zA-Z_]+\\w*");
	ui.mLineEditExportName->setValidator(new QRegExpValidator(regVar, this));
	//ui.mComboxExportTo->
}


void DClassifyManager::addAClassToTree(QString &clsName, QColor &c)
{
	QTreeWidgetItem *topItem = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItem->setText(0, clsName);
	topItem->setCheckState(0, Qt::Checked);
	ui.mMainTreeWidget->addTopLevelItem(topItem);
	// 第二列
	QPushButton *btnColor = new QPushButton();
	btnColor->setStatusTip(clsName);
	QString strColor = c.name();
	QString styleColor = "background-color:" + strColor;
	btnColor->setStyleSheet(styleColor);
	ui.mMainTreeWidget->setColumnWidth(1, 60);
	ui.mMainTreeWidget->setItemWidget(topItem, 1, btnColor);
	connect(btnColor, &QPushButton::clicked, this, &DClassifyManager::onChangeClassColorBtnClicked);
	// 第三列
	QLabel *labelCount = new QLabel("0");
	ui.mMainTreeWidget->setItemWidget(topItem, 2, labelCount);
	mClassItems.append(topItem);
}


void DClassifyManager::updateClassCount(QString &clsName, int count)
{
	for each (QTreeWidgetItem* item in mClassItems)
	{
		if (item->text(0).compare(clsName) == 0)
		{
			QWidget *w = ui.mMainTreeWidget->itemWidget(item, 2);
			QLabel *label = qobject_cast<QLabel *>(w);
			if (!label)
				label->setText(QString("  %1").arg(count));
		}
	}
}

void DClassifyManager::onCurrentTextChanged(int index)
{
	QString clsName = ui.mComboxClassNames->currentText();
	emit currentDrawClassChanged(clsName);
}

// 选择颜色按钮点击
void DClassifyManager::onBtnSelectColorClicked()
{
	mCurrentSelectionColor = QColorDialog::getColor(Qt::black, this, "select draw color");
	QString strColor = mCurrentSelectionColor.name();
	QString styleColor = "background-color:" + strColor;
	if (strColor.compare("#000000") == 0)
		return;
	ui.mBtnClassColor->setStyleSheet(styleColor);
}


void DClassifyManager::onBtnNewAClassClicked()
{
	QString className = ui.mLineEditClassName->text();
	if (className.simplified().isEmpty())
		return;

	for each (QString name in *mClassNames)
	{
		if (className.compare(name, Qt::CaseInsensitive) == 0)
		{
			QMessageBox::warning(this, "Error", "The className has existed !", QMessageBox::Ok);
			return;
		}
	}

	emit CreateNewClass(className, mCurrentSelectionColor);
	mDrawColors.insert(className, mCurrentSelectionColor);
	mClassNames->append(className);
	ui.mLineEditClassName->setText("");
	ui.mComboxClassNames->addItem(className);
	addAClassToTree(className, mCurrentSelectionColor);
}

// 当父节点隐藏/显示，子节点隐藏/显示
void DClassifyManager::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
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
	}

	if (item->childCount() > 0)
		emit toChangeAllItemVisible(item->text(0), isChecked);
	else
		emit itemVisibleChanged(item->text(0), isChecked);
}

// 当绘制成果一个样本后，添加到树中
void DClassifyManager::addASampleItemToTree(int id)
{
	QTreeWidgetItem * parentItem = nullptr;
	QString currentClsName = ui.mComboxClassNames->currentText();
	for each (QTreeWidgetItem* item in mClassItems)
	{
		if (item->text(0).compare(currentClsName) == 0)
		{
			parentItem = item;
			break;
		}
	}
	if (!parentItem) return;

	QTreeWidgetItem *itemSample = new QTreeWidgetItem;
	itemSample->setCheckState(0, Qt::Checked);
	itemSample->setText(0, QString("%1").arg(id));
	parentItem->addChild(itemSample);
	
	// 更新计数
	QWidget *w = ui.mMainTreeWidget->itemWidget(parentItem, 2);
	QLabel *label = qobject_cast<QLabel *>(w);
	if (label)
		label->setText(QString("  %1").arg(parentItem->childCount()));
}

void DClassifyManager::onDeleteBtnClicked()
{
	QTreeWidgetItem *selectedItem = ui.mMainTreeWidget->selectedItems().at(0);
	bool hasParent = !(selectedItem->parent() == nullptr);
	if (hasParent)
	{
		QTreeWidgetItem *parentItem = selectedItem->parent();
		QString clsName = parentItem->text(0);
		int sampleId = selectedItem->text(0).toInt();
		emit toRemoveAGeom(clsName, sampleId);
		parentItem->removeChild(selectedItem);
		// 更新计数
		QWidget *w = ui.mMainTreeWidget->itemWidget(parentItem, 2);
		QLabel *label = qobject_cast<QLabel *>(w);
		if (label)
			label->setText(QString("  %1").arg(parentItem->childCount()));
	}
	else
	{
		emit toRemoveAClass(selectedItem->text(0));
		int index = ui.mMainTreeWidget->indexOfTopLevelItem(selectedItem);
		ui.mMainTreeWidget->takeTopLevelItem(index);	// tree中移除结点
		ui.mComboxClassNames->removeItem(index);	// combox中移除结点
		mDrawColors.remove(selectedItem->text(0));

		// 同步导出combox
		//ui.mComboxClassAll->clear();
		//int allCount = ui.mComboxClassNames->count();
		//ui.mWidgetBottom->clear();
		//for (int i = 0; i < allCount; i++)
		//{
		//	QString temName = ui.mComboxClassNames->itemText(i);
		//	ui.mWidgetBottom->addItem(temName);
		//}
	}
}

void DClassifyManager::onChangeClassColorBtnClicked()
{
	QPushButton *btn = qobject_cast<QPushButton *>(sender());
	if (btn)
	{
		QString clsName = btn->statusTip();
		QColor originalColor = mDrawColors.value(clsName);
		QColor newColor = QColorDialog::getColor(Qt::black, this, "select draw color");
		QString strColor = newColor.name();
		if (strColor.compare("#000000") == 0)
			return;
		QString styleColor = "background-color:" + strColor;

		if (newColor.name().compare(originalColor.name(), Qt::CaseInsensitive) != 0)
		{
			btn->setStyleSheet(styleColor);
			emit toChangeDrawStyle(clsName, newColor);
			mDrawColors.insert(clsName, newColor);
		}
	}
}