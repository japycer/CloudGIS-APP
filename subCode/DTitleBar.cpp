#include "DTitleBar.h"
#include <QMouseEvent>

DTitleBar::DTitleBar(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.mBtnMin, &QPushButton::clicked, this, &DTitleBar::onBtnClicked);
	connect(ui.mBtnMax, &QPushButton::clicked, this, &DTitleBar::onBtnClicked);
	connect(ui.mBtnClose, &QPushButton::clicked, this, &DTitleBar::onBtnClicked);
}

DTitleBar::~DTitleBar()
{
}

void DTitleBar::updateMaximize()
{
	QWidget* pWindow = this->window();
	if (pWindow->isTopLevel())
	{
		bool bMaximize = pWindow->isMaximized();
		ui.mBtnMax->setToolTip(bMaximize ? tr("Restore") : tr("Maximize"));
		ui.mBtnMax->setIcon(bMaximize ? QIcon(":/images/Mine/zuidahua.png") : QIcon(":/images/Mine/zuidahua2.png"));
		ui.mBtnMax->setProperty("maximizeProperty", bMaximize ? "Restore" : "Maximize");
	}
}

void DTitleBar::onBtnClicked()
{
	QPushButton* pBtn = qobject_cast<QPushButton*>(sender());
	QWidget* pWnd = this->window();
	if (pWnd->isTopLevel())
	{
		if (pBtn == ui.mBtnMin)
		{
			pWnd->showMinimized();
		}

		else if (pBtn == ui.mBtnMax)
		{
			if (pWnd->isMaximized())
				pWnd->showNormal();
			else
				pWnd->showMaximized();

			updateMaximize();
		}
		else if (pBtn == ui.mBtnClose)
			this->parentWidget()->close();
	}
}

void DTitleBar::mousePressEvent(QMouseEvent* event)
{
	mIsPressed = true;
	mStartMovePos = event->globalPos();

	return QWidget::mousePressEvent(event);
}

void DTitleBar::mouseMoveEvent(QMouseEvent* event)
{
	if (mIsPressed)
	{
		QWidget* pWindow = this->window();
		if (pWindow->isMaximized())
		{
			pWindow->showNormal();
			updateMaximize();
		}
	}

	if (mIsPressed)
	{
		QPoint movePoint = event->globalPos() - mStartMovePos;
		if (parentWidget())
		{
			QPoint widgetPos = this->parentWidget()->pos();
			mStartMovePos = event->globalPos();
			parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
		}
	}
	return QWidget::mouseMoveEvent(event);
}

void DTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
	mIsPressed = false;
	return QWidget::mouseReleaseEvent(event);
}

void DTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	// 双击时获取焦点
	this->setFocus();
	QWidget* pWindow = this->window();
	if (pWindow->isMaximized())
		pWindow->showNormal();
	else
		pWindow->showMaximized();

	updateMaximize();
}

void DTitleBar::setCustomTitle(const QString& title)
{
	ui.mLabelTitle->setText(title);
}

bool DTitleBar::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type())
	{
	case QEvent::WindowTitleChange:
	{
		QWidget *pWidget = qobject_cast<QWidget *>(obj);
		if (pWidget)
		{
			ui.mLabelTitle->setText(pWidget->windowTitle());
			return true;
		}
	}
	case QEvent::WindowIconChange:
	{
		QWidget *pWidget = qobject_cast<QWidget *>(obj);
		if (pWidget)
		{
			ui.mLogo->setIcon(pWidget->windowIcon());
			return true;
		}
	}
	case QEvent::WindowStateChange:
	case QEvent::Resize:
		updateMaximize();
		return true;
	}
	return QWidget::eventFilter(obj, event);
}

void DTitleBar::addMenuBar(QWidget *w)
{
	this->layout()->addWidget(w);
}