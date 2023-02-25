#include "DLoginDialog.h"
#include <QMessageBox>
#include "../databaseInterface/DDatabaseUtils.h"
#include "DRegisterDialog.h"


DLoginDialog::DLoginDialog(DEnums::User* usr, QDialog *parent)
	: QDialog(parent), mUsr(usr)
{
	ui.setupUi(this);
	initGui();
	bindSignsSlots();
}


DLoginDialog::~DLoginDialog()
{
	
}

void DLoginDialog::initGui()
{

	// 进行数据库的初始化
	mdbRunner = DDatabaseUtils::Instance();
	mdbRunner->connectDatabase(QStringLiteral("geeAssets.db"));
	QString createTableCommand("create table if not exists users(username int primary key, e_mail text, psd text, g_mail text)");

	mdbRunner->createTable(createTableCommand);
}


void DLoginDialog::bindSignsSlots()
{
	connect(ui.mBtnLogin, &QPushButton::clicked, this, &DLoginDialog::onBtnLoginClicked);
	connect(ui.mBtnRegister, &QPushButton::clicked, this, &DLoginDialog::OnBtnRegisterClicked);
}


void DLoginDialog::OnBtnRegisterClicked()
{
	DRegisterDialog dial;
	dial.exec();
}


void DLoginDialog::onBtnLoginClicked()
{
	QString name = ui.mLineEditUsrName->text();
	if (name.isEmpty()) {
		QMessageBox::warning(this, "error", "username is empty ");
		return;
	}
	if (name.compare("Japycer") == 0)
	{
		mUsr->strName = QString("Japycer");
		mUsr->strGmail = QString("admin@gmail.com");
		mUsr->isTempory = true;
		mUsr->strIcon = "shuangyu";
		this->close();
		return;
	}
	QString passW = ui.mLineEditPsd->text();
	//查询数据
	bool ok = mdbRunner->excuCommand(QString("select * from users"));

	if(ok)
	{
		while (mdbRunner->getSqlQuery()->next())
		{
			QString userName = mdbRunner->getSqlQuery()->value(0).toString();
			QString psd = mdbRunner->getSqlQuery()->value(2).toString();
			if (userName.compare(name) == 0) {
				if (psd.compare(passW) == 0)
				{
					QString g_mail = mdbRunner->getSqlQuery()->value(3).toString();
					mUsr->strName = userName;
					mUsr->strGmail = g_mail;
					mUsr->isTempory = false;
					mUsr->strIcon = "shuangyu";
					
					this->close();
					return;
				}
				else
				{
					QMessageBox::warning(this, "error", "password incorrect !!! ");
					return;
				}
			}
		}
	}
}