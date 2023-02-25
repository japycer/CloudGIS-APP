#pragma once

#include <QWidget>
#include "ui_DGeeInfoBase.h"

class DGeeInfoBase : public QWidget
{
	Q_OBJECT

public:
	DGeeInfoBase(QWidget *parent = Q_NULLPTR);
	~DGeeInfoBase();
	void DGeeInfoBase::setLabelContent(QString & Title, QString & dateAvalibility, QString & snippet, QString & catalogLink);

private:
	Ui::DGeeInfoBase ui;
};
