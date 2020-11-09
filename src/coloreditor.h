#ifndef COLOREDITOR_H
#define COLOREDITOR_H

#include <QWidget>
#include <QMap>
#include <QScrollArea>

#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QTextEdit>


class ColorEditor : public QWidget
{
	Q_OBJECT
public:
	explicit ColorEditor(QApplication *app, QWidget *parent = nullptr);

private:
	void buildMenuForMap();
	bool handleRgbaColor(const QString &key, const QString &line, int index,
	                     QPushButton *btn);
	bool handleHexColor(const QString &key, const QString &line, int index,
	                     QPushButton *btn);

	void rebuildAndApplyStylesheet();

private	Q_SLOTS:
	void changeColor();

private:
	QMap<QString, QStringList> m_entityStylesheetMap;
	QScrollArea *m_scrollArea{nullptr};
	QTextEdit *m_textEdit{nullptr};
	QApplication *m_app;
};

#endif // COLOREDITOR_H
