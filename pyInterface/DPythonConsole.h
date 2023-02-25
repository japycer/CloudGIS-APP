#pragma once

#include <QWidget>
#include "ui_DPythonConsole.h"
#include<QKeyEvent>

class DPythonConsole : public QWidget
{
	Q_OBJECT

public:
	DPythonConsole(QWidget *parent = Q_NULLPTR);
	~DPythonConsole();
	//向python输出窗口输出内容，outP为输出内容， successFlag为运行成功标志， tips为添加的提示信息
	void setOutPut(QString &outP, bool successFlag = true, QString &tips = QString("Start To Run ..."));
	
signals:
	void runACommand(QString &command);

protected:
	void keyPressEvent(QKeyEvent *event)override;

private:
	Ui::DPythonConsole ui;
	QList<QString> historys;	//保存执行的命令
	int currentIndex = 0;
	void onRunBtnClicked();
	void onPreviousBtnClicked();
	void onNextBtnClicked();
	void onClearBtnClicked();
};
