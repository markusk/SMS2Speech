#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	mutex = new QMutex();
	// Interface to Arduino
	interface1 = new InterfaceAvr();
	circuit1 = new Circuit(interface1, mutex);
	speakThread = new SpeakThread();

	// show messages in the GUI log or in the console
	connect(this, SIGNAL( message(QString, bool, bool, bool) ), this, SLOT( appendLog(QString, bool, bool, bool) ));

	// speech
	connect(this, SIGNAL( speak(QString) ), speakThread, SLOT( speak(QString) ));

	if (speakThread->isRunning() == false)
	{
		emit message("Starting speak thread...");
		speakThread->start();
		emit message("Speak thread started.");
	}
}

MainWindow::~MainWindow()
{
	emit message("Closing serial port.");
	interface1->closeComPort();

	if (speakThread->isRunning())
	{
		speakThread->stop();
	}

	delete interface1;
	delete circuit1;
	delete speakThread;
	delete ui;
}

void MainWindow::on_pushButtonConnect_clicked()
{
	//-------------------------------------------------------
	// Open serial port from Arduino for communication
	//-------------------------------------------------------
	serialPortMicrocontroller = ui->lineEditUSBPort->text();

	emit message(QString("Opening serial port %1...").arg(serialPortMicrocontroller));

	if (interface1->openComPort(serialPortMicrocontroller) == false)
	{
		emit message("ERROR opeing serial port.");
	}
	else
	{
		//
		// USB Port Okay
		//
		emit message("Serial port opened.");

		// call Slot initArduino on Signal checkArduinoState
		connect(this, SIGNAL(checkArduinoState()), circuit1, SLOT(initArduino()));

		emit checkArduinoState();
	}
}

void MainWindow::on_pushButtonSpeak_clicked()
{
	// speak text from form
	emit speak(ui->lineEditInputSpeech->text());
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
