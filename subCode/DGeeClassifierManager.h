#pragma once

#include <QWidget>
#include "ui_DGeeClassifierManager.h"

class DEELayerShowOptionWin;

class DGeeClassifierManager : public QWidget
{
	Q_OBJECT

public:
	DGeeClassifierManager(QWidget *parent = Q_NULLPTR);
	~DGeeClassifierManager();

	enum Classifier
	{
		amnhMaxent,
		decisionTree,
		decisionTreeEnsemble,
		libsvm,
		minimumDistance,
		smileCart,
		smileGradientTreeBoost,
		smileNaiveBayes,
		smileRandomForest,
		other
	};

private:
	Ui::DGeeClassifierManager ui;
	DEELayerShowOptionWin* mSetShowParamsWin = nullptr;

private:
	void initGui();
	void bindSignalsSlots();
	void onQuitBtnClicked();
	void onBtnAddCliecked();
	void onBtnTainCliecked();
	void onBtnClassifyBtnCliecked();
	void onBtnTrainAndClassifyCliecked();
	void onBtnShowParamsClicked();
	void onBtnExplainClicked();
	void onBtnConfusionTableClicked();
	void updateBandNames();
	QString getInputLayerName();
	bool checkInputsValidity();  // 检测输入变量的合法性
};
