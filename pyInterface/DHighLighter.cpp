#include"DHighLighter.h"


//构造函数，负责建立识别规则
DHighLighter::DHighLighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;
	// 1、关键字规则
	//keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setForeground(QColor(68, 181, 181));
	keywordFormat.setFontWeight(QFont::Bold);
	//关键字正则表达式
	const QString keywordPatterns[] = {
		QStringLiteral("\\bFalse\\b"), QStringLiteral("\\bNone\\b"), QStringLiteral("\\bTrue\\b"),
		QStringLiteral("\\band\\b"), QStringLiteral("\\bas\\b"), QStringLiteral("\\bassert\\b"),
		QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bcontinue\\b"),
		QStringLiteral("\\bdef\\b"), QStringLiteral("\\bdel\\b"), QStringLiteral("\\belif\\b"),
		QStringLiteral("\\belse\\b"), QStringLiteral("\\bexcept\\b"), QStringLiteral("\\bfinally\\b"),
		QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfrom\\b"), QStringLiteral("\\bglobal\\b"),
		QStringLiteral("\\bif\\b"), QStringLiteral("\\bimport\\b"), QStringLiteral("\\bin\\b"),
		QStringLiteral("\\bis\\b"), QStringLiteral("\\blambda\\b"), QStringLiteral("\\btry\\b"),
		QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bwith\\b"), QStringLiteral("\\byield\\b")
	};
	//迭代将关键字加入到规则中
	for (const QString &pattern : keywordPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}
	// 1、end

	//! 2、类名规则
	//classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(QColor(78, 206, 171));
	classFormat.setForeground(Qt::darkMagenta);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z]+[A-Za-z1-9]+[ ]+:\\b"));
	rule.format = classFormat;
	highlightingRules.append(rule);
	//! 2、end

	//! 字符串
	//quotationFormat.setForeground(Qt::darkGreen);
	quotationFormat.setForeground(QColor(214, 157, 133));
	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	rule.format = quotationFormat;
	highlightingRules.append(rule);
	rule.pattern = QRegularExpression(QStringLiteral("\'.*\'"));
	rule.format = quotationFormat;
	highlightingRules.append(rule);
	//! 字符串 end

	//! 5、函数名斜体
	functionFormat.setFontItalic(true);
	//functionFormat.setForeground(Qt::blue);
	functionFormat.setForeground(QColor(255, 191, 101));
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	rule.format = functionFormat;
	highlightingRules.append(rule);
	//! 5 end

	//! 6、多行注释
	commentStartExpression = QRegularExpression(QStringLiteral("\"\"\""));
	commentEndExpression = QRegularExpression(QStringLiteral("\"\"\""));
	//! 6 end

	//! 3、单行注释
	//singleLineCommentFormat.setForeground(Qt::red);
	singleLineCommentFormat.setForeground(QColor(39, 139, 39));
	rule.pattern = QRegularExpression(QStringLiteral("#[^\n]*"));
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::red);
	//! 3 end
}



//! 高亮文本块
void DHighLighter::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(commentStartExpression);

	while (startIndex >= 0) {
		QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex
				+ match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}
}