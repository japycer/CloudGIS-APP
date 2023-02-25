#pragma once

#include <QWidget>
#include "ui_DGeeExporter.h"

class DGeeExporter : public QWidget
{
	Q_OBJECT

public:
	DGeeExporter(QWidget *parent = Q_NULLPTR);
	~DGeeExporter();

private:
	Ui::DGeeExporter ui;
};
