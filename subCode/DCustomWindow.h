#pragma once

#include <QWidget>
#include"DTitleBar.h"
#include <QVBoxLayout>

// ����һ��������ʾ�����࣬ӵ���Զ��������
class DCustomWindow : public QWidget
{
	Q_OBJECT

public:
	DCustomWindow(QWidget *parent=nullptr);
	void addMainWidget(QWidget *w);
	void setCustomTitle(const QString&);
	~DCustomWindow();

private:
	DTitleBar *mTitleBar = nullptr;
	QVBoxLayout *mMainLayout = nullptr;
	QWidget *mMainW = nullptr;
};
