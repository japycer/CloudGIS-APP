#include "DAlgorithm.h"
#include "qmath.h"

// 迭代两次的暗像元大气校正算法
QList<QList<float>> DAlgorithm::darkPixelCalculate(QList<float> datas, int loop)
{
	int length = datas.count();
	float degreeF = 57.782833f;
	float radianF = qDegreesToRadians(degreeF);
	float cos0 = qCos(radianF);
	float p0 = (cos0 * cos0 + 1) * 3.0 / 4;

	QList<float> L5;
	for (int i = 0; i < length; i++)
		L5 << datas.at(i) * 4 * cos0 / p0;

	QList<float> L6;
	for (int i = 0; i < length; i++)
		L6 << 1 - L5.at(i);

	QList<float> L7;
	for (int i = 0; i < length; i++)
		L7 << - qLn(L6.at(i));

	QList<float> L8;
	for (int i = 0; i < length; i++)
		L8 << qExp(-L7.at(i) / cos0);

	
	QList<float> L9;
	for (int i = 0; i < length; i++)
		L9 << (L8.at(i)*cos0 + (1-L6.at(i))/2 )*L6.at(i)/ cos0;

	// a1
	QList<float> L10;
	for (int i = 0; i < length; i++)
		L10 << 1 / L9.at(i);

	// b1
	QList<float> L11;
	for (int i = 0; i < length; i++)
		L11 << -datas.at(i)/L9.at(i);

	QList<float> L13;
	for (int i = 0; i < length; i++)
		L13 << (1-L6.at(i))/2;

	QList<float> L14;
	L14 << 4 << 8 << 4 << 52 << 35 << 25;

	QList<float> L15;
	for (int i = 0; i < length; i++)
		L15 << L13.at(i)*L6.at(i)*L14.at(i)/100;

	QList<float> L16;
	for (int i = 0; i < length; i++)
		L16 << datas.at(i) - L15.at(i);

	QList<float> L17;
	for (int i = 0; i < length; i++)
		L17 << L16.at(i) * 4 * cos0 / p0;

	QList<float> L18;
	for (int i = 0; i < length; i++)
		L18 << 1 - L17.at(i);

	QList<float> L19;
	for (int i = 0; i < length; i++)
		L19 << -qLn(L18.at(i));

	QList<float> L20;
	for (int i = 0; i < length; i++)
		L20 << qExp(-L19.at(i) / cos0);

	QList<float> L21;
	for (int i = 0; i < length; i++)
		L21 << (L20.at(i)*cos0 + (1 - L18.at(i)) / 2)*L18.at(i) / cos0;

	// a2
	QList<float> L22;
	for (int i = 0; i < length; i++)
		L22 << 1 / L21.at(i);

	// b2
	QList<float> L23;
	for (int i = 0; i < length; i++)
		L23 << -L16.at(i) / L21.at(i);

	QList<float> L25;
	for (int i = 0; i < length; i++)
		L25 << (1 - L18.at(i)) / 2;

	QList<float> L26;
	L26 << 2.48 << 4.12 << 2.86 << 25.38 << 10.46 << 3.5;

	QList<float> L27;
	for (int i = 0; i < length; i++)
		L27 << L25.at(i)*L18.at(i)*L26.at(i) / 100;

	QList<float> L28;
	for (int i = 0; i < length; i++)
		L28 << L16.at(i) - L27.at(i);

	QList<float> L29;
	for (int i = 0; i < length; i++)
		L29 << L28.at(i) * 4 * cos0 / p0;

	QList<float> L30;
	for (int i = 0; i < length; i++)
		L30 << 1 - L29.at(i);

	QList<float> L31;
	for (int i = 0; i < length; i++)
		L31 << -qLn(L30.at(i));

	QList<float> L32;
	for (int i = 0; i < length; i++)
		L32 << qExp(-L31.at(i) / cos0);

	QList<float> L33;
	for (int i = 0; i < length; i++)
		L33 << (L32.at(i)*cos0 + (1 - L30.at(i)) / 2)*L30.at(i) / cos0;

	// a3
	QList<float> L34;
	for (int i = 0; i < length; i++)
		L34 << 1 / L33.at(i);

	// b3
	QList<float> L35;
	for (int i = 0; i < length; i++)
		L35 << -L28.at(i) / L33.at(i);

	QList<QList<float>> res;
	res << L10 << L11 << L22 << L23 << L34 << L35;
	return res;
}