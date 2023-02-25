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

// ����չʾ���ݿ���࣬�������ݿ���Ϣ����ʾ��Gee���������չʾ�����ݿ���⣬���ݿ��ѯ�����ݿ�ĳ����չʾ�����ݹ�����ӻ�����
class DDatabaseInterface : public QWidget
{
	Q_OBJECT

public:
	DDatabaseInterface(QWidget *parent = Q_NULLPTR);
	//~DDatabaseInterface();
	void init();	// ��ʼ�����ݿ�
	void exit();	// �˳����ݿ�
	void insert(QVariantMap &res);	// ��res�������ݿ�
	void updateDatabase();	// ���µ�ǰ���ݿ�
	void updateResultToDatabase(bool isSingle = true);	// ����������������ݿ�


private:
	// ���µ�ǰ���ݿ�ģ��
	void initModelAll();
	//��ʼ��tableview�ĸ�ʽ
	void initTableView();
	void startToSearchOnline(QString &target);	//һ�����µķ���������onBtnsearchDataOnlineClicked

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
	QSqlQueryModel *mModelAll = nullptr;	//�������ݿ���Ϣ����
	QSortFilterProxyModel *mProxyModel = nullptr;	//��������
	QStandardItemModel *mModelSearchOnline = nullptr;	//����չʾgee������Ϣ
	DDatabaseUtils *mdbRunner = nullptr;	//����ִ�����ݿ���ز���
	QWebView * mMainWebview = nullptr;
	QString currentKeyword;	//���ڱ��浱ǰ�ؼ���
	QString insertCmdComplate;	//���ڱ������ݿ�������ģ��
	bool hasUpdateToDb = false;	// �������������Ƿ��Ѿ����µ����ݿ�
	QString mShowInfoTemplate;
	QStringList allKeyWords;
	DGeeInfoBase *mInfoPanel = nullptr;
	QVariantList mItemList; //���ڱ������һ��gee�������
	bool isFirstShown = true;
	bool isFirstSearch = true;
};
