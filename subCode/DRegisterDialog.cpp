#include "DRegisterDialog.h"
#include <QMessageBox>
#include<QIntValidator>
#include<QFile>
#include <QSqlError>
#include <QDebug>
#include "../databaseInterface/DDatabaseUtils.h"


DRegisterDialog::DRegisterDialog(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);
	mdbRunner = DDatabaseUtils::Instance();
	mdbRunner->connectDatabase(QStringLiteral("geeAssets.db"));
	QString createTableCommand("create table if not exists users(username int primary key, e_mail text, psd text, g_mail text)");
	mdbRunner->createTable(createTableCommand);

	ui.lineEdit_userName->installEventFilter(this);
	ui.lineEdit_email->installEventFilter(this);
	ui.lineEdit_psd->installEventFilter(this);
	ui.lineEdit_psd2->installEventFilter(this);
	ui.lineEdit_gmail->installEventFilter(this);

	QRegExp exp("[a-zA-Z0-9-_]+@[a-zA-Z0-9-_]+\\.[a-zA-Z]+");
	QRegExpValidator *rval = new QRegExpValidator(exp);
	ui.lineEdit_email->setValidator(rval);
	ui.lineEdit_gmail->setValidator(rval);
}


DRegisterDialog::~DRegisterDialog()
{

}

void DRegisterDialog::on_btn_ok_clicked()
{
	if (!checkEditUserName().compare("Certain") == 0)
	{
		QMessageBox::warning(this, "error", "username is incorrect ");
		return;
	}
	else if (!checkEditEmail().compare("Certain") == 0)
	{
		QMessageBox::warning(this, "error", "E-mail is incorrect ");
		return;
	}

	else if (!checkEditpsd1().compare("Certain") == 0)
	{
		QMessageBox::warning(this, "error", "password is incorrect ");
		return;
	}
	else if (!checkEditpsd2().compare("Certain") == 0)
	{
		QMessageBox::warning(this, "error", "password2 is incorrect ");
		return;
	}
	else if (!checkEditgmail().compare("Certain") == 0)
	{
		QMessageBox::warning(this, "error", "G-mail is incorrect ");
		return;
	}

	QString str_name = ui.lineEdit_userName->text();
	QString str_email = ui.lineEdit_email->text();
	QString str_psd = ui.lineEdit_psd->text();
	QString str_gmail = ui.lineEdit_gmail->text();

	QString query = QString("INSERT INTO users VALUES(\"%1\", \"%2\", \"%3\",\"%4\")").arg(str_name)\
		.arg(str_email).arg(str_psd).arg(str_gmail);

	//插入数据
	if (!mdbRunner->getSqlQuery()->exec(query))
		QMessageBox::critical(nullptr, "Fail to register user", mdbRunner->getSqlQuery()->lastError().text(), QMessageBox::Ok);

	//查询数据
	mdbRunner->getSqlQuery()->exec("select * from users");
	if (!mdbRunner->getSqlQuery()->exec())
		QMessageBox::critical(nullptr, "Fail to register user", mdbRunner->getSqlQuery()->lastError().text(), QMessageBox::Ok);
	else
	{
		while (mdbRunner->getSqlQuery()->next())
		{
			QString userName = mdbRunner->getSqlQuery()->value(0).toString();
			QString email = mdbRunner->getSqlQuery()->value(1).toString();
			QString psd = mdbRunner->getSqlQuery()->value(2).toString();
			QString gmail = mdbRunner->getSqlQuery()->value(3).toString();

			QMessageBox::critical(nullptr, "Succeed to register user", QString("userName:%1    email:%2    psd:%3    gmail:%4").arg(userName).arg(email).arg(psd).arg(gmail), QMessageBox::Ok);
		}
	}
}


void DRegisterDialog::on_btn_quit_clicked()
{
	//关闭数据库
	mdbRunner->closeDatabase();
	this->close();
}


bool DRegisterDialog::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::FocusOut) {
		if (obj == ui.lineEdit_userName) {
			QString res = checkEditUserName();
			if (res.compare(QString("Certain")) == 0)
				ui.label_t1->setStyleSheet(QString("color:green;"));
			else
				ui.label_t1->setStyleSheet(QString("color:red;"));
			ui.label_t1->setText(res);
		}
		else if (obj == ui.lineEdit_email) {
			QString res = checkEditEmail();
			if (res.compare(QString("Certain")) == 0)
				ui.label_t2->setStyleSheet(QString("color:green;"));
			else
				ui.label_t2->setStyleSheet(QString("color:red;"));
			ui.label_t2->setText(res);
		}
		else if (obj == ui.lineEdit_psd) {
			QString res = checkEditpsd1();
			if (res.compare(QString("Certain")) == 0)
				ui.label_t3->setStyleSheet(QString("color:green;"));
			else
				ui.label_t3->setStyleSheet(QString("color:red;"));
			ui.label_t3->setText(res);
		}
		else if (obj == ui.lineEdit_psd2) {
			QString res = checkEditpsd2();
			if (res.compare(QString("Certain")) == 0)
				ui.label_t4->setStyleSheet(QString("color:green;"));
			else
				ui.label_t4->setStyleSheet(QString("color:red;"));
			ui.label_t4->setText(res);
		}
		else if (obj == ui.lineEdit_gmail) {
			QString res = checkEditgmail();
			if (res.compare(QString("Certain")) == 0)
				ui.label_t5->setStyleSheet(QString("color:green;"));
			else
				ui.label_t5->setStyleSheet(QString("color:red;"));
			ui.label_t5->setText(res);
		}
	}

	return QWidget::eventFilter(obj, e);
}

QString DRegisterDialog::checkEditUserName()
{
	if (ui.lineEdit_userName->text().isEmpty()) {
		return QString("Error empty !");
	}

	mdbRunner->getSqlQuery()->exec("select * from users");
	if (!mdbRunner->getSqlQuery()->exec())
		return QString("Unkown Error!");
		//QMessageBox::warning(nullptr, "Fail to link database", mdbRunner->getSqlQuery()->lastError(), QMessageBox::Ok);
	else
	{
		while (mdbRunner->getSqlQuery()->next())
		{
			QString userName = mdbRunner->getSqlQuery()->value(0).toString();
			if (userName.compare(ui.lineEdit_userName->text()) == 0)
				return QString("Repeated name");
		}
	}
	return QString("Certain");
}

QString DRegisterDialog::checkEditEmail()
{
	const QValidator *v = ui.lineEdit_email->validator();
	int pos = 0;
	QString str(ui.lineEdit_email->text());
	if (v->validate(str, pos) != QValidator::Acceptable) {
		return QString("E-mail format error");
	}
	return QString("Certain");
}

QString DRegisterDialog::checkEditpsd1()
{
	if (ui.lineEdit_psd->text().isEmpty()) {
		return QString("Error empty !");
	}
	return QString("Certain");
}

QString DRegisterDialog::checkEditpsd2()
{
	if (!ui.lineEdit_psd2->text().compare(ui.lineEdit_psd->text()) == 0)
	{
		return QString("Not equal !");
	}
	if (ui.lineEdit_psd2->text().isEmpty()) {
		return QString("Error empty !");
	}
	return QString("Certain");
}

QString DRegisterDialog::checkEditgmail()
{
	const QValidator *v = ui.lineEdit_gmail->validator();
	int pos = 0;
	QString str(ui.lineEdit_gmail->text());
	if (v->validate(str, pos) != QValidator::Acceptable) {
		return QString("G-mail format error");
	}
	return QString("Certain");
}

void DRegisterDialog::on_lineEdit_psd_textChanged(const QString &arg1)
{
	QString res = checkEditpsd1();
	if (res.compare(QString("Certain")) == 0)
		ui.label_t3->setStyleSheet(QString("color:green;"));
	else
		ui.label_t3->setStyleSheet(QString("color:red;"));
	ui.label_t3->setText(res);
}

void DRegisterDialog::on_lineEdit_userName_textChanged(const QString &arg1)
{
	QString res = checkEditUserName();
	if (res.compare(QString("Certain")) == 0)
		ui.label_t1->setStyleSheet(QString("color:green;"));
	else
		ui.label_t1->setStyleSheet(QString("color:red;"));
	ui.label_t1->setText(res);
}


void DRegisterDialog::on_lineEdit_psd2_textChanged(const QString &arg1)
{
	QString res = checkEditpsd2();
	if (res.compare(QString("Certain")) == 0)
		ui.label_t4->setStyleSheet(QString("color:green;"));
	else
		ui.label_t4->setStyleSheet(QString("color:red;"));
	ui.label_t4->setText(res);
}

void DRegisterDialog::on_lineEdit_email_textChanged(const QString &arg1)
{
	QString res = checkEditEmail();
	if (res.compare(QString("Certain")) == 0)
		ui.label_t2->setStyleSheet(QString("color:green;"));
	else
		ui.label_t2->setStyleSheet(QString("color:red;"));
	ui.label_t2->setText(res);
}

void DRegisterDialog::on_lineEdit_gmail_textChanged(const QString &arg1)
{
	QString res = checkEditgmail();
	if (res.compare(QString("Certain")) == 0)
		ui.label_t5->setStyleSheet(QString("color:green;"));
	else
		ui.label_t5->setStyleSheet(QString("color:red;"));
	ui.label_t5->setText(res);
}
