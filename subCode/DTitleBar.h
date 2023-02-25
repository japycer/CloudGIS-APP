#pragma once

#include <QWidget>
#include "ui_DTitleBar.h"

// 自定义标题栏
class DTitleBar : public QWidget
{
	Q_OBJECT

public:
	DTitleBar(QWidget *parent = Q_NULLPTR);
	~DTitleBar();
	void addMenuBar(QWidget *w);
	void setCustomTitle(const QString&);

private:
	Ui::DTitleBar ui;

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
	void onBtnClicked();

private:
	void updateMaximize();
	bool mIsPressed = false;
	QPoint mStartMovePos;
};
