#pragma once

#include <QWidget>
#include "ui_DClassifyManager.h"
class DClassifyManager : public QWidget
{
	Q_OBJECT

public:
	DClassifyManager(QWidget *parent = Q_NULLPTR);
	~DClassifyManager();

signals: 
	void CreateNewClass(QString &, QColor &);
	void itemsVisibleChanged(QList<QString> &, bool);
	void toChangeAllItemVisible(QString &, bool);
	void itemVisibleChanged(QString&, bool);
	void currentDrawClassChanged(QString &clsName);
	void toRemoveAClass(QString &);
	void toRemoveAGeom(QString &, int);
	void toChangeDrawStyle(QString &, QColor & );
	void toExportSamples(QString&, QString&);


public slots:
	void addASampleItemToTree(int id);

private:
	void init();
	void onBtnSelectColorClicked();
	void onBtnNewAClassClicked();
	void onItemDoubleClicked(QTreeWidgetItem *item, int column);
	void onDeleteBtnClicked();
	void onCurrentTextChanged(int index);
	void onChangeClassColorBtnClicked();

	void addAClassToTree(QString &clsName, QColor &c);
	void updateClassCount(QString &clsName, int count);

private:
	Ui::DClassifyManager ui;
	QColor mCurrentSelectionColor;
	QList<QString>* mClassNames = nullptr;
	QList<QTreeWidgetItem *> mClassItems;
	QMap<QString, QColor> mDrawColors;
};
