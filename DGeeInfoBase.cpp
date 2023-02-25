#include "DGeeInfoBase.h"

DGeeInfoBase::DGeeInfoBase(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setMaximumHeight(320);
	ui.mLabelCatalogLink->setOpenExternalLinks(true);
	ui.mLineEditSnippet->setReadOnly(true);
}

DGeeInfoBase::~DGeeInfoBase()
{
}

void DGeeInfoBase::setLabelContent(QString & Title, QString & dateAvalibility, QString & snippet, QString & catalogLink)
{
	ui.mLabelTitle->setText(Title);
	ui.mLabelDates->setText(dateAvalibility);
	ui.mLineEditSnippet->setText(snippet);
	QString content = QString("<a href=%1>Catalog").arg(catalogLink);
	ui.mLabelCatalogLink->setText(content);
}
