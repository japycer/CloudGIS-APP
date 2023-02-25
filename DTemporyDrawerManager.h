#pragma once

#include <QWidget>
#include"ui_DTemporyDrawerManager.h"

class DTemporyDrawerManager : public QWidget
{
	Q_OBJECT

public:
	DTemporyDrawerManager(QWidget *parent = Q_NULLPTR);
	~DTemporyDrawerManager();

public slots:
	void insertAItemToTree(QString &name);

public:
	enum DrawType
	{
		Point,
		LineString,
		MultiPolygon,
		Rectangle,
		UnKnown
	};

signals:
	void drawStyleChanged(DTemporyDrawerManager::DrawType &, QColor &);
	void itemsVisibleChanged(QList<QString> &, bool);
	void toDeleteAGeometry(QList<QString> &);

private:
	Ui::DTemporyDrawerManager ui;
	QColor currentSelectionColor;
	QTreeWidgetItem *topItem1 = nullptr;
	QTreeWidgetItem *topItem2 = nullptr;
	QTreeWidgetItem *topItem3 = nullptr;
	QTreeWidgetItem *topItem4 = nullptr;
	DTemporyDrawerManager::DrawType currentDrawType;
	void initGui();
	void onOkClicked();
	void onSelectColorBtnClicked();
	void onItemDoubleClicked(QTreeWidgetItem *item, int column);
	void onBtnDeleteClicked();
	DTemporyDrawerManager::DrawType judgetCurrentDrawType();
};
