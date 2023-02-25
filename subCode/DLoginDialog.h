#pragma once

#include <QDialog>
#include "DEnums.h"
#include "ui_DLoginDialog.h"

class DDatabaseUtils;

class DLoginDialog : public QDialog
{
	Q_OBJECT

public:
	DLoginDialog(DEnums::User* usr, QDialog *parent = Q_NULLPTR);
	~DLoginDialog();

private:
	Ui::DLoginDialog ui;
	DDatabaseUtils* mdbRunner = nullptr;
	DEnums::User *mUsr;

private:
	void initGui();
	void bindSignsSlots();
	void OnBtnRegisterClicked();
	void onBtnLoginClicked();
};
