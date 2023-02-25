#pragma once

#include <QWidget>
#include "ui_DGeeObjectManager.h"

// 这是一个gee python变量管理类，单例模式
class DGeeObjectManager : public QWidget
{
	Q_OBJECT

public:
	static DGeeObjectManager* Instance(QWidget *parent = Q_NULLPTR);
	~DGeeObjectManager();
	QStringList getGeometryAndFeatures();
	QStringList getGeometrys();

	enum GeeObjType
	{
		Image,
		ImageCollection,
		Geometry,
		Feature,
		FeatureCollection,
		Classifier,
		Others,
		Undefine
	};
	bool addOneRecord(QString& objName, GeeObjType eeObjType = GeeObjType::Undefine, int count = -1);
	QStringList getObjectsByType(GeeObjType);

signals:
	void toDeleteOnePyObj(QString&);

private:
	DGeeObjectManager(QWidget *parent = Q_NULLPTR);
	void initUI();
	void bindSignalsSlots();
	void onQuitBtnClicked();
	void onDeleteSelectedBtnClicked();
	void onItemPressed(QTreeWidgetItem*, int);
	void slotDeleteItem(QTreeWidgetItem*);
	void slotCopyItemContent(QTreeWidgetItem*);
	DGeeObjectManager::GeeObjType findObjTypeByName(QString&, QTreeWidgetItem*);

private:
	Ui::DGeeObjectManager ui;
	QStringList mRecoeds;
	static DGeeObjectManager* mInstance;
	QTreeWidgetItem* topItemImage = nullptr;	// Image
	QTreeWidgetItem* topItemImgCol = nullptr;	// ImageCollection
	QTreeWidgetItem* topItemGeometry = nullptr;	// Geometry
	QTreeWidgetItem* topItemFeature = nullptr;	// Feature
	QTreeWidgetItem* topItemFeatureCol = nullptr;
	QTreeWidgetItem* topItemClassifier = nullptr;
	QTreeWidgetItem* topItemOthers = nullptr;
};
