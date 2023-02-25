#include "DGeeObjectManager.h"
#include "DPyGeeUtils.h"
#include <QMenu>
#include <QClipBoard>

// 单例模式
DGeeObjectManager* DGeeObjectManager::mInstance = nullptr;

DGeeObjectManager* DGeeObjectManager::Instance(QWidget *parent)
{
	if (mInstance == nullptr)
	{
		mInstance = new DGeeObjectManager(parent);
	}
	return mInstance;
}

// Private constructor
DGeeObjectManager::DGeeObjectManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initUI();
	bindSignalsSlots();
}

// public disconstructor
DGeeObjectManager::~DGeeObjectManager()
{
	delete topItemImage;
	delete topItemImgCol;
	delete topItemGeometry;
	delete topItemFeature;
	delete topItemFeatureCol;
	delete topItemClassifier;
	delete topItemOthers;
}

void DGeeObjectManager::initUI()
{
	// 初始化treeWidget
	ui.mMainTreeWidget->setColumnCount(2);		// 2列
	ui.mMainTreeWidget->setHeaderLabels(QStringList() << "Obj Name" << "Count");
	ui.mMainTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection); // 单选

	topItemImage = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemImage->setText(0, "Image");
	ui.mMainTreeWidget->addTopLevelItem(topItemImage);

	topItemImgCol = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemImgCol->setText(0, "ImageCollection");
	ui.mMainTreeWidget->addTopLevelItem(topItemImgCol);

	topItemGeometry = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemGeometry->setText(0, "Geometry");
	ui.mMainTreeWidget->addTopLevelItem(topItemGeometry);

	topItemFeature = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemFeature->setText(0, "Feature");
	ui.mMainTreeWidget->addTopLevelItem(topItemFeature);

	topItemFeatureCol = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemFeatureCol->setText(0, "FeatureCollection");
	ui.mMainTreeWidget->addTopLevelItem(topItemFeatureCol);

	topItemClassifier = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemClassifier->setText(0, "Classifier");
	ui.mMainTreeWidget->addTopLevelItem(topItemClassifier);

	topItemOthers = new QTreeWidgetItem(ui.mMainTreeWidget);
	topItemOthers->setText(0, "Others");
	ui.mMainTreeWidget->addTopLevelItem(topItemOthers);

	ui.mMainTreeWidget->expandAll();
}


void DGeeObjectManager::bindSignalsSlots()
{
	connect(ui.mBtnQuit, &QPushButton::clicked, this, &DGeeObjectManager::onQuitBtnClicked);
	connect(ui.mBtnDeleteSelect, &QPushButton::clicked, this, &DGeeObjectManager::onDeleteSelectedBtnClicked);
	connect(ui.mMainTreeWidget, &QTreeWidget::itemPressed, this, &DGeeObjectManager::onItemPressed);
}


void DGeeObjectManager::onQuitBtnClicked()
{
	if (this->parent())
		qobject_cast<QWidget *>(this->parent())->hide();
	else
		this->hide();
}

// Add a record to treeWidget
bool DGeeObjectManager::addOneRecord(QString& objName, GeeObjType eeObjType, int count)
{
	bool resImport = DPyGeeUtils::runString(QString("from EEPlugin import DUtils"));
	if (!resImport) return false;

	bool hasExisted = false;
	//for each (QString str in mRecoeds)
	//{
	//	if (str.compare(objName) == 0)
	//	{
	//		hasExisted = true;
	//		break;
	//	}
	//}

	GeeObjType previousType = GeeObjType::Undefine;
	QTreeWidgetItem* findItem = nullptr;
	previousType = findObjTypeByName(objName, findItem);
	if (previousType != GeeObjType::Undefine)
		hasExisted = true;

	if (eeObjType == GeeObjType::Undefine)
	{
		QString resType = DPyGeeUtils::getEEObjType(objName);
		//int ress = resType.compare("ImageCollection");
		if (resType.startsWith("ERROR"))
			return false;
		else if (resType.compare("Image") == 0)
			eeObjType = GeeObjType::Image;
		else if (resType.compare("ImageCollection") == 0)
			eeObjType = GeeObjType::ImageCollection;
		else if (resType.compare("Feature") == 0)
			eeObjType = GeeObjType::Feature;
		else if (resType.compare("FeatureCollection") == 0)
			eeObjType = GeeObjType::FeatureCollection;
		else if (resType.compare("Point") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("MultiPoint") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("LineString") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("MultiLineString") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("Polygon") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("MultiPolygon") == 0)
			eeObjType = GeeObjType::Geometry;
		else if (resType.compare("Classifier") == 0)
			eeObjType = GeeObjType::Classifier;
		else
			eeObjType = GeeObjType::Others;
	}

	QRegExp regVar("[a-zA-Z_]+\\w*");
	bool compareRes = regVar.exactMatch(objName);
	if (!compareRes) return false;

	QString strCount;
	if (count != -1)
	{
		strCount = QString("%1").arg(count);
	}
	else
	{
		bool resCompute = DPyGeeUtils::runString(QString("print(DUtils.getObjCount(%1))").arg(objName));
		if (resCompute)
			strCount = DPyGeeUtils::getLatestRunOut(true);
		else
		{
			DPyGeeUtils::getLatestRunOut(false);
			strCount = QString("1");
		}
	}

	if (hasExisted)
	{
		if (eeObjType == previousType)
		{
			if (findItem != nullptr)
				findItem->setText(1, strCount);
		}
		else
		{
			switch (previousType)
			{
			case DGeeObjectManager::Image:
				topItemImage->removeChild(findItem);
				break;
			case DGeeObjectManager::ImageCollection:
				topItemImgCol->removeChild(findItem);
				break;
			case DGeeObjectManager::Geometry:
				topItemGeometry->removeChild(findItem);
				break;
			case DGeeObjectManager::Feature:
				topItemFeature->removeChild(findItem);
				break;
			case DGeeObjectManager::FeatureCollection:
				topItemFeatureCol->removeChild(findItem);
				break;
			case DGeeObjectManager::Classifier:
				topItemClassifier->removeChild(findItem);
				break;
			case DGeeObjectManager::Others:
				topItemOthers->removeChild(findItem);
				break;
			case DGeeObjectManager::Undefine:
				break;
			default:
				break;
			}

			QTreeWidgetItem *itemRecord = new QTreeWidgetItem;
			itemRecord->setText(0, objName);
			itemRecord->setText(1, strCount);

			switch (eeObjType)
			{
			case DGeeObjectManager::Image:
				topItemImage->addChild(itemRecord);
				break;
			case DGeeObjectManager::ImageCollection:
				topItemImgCol->addChild(itemRecord);
				break;
			case DGeeObjectManager::Geometry:
				topItemGeometry->addChild(itemRecord);
				break;
			case DGeeObjectManager::Feature:
				topItemFeature->addChild(itemRecord);
				break;
			case DGeeObjectManager::FeatureCollection:
				topItemFeatureCol->addChild(itemRecord);
				break;
			case DGeeObjectManager::Classifier:
				topItemClassifier->addChild(itemRecord);
				break;
			case DGeeObjectManager::Others:
				topItemOthers->addChild(itemRecord);
				break;
			case DGeeObjectManager::Undefine:
				break;
			default:
				break;
			}
		}
	}
	else
	{
		QTreeWidgetItem *itemRecord = new QTreeWidgetItem;
		itemRecord->setText(0, objName);
		itemRecord->setText(1, strCount);

		mRecoeds << objName;

		switch (eeObjType)
		{
		case DGeeObjectManager::Image:
			topItemImage->addChild(itemRecord);
			break;
		case DGeeObjectManager::ImageCollection:
			topItemImgCol->addChild(itemRecord);
			break;
		case DGeeObjectManager::Geometry:
			topItemGeometry->addChild(itemRecord);
			break;
		case DGeeObjectManager::Feature:
			topItemFeature->addChild(itemRecord);
			break;
		case DGeeObjectManager::FeatureCollection:
			topItemFeatureCol->addChild(itemRecord);
			break;
		case DGeeObjectManager::Classifier:
			topItemClassifier->addChild(itemRecord);
			break;
		case DGeeObjectManager::Others:
			topItemOthers->addChild(itemRecord);
			break;
		case DGeeObjectManager::Undefine:
			break;
		default:
			break;
		}
	}
	return true;
}


// 删除对象并且释放空间
void DGeeObjectManager::onDeleteSelectedBtnClicked()
{
	QTreeWidgetItem *selectedItem = ui.mMainTreeWidget->selectedItems().at(0);
	bool hasParent = !(selectedItem->parent() == nullptr);
	if (hasParent)
	{
		QTreeWidgetItem *parentItem = selectedItem->parent();
		QString objName = selectedItem->text(0);
		parentItem->removeChild(selectedItem);
		DPyGeeUtils::runString(QString("delete %1").arg(objName));
		emit toDeleteOnePyObj(objName);

		for (int i = 0; i < mRecoeds.count(); i++)
		{
			if (mRecoeds.at(i).compare(objName) == 0)
			{
				mRecoeds.removeAt(i);
				break;
			}
		}
	}
}


QStringList DGeeObjectManager::getGeometryAndFeatures()
{
	QStringList res;
	int childCount = topItemGeometry->childCount();
	for (int i = 0; i < childCount; i++)
	{
		auto child = topItemGeometry->child(i);
		res << child->text(0);
	}

	int childCount2 = topItemFeature->childCount();
	if (childCount2 == 0)
		return res;
	for (int i = 0; i < childCount2; i++)
	{
		auto child = topItemFeature->child(i);
		res << child->text(0);
	}
	return res;
}

QStringList DGeeObjectManager::getGeometrys()
{
	QStringList res;
	int childCount = topItemGeometry->childCount();
	if (childCount == 0)
		return res;

	for (int i = 0; i < childCount; i++)
	{
		auto child = topItemGeometry->child(i);
		res << child->text(0);
	}
	return res;
}


void DGeeObjectManager::slotDeleteItem(QTreeWidgetItem* item)
{
	bool hasParent = !(item->parent() == nullptr);
	if (hasParent)
	{
		QTreeWidgetItem *parentItem = item->parent();
		QString objName = item->text(0);
		parentItem->removeChild(item);
		DPyGeeUtils::runString(QString("del %1").arg(objName));
		emit toDeleteOnePyObj(objName);

		for (int i = 0; i < mRecoeds.count(); i++)
		{
			if (mRecoeds.at(i).compare(objName) == 0)
			{
				mRecoeds.removeAt(i);
				break;
			}
		}
	}
}


// 复制选中内容到剪贴板
void DGeeObjectManager::slotCopyItemContent(QTreeWidgetItem* item)
{
	bool hasParent = !(item->parent() == nullptr);
	if (hasParent)
	{
		QString objName = item->text(0);
		QApplication::clipboard()->setText(objName);
	}
}

DGeeObjectManager::GeeObjType DGeeObjectManager::findObjTypeByName(QString & name, QTreeWidgetItem * item)
{
	GeeObjType type = GeeObjType::Undefine;
	int count = 0;

	count = topItemImage->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemImage->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::Image;
		}
	}

	count = topItemImgCol->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemImgCol->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::ImageCollection;
		}

	}

	count = topItemGeometry->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemGeometry->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::Geometry;
		}
	}

	count = topItemFeature->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemFeature->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::Feature;
		}
	}

	count = topItemFeatureCol->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemFeatureCol->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::FeatureCollection;
		}
	}

	count = topItemClassifier->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemClassifier->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::Classifier;
		}
	}

	count = topItemOthers->childCount();
	for (int i = 0; i < count; i++)
	{
		auto pChild = topItemOthers->child(i);
		QString childName = pChild->text(0);
		if (name.compare(childName) == 0)
		{
			item = pChild;
			return GeeObjType::Others;
		}
	}
	return GeeObjType::Undefine;
}


void DGeeObjectManager::onItemPressed(QTreeWidgetItem* item, int column)
{
	if (qApp->mouseButtons() == Qt::RightButton)
	{
		QMenu*menu = new QMenu(ui.mMainTreeWidget);
		if (item->parent()!=nullptr)
		{
			QAction* copyVarName = menu->addAction("copy");
			QAction* deleteVarName = menu->addAction("delete");

			connect(copyVarName, &QAction::triggered, [=] {slotCopyItemContent(item); });
			connect(deleteVarName, &QAction::triggered, [=] {slotDeleteItem(item); });

			menu->exec(QCursor::pos());   //菜单弹出位置为鼠标点击位置
		}
	}
}


QStringList DGeeObjectManager::getObjectsByType(DGeeObjectManager::GeeObjType type)
{
	QTreeWidgetItem* targetItem = nullptr;
	switch (type)
	{
	case DGeeObjectManager::Image:
		targetItem = topItemImage;
		break;
	case DGeeObjectManager::ImageCollection:
		targetItem = topItemImgCol;
		break;
	case DGeeObjectManager::Geometry:
		targetItem = topItemGeometry;
		break;
	case DGeeObjectManager::Feature:
		targetItem = topItemFeature;
		break;
	case DGeeObjectManager::FeatureCollection:
		targetItem = topItemFeatureCol;
		break;
	case DGeeObjectManager::Classifier:
		targetItem = topItemClassifier;
		break;
	case DGeeObjectManager::Others:
		targetItem = topItemOthers;
		break;
	case DGeeObjectManager::Undefine:
		break;
	default:
		break;
	}
	
	QStringList res;
	if (targetItem)
	{
		int count = targetItem->childCount();
		for (int i = 0; i < count; i++)
		{
			auto pChild = topItemFeature->child(i);
			res << pChild->text(0);
		}
	}
	return res;
}