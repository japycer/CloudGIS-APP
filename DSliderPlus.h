#pragma once

#include <QWidget>
#include "ui_DSliderPlus.h"

class DSliderPlus : public QWidget
{
	Q_OBJECT

public:
	DSliderPlus(QWidget *parent = Q_NULLPTR);
	~DSliderPlus();
	void setValue(int);
	void setValueRange(int minimum = 0, int maximum = 100);
	void setEnable(bool);
	int getCurrentValue();

private:
	Ui::DSliderPlus ui;
};
