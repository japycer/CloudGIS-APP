#pragma once

#include <QWidget>
#include"DTitleBar.h"
#include <QVBoxLayout>

// 这是一个容器显示窗口类，拥有自定义标题栏
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
