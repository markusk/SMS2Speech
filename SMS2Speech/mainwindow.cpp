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
	gsmThread = new GSMThread(interface1, mutex);

	// show messages in the GUI log or in the console
	connect(this, SIGNAL( message(QString, bool, bool, bool) ), this, SLOT( appendLog(QString, bool, bool, bool) ));
	connect(interface1, SIGNAL( message(QString,bool,bool,bool)), this, SLOT(appendLog(QString,bool,bool,bool)));
	connect(circuit1, SIGNAL( message(QString, bool, bool, bool)), this, SLOT(appendLog(QString, bool, bool, bool)));
	connect(gsmThread, SIGNAL(message(QString, bool, bool, bool)), this, SLOT(appendLog(QString, bool, bool, bool)));

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

	if (gsmThread->isRunning())
	{
		gsmThread->stop();
	}

	if (speakThread->isRunning())
	{
		speakThread->stop();
	}

	delete interface1;
	delete circuit1;
	delete gsmThread;
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
		// connect(this, SIGNAL(checkArduinoState()), circuit1, SLOT(initArduino()));

		emit message("Establishing connection to Arduino...");
//		circuit1->initArduino();

		if (circuit1->isRunning() == false)
		{
			emit message("Starting cicuit thread...");
			circuit1->start();
			emit message("Cicuit thread started.");
		}
/*
		// start the GSM thread
		if (gsmThread->isRunning() == false)
		{
			// whenever there is a material error, react!
			//connect(gsmThread, SIGNAL( systemerror(int) ), this, SLOT( systemerrorcatcher(int) ) );

			// show GSM status in the GUI
			//connect(gsmThread, SIGNAL(GSMStatus(unsigned char)), gui, SLOT(setLEDGSM(unsigned char)));

			//----------------------
			// init the GSM module
			//----------------------
			emit message("Initialising GSM module...", false);

			if (gsmThread->init() == true)
			{
				emit message("GSM module initialised.");

				// show SMS available in the GUI
				//connect(gsmThread, SIGNAL(SMSavailable(int, QString)), gui, SLOT(showSMSavailable(int)));

				// "new SMS" handling
				//connect(gsmThread, SIGNAL(SMSavailable(int, QString)), this, SLOT(SMSTracking(int, QString)));

				emit message("Starting GSM thread...");

				// NOW start the run() method inside!
				gsmThread->start();

				emit message("GSM thread started.");
			}
		}
*/
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
