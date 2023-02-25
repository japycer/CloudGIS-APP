#pragma once

#include <QWidget>
#include "ui_DWelComePage.h"
#include "DEnums.h"

class DWelComePage : public QWidget
{
	Q_OBJECT
	//	bool isTempory;
	//QString strName;
	//QString strIcon;
	//QString strGmail;
public:
	DWelComePage(DEnums::User* usr, QWidget *parent = Q_NULLPTR);
	~DWelComePage();
	enum AssetsType
	{
		Image,
		Table,
		ImageCollection,
		Undifine
	};

signals:
	void toLoadProject(QString&);
	//void toAddOneCommandToPyinterface(QString&);
	
public slots:
	void addOneTaskRecord(QString&);

private:
	Ui::DWelComePage ui;
	DEnums::User* mCurrentUsr;
	QTreeWidgetItem *topItemRasters;
	QTreeWidgetItem *topItemTables;
	QTreeWidgetItem *topItemOthers;
	
private:
	void initGui();
	void bingSignalsSlots();
	void initUsrInfos();
	void initUsrAssets();
	void initLatestProjects();
	void initUploadTask();
	void initLatestUpdate();
	void onProjectsItemDoubleClicked(QListWidgetItem *item);
	void onBtnContactMeclicked();
	void onUpdateAssets();
	void addOneAssetByFullName(QString&, AssetsType type = AssetsType::Image);
	void toInsertNodeByDiGui(QTreeWidgetItem*, QStringList& ,AssetsType type = AssetsType::Image);
	void onBtnImportAssetsClicked();
	void onBtnCopyAssetsClicked();
};
