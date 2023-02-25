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
	//��python�������������ݣ�outPΪ������ݣ� successFlagΪ���гɹ���־�� tipsΪ��ӵ���ʾ��Ϣ
	void setOutPut(QString &outP, bool successFlag = true, QString &tips = QString("Start To Run ..."));
	
signals:
	void runACommand(QString &command);

protected:
	void keyPressEvent(QKeyEvent *event)override;

private:
	Ui::DPythonConsole ui;
	QList<QString> historys;	//����ִ�е�����
	int currentIndex = 0;
	void onRunBtnClicked();
	void onPreviousBtnClicked();
	void onNextBtnClicked();
	void onClearBtnClicked();
};
