#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// Interface to Arduino
	interface1 = new InterfaceAvr();
	speakThread = new SpeakThread();

	// show messages in the GUI log or in the console
	connect(this, SIGNAL( message(QString, bool, bool, bool) ), this, SLOT( appendLog(QString, bool, bool, bool) ));
}

MainWindow::~MainWindow()
{
	emit message("Closing serial port.");
	interface1->closeComPort();

	delete interface1;
	delete speakThread;
	delete ui;
}

void MainWindow::on_pushButtonConnect_clicked()
{
	//-------------------------------------------------------
	// Open serial port from Arduino for communication
	//-------------------------------------------------------
	serialPortMicrocontroller = "/dev/tty.usbserial-AH02ZBXJ";

	emit message(QString("Opening serial port %1...").arg(serialPortMicrocontroller));

	if (interface1->openComPort(serialPortMicrocontroller) == false)
	{
		emit message("ERROR opeing serial port.");
	}
	else
	{
		emit message("Serial port opened.");
	}
}

void MainWindow::on_pushButtonQuit_clicked()
{
	close();
}

void MainWindow::on_pushButtonSendCommand_clicked()
{
	emit message("Sending command...");
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
