#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButtonQuit_clicked()
{
	close();
}

void MainWindow::appendLog(QString text, bool CR, bool sayIt, bool addTimestamp)
{
	if (addTimestamp == true)
	{
		// get the current date and time for a timestimp in the log
		now = QDateTime::currentDateTime();

		// prepend timestamp
		text = QString("%1:%2:%3 %4").arg(now.toString("hh")).arg(now.toString("mm")).arg(now.toString("ss")).arg(text);
	}

	// insert the new text in the GUI
	ui->textEditOutput->insertHtml(text);

	if (CR == true) // default!
	{
		ui->textEditOutput->insertHtml("<br>");
	}

	// Ensures that the cursor is visible by scrolling the text edit if necessary.
	ui->textEditOutput->ensureCursorVisible();


	// say the message via Linux espeak
	if (sayIt == true)
	{
		emit speak(text);
	}
}
