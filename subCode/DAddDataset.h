#pragma once

#include <QWidget>
#include "ui_DAddDataset.h"

class QTextBrowser;
class QCalendarWidget;
class DEELayerShowOptionWin;
class DImageViewer;
class DImgCollectionViewer;

class DAddDataset : public QWidget
{
	Q_OBJECT

public:
	DAddDataset(QWidget *parent = Q_NULLPTR);
	~DAddDataset();
	void setCurrentIndex(int index);

signals:
	//! Emitted when a vector layer was selected for addition: for signal forwarding to QgisApp
	void addVectorLayer(const QString &fileName, const QString &encode, const QString &providerKey);
	void addRasterLayer(const QString &fileName);
	void addWmsLayer();
	void testCurrentLink(const QString &URL, const QString &layerName, bool *res);

private slots:
	void onAddBtnClicked();
	void writeSelectionDate();
	void initPage4AddBaseMap();
	void slotInputEESnippetIDFinished();
	void onGeeSummitBtnClicked();
	void onShowBandDetailsBtnClicked();
	void onSummitFilterBtnClicked();

private:
	void initGui();
	void bindSignalsSlots();
	bool compareDateBetween();		// 判断输入事件是否正确
	void runPostOperation();	// 进行数据集后处理

private:
	Ui::DAddDataset ui;
	QCalendarWidget *mCalendar = nullptr;
	bool mIsLeft = true;
	bool hasInitPage4AddBaseMap = false;
	QVariantList mItemList;
	// QMap<QString, int>* mFilterBounds = nullptr;
	bool mSummitGeeFlag = false;  // 数据集添加成功标志
	QTextBrowser *mShowImgCollectionDetailsWind = nullptr;
	QString mLastBandsNames;
	DEELayerShowOptionWin *mEELayerShowOptionWin = nullptr;
	QString mCurrentShowParams;
	DImageViewer* mImageViewer = nullptr;
	DImgCollectionViewer* mImageCollectionViewer = nullptr;
	bool succeedSummitFilters = false;	// 数据集筛选成功标志
	bool succeedPostOperation = false;  // 后处理成功标志
};
