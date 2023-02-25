#include "DSliderPlus.h"

DSliderPlus::DSliderPlus(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.mMainHorizontalSlider->setMaximum(100);
	ui.mMainHorizontalSlider->setMinimum(0);
	ui.mMainSpinBox->setMaximum(100);
	ui.mMainSpinBox->setMinimum(0);

	connect(ui.mMainSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui.mMainHorizontalSlider, &QSlider::setValue);
	connect(ui.mMainHorizontalSlider, QOverload<int>::of(&QSlider::valueChanged), ui.mMainSpinBox, &QSpinBox::setValue);
}

DSliderPlus::~DSliderPlus()
{
}

void DSliderPlus::setValue(int valuee)
{
	ui.mMainHorizontalSlider->setValue(valuee);
}


void DSliderPlus::setValueRange(int minimum, int maximum)
{
	ui.mMainHorizontalSlider->setMaximum(maximum);
	ui.mMainSpinBox->setMaximum(maximum);

	ui.mMainHorizontalSlider->setMinimum(minimum);
	ui.mMainSpinBox->setMinimum(minimum);
}

int DSliderPlus::getCurrentValue()
{
	return ui.mMainSpinBox->value();
}

void DSliderPlus::setEnable(bool flag)
{
	ui.mMainHorizontalSlider->setEnabled(flag);
	ui.mMainSpinBox->setEnabled(flag);
}