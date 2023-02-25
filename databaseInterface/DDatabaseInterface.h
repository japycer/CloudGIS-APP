#pragma once

#include <QtWidgets/QWidget>
#include "ui_DDatabaseInterface.h"
#include"qjsondocument.h"

class QStandardItemModel;
class QSqlQueryModel;
class DDatabaseUtils;
class QSortFilterProxyModel;
class QWebView;
class DGeeInfoBase;

// 用于展示数据库的类，包含数据库信息的显示，Gee搜索结果的展示，数据库入库，数据库查询、数据库某条列展示等数据管理可视化操作
class DDatabaseInterface : public QWidget
{
	Q_OBJECT

public:
	DDatabaseInterface(QWidget *parent = Q_NULLPTR);
	//~DDatabaseInterface();
	void init();	// 初始化数据库
	void exit();	// 退出数据库
	void insert(QVariantMap &res);	// 将res插入数据库
	void updateDatabase();	// 更新当前数据库
	void updateResultToDatabase(bool isSingle = true);	// 更新搜索结果至数据库


private:
	// 更新当前数据库模型
	void initModelAll();
	//初始化tableview的格式
	void initTableView();
	void startToSearchOnline(QString &target);	//一个最新的方法，代替onBtnsearchDataOnlineClicked

private slots:
	void onBtnSearchClicked(bool checked);
	void onBtnsearchDataOnlineClicked();
	void showOnlineSearchRes();
	void onKeywordComboChangedText(QString &currentText);
	void onBtnShowCurrentItemClicked();

protected:
	//bool eventFilter(QObject *watched, QEvent *event);

private:
	Ui::DDatabaseInterfaceClass ui;
	QSqlQueryModel *mModelAll = nullptr;	//用于数据库信息保存
	QSortFilterProxyModel *mProxyModel = nullptr;	//用于排序
	QStandardItemModel *mModelSearchOnline = nullptr;	//用于展示gee搜索信息
	DDatabaseUtils *mdbRunner = nullptr;	//用于执行数据库相关操作
	QWebView * mMainWebview = nullptr;
	QString currentKeyword;	//用于保存当前关键字
	QString insertCmdComplate;	//用于保存数据库插入语句模板
	bool hasUpdateToDb = false;	// 搜索到的数据是否已经更新到数据库
	QString mShowInfoTemplate;
	QStringList allKeyWords;
	DGeeInfoBase *mInfoPanel = nullptr;
	QVariantList mItemList; //用于保存最后一次gee搜索结果
	bool isFirstShown = true;
	bool isFirstSearch = true;
};
