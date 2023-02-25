#pragma once

#include <QDialog>
#include "ui_DRegisterDialog.h"
class DDatabaseUtils;

class DRegisterDialog : public QDialog
{
	Q_OBJECT

public:
	DRegisterDialog(QWidget *parent = Q_NULLPTR);
	~DRegisterDialog();

private:
	Ui::DRegisterDialog ui;
	DDatabaseUtils* mdbRunner = nullptr;

protected:
	bool eventFilter(QObject *, QEvent *);

private:
	void on_btn_ok_clicked();
	void on_btn_quit_clicked();
	void on_lineEdit_psd_textChanged(const QString &arg1);
	void on_lineEdit_userName_textChanged(const QString &arg1);
	void on_lineEdit_psd2_textChanged(const QString &arg1);
	void on_lineEdit_email_textChanged(const QString &arg1);
	void on_lineEdit_gmail_textChanged(const QString &arg1);
	QString checkEditUserName();
	QString checkEditEmail();
	QString checkEditpsd1();
	QString checkEditpsd2();
	QString checkEditgmail();
};
