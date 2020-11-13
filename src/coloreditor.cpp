#include "coloreditor.h"

#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QColorDialog>

#include <QScrollArea>
#include <QLabel>
#include <QDebug>
#include <QApplication>
#include <QTabWidget>

#include "application_restarter.h"

#include <iostream>

ColorEditor::ColorEditor(QApplication *app, QWidget *parent)
	: QWidget(parent)
	, m_app(app)
{
	QFile file(":/stylesheets/stylesheets/global.qss");
	file.open(QFile::ReadOnly);

//	QString stylesheet = QString::fromLatin1(file.readAll());

	auto layout = new QVBoxLayout();
	setLayout(layout);

	QPushButton *restart = new QPushButton("Restart Scopy!");
	connect(restart, &QPushButton::clicked, [=](){
		adiscope::ApplicationRestarter::triggerRestart();
	});

	layout->addWidget(restart);

	m_textEdit = new QTextEdit();
//	layout->addWidget(m_textEdit);

	QTabWidget *tabs = new QTabWidget();

	m_scrollArea = new QScrollArea();
//	layout->insertWidget(0, m_scrollArea);

	tabs->addTab(m_scrollArea, "Item List");
	tabs->addTab(m_textEdit, "Advanced Editor");

	tabs->setMovable(true);

	layout->insertWidget(0, tabs);

	auto scrollAreaWidget = new QWidget();
	auto scrollAreaLayout = new QVBoxLayout();
	scrollAreaWidget->setLayout(scrollAreaLayout);

	m_scrollArea->setWidget(scrollAreaWidget);
	m_scrollArea->setWidgetResizable(true);

	QString toParse = QString::fromLatin1(file.readAll());
	QStringList print = toParse.split("{").join("").split("}");

	QString textEditText = "";

	for (auto token : print) {
		QStringList ttoken = token.split("\n", Qt::SkipEmptyParts);
		if (ttoken.size()) {
			for (auto t : ttoken) {
				if (t.startsWith("/*")) continue; //ignore comments
				textEditText += t;
				textEditText += '\n';
				break;
			}

			int i = 0;
			QString mapKey = "";
			for (; i < ttoken.size(); ++i) {
				if (ttoken[i].startsWith("/*")) continue; // ignore comments
				mapKey = ttoken[i];
				break;
			}

			if (mapKey == "") {
				break;
			}

			m_entityStylesheetMap[mapKey] = QStringList();

			i++; // advance so we skip the key

			for (; i < ttoken.size(); ++i) {
				m_entityStylesheetMap[mapKey].push_back(ttoken[i]);
			}
		}
	}

//	textEdit->setText(textEditText);

//	for (auto it = m_entityStylesheetMap.begin(); it != m_entityStylesheetMap.end(); ++it) {
//		scrollAreaLayout->addWidget(new QPushButton(it.key()));
//	}

//	m_textEdit->setText(file.readAll());

	buildMenuForMap();
}

void ColorEditor::buildMenuForMap()
{
	for (auto it = m_entityStylesheetMap.begin(); it != m_entityStylesheetMap.end(); ++it) {
//		m_scrollArea->widget()->layout()->addWidget(new QPushButton(it.key()));
		auto layout = m_scrollArea->widget()->layout();
		QWidget *colorEditor = new QWidget();
		QVBoxLayout *vBox = new QVBoxLayout();
		colorEditor->setLayout(vBox);
		vBox->addWidget(new QLabel(it.key()));

		std::cout << it.key().toStdString() << std::endl;

		bool toAdd = false;

		for (auto line : it.value()) {
			std::vector<QPushButton *> controls;
			int index = 0;
			while ((index = line.indexOf("rgba(", index)) != -1) {
				std::cout << "Found rgba( on line: " << line.toStdString() << " at index: " << index << std::endl;
				auto btn = new QPushButton(line.split(":")[0].replace(" ", ""));
				btn->setProperty("key", QVariant(it.key()));
				btn->setProperty("line", QVariant(line));
				btn->setProperty("index", QVariant(index));

				controls.push_back(btn);
				++index;
			}

			index = 0;
			while ((index = line.indexOf("#", index)) != -1) {
				std::cout << "Found # on line: " << line.toStdString() << " at index: " << index << std::endl;
				auto btn = new QPushButton(line.split(":")[0].replace(" ", ""));
				btn->setProperty("key", QVariant(it.key()));
				btn->setProperty("line", QVariant(line));
				btn->setProperty("index", QVariant(index));

				controls.push_back(btn);
				++index;
			}

			QHBoxLayout *btnLayout = nullptr;
			if (controls.size()) {
				btnLayout = new QHBoxLayout();
			}
			for (auto b : controls) {
				connect(b, &QPushButton::clicked,
					this, &ColorEditor::changeColor);
				btnLayout->addWidget(b);
				toAdd = true;
			}
			if (btnLayout) {
				btnLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
				vBox->addLayout(btnLayout);
			}
		}



		if (toAdd) {
			layout->addWidget(colorEditor);
		}
	}
}

bool ColorEditor::handleRgbaColor(const QString &key, const QString &line, int index, QPushButton *btn)
{
	int endIndex = -1;
	for (int i = index; i < line.size(); ++i) {
		if (line[i] == ")") {
			endIndex = i;
		}
	}

	std::cout << line[index].toLatin1() << " " << line[endIndex].toLatin1() << std::endl;

	QString colorString = "";
	for (int i = index; i <= endIndex; ++i) {
		colorString += line.at(i);
	}

	std::cout << colorString.toStdString() << std::endl;

	QStringList values = colorString.split("(")[1].split(")")[0].replace(" ", "").split(",");
	int r = 0, g = 0, b = 0, a = 0;

	r = values[0].toInt();
	g = values[1].toInt();
	b = values[2].toInt();
	if (values.size() == 4) {
		a = values[3].toInt();
	}

	std::cout << r << " " << g << " " << b << " " << a << std::endl;
	QColor oldColor = QColor();
	oldColor.setRed(r);
	oldColor.setGreen(g);
	oldColor.setBlue(b);
	oldColor.setAlpha(a);

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {
		return false;
	}

	QString newColorString = "rgba(" + QString::number(newColor.red())
			+ ", " +  QString::number(newColor.green())
			+ ", " +  QString::number(newColor.blue())
			+ ", " +  QString::number(newColor.alpha())
			+ ")";

	std::cout << newColorString.toStdString() << std::endl;

	QStringList value = m_entityStylesheetMap[key];
	for (int i = 0; i < value.size(); ++i) {
		if (value[i] == line) {
			value[i].replace(index, endIndex - index + 1, newColorString);
			btn->setProperty("line", QVariant(value[i]));
		}
	}
	m_entityStylesheetMap[key] = value;

	return true;
}

bool ColorEditor::handleHexColor(const QString &key, const QString &line, int index, QPushButton *btn)
{
	int endIndex = -1;
	for (int i = index; i < line.size(); ++i) {
		if (line[i] == " " || line[i] == ";") {
			endIndex = i;
		}
	}

	QString colorString = "";
	for (int i = index; i < endIndex; ++i) {
		colorString += line.at(i);
	}

	QColor oldColor(colorString);

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {
		return false;
	}

	QString newColorString = newColor.name();

	QStringList value = m_entityStylesheetMap[key];
	for (int i = 0; i < value.size(); ++i) {
		if (value[i] == line) {
			value[i].replace(index, endIndex - index, newColorString);
			btn->setProperty("line", QVariant(value[i]));
		}
	}
	m_entityStylesheetMap[key] = value;

	return true;
}

void ColorEditor::rebuildAndApplyStylesheet()
{
	QString stylesheet = "";
	for (auto it = m_entityStylesheetMap.begin(); it != m_entityStylesheetMap.end(); ++it) {
		stylesheet += it.key() + " {\n";

		for (auto line : it.value()) {
			stylesheet += line + "\n";
		}

		stylesheet += "}\n\n";
	}

	m_textEdit->setText(stylesheet);
	m_app->setStyleSheet(stylesheet);
}

void ColorEditor::createNewFile()
{

}

void ColorEditor::loadFile()
{

}

QString ColorEditor::getStyleSheet() const
{
	QFile file(":/stylesheets/stylesheets/global.qss");
	file.open(QFile::ReadOnly);

	return QString::fromLatin1(file.readAll());
}

void ColorEditor::changeColor()
{
	QPushButton *btn = qobject_cast<QPushButton *>(QObject::sender());

	QString key = btn->property("key").toString();
	QString line = btn->property("line").toString();
	int index = btn->property("index").toInt();

	bool changed = false;
	if (line[index] == "r") {
		changed = handleRgbaColor(key, line, index, btn);
	} else if (line[index] == "#") {
		changed = handleHexColor(key, line, index, btn);
	}

	if (changed) {
		rebuildAndApplyStylesheet();
	}
}
