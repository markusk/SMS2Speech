#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QApplication::setWindowIcon(QIcon("SMS2Speech.icns"));

	ui->setupUi(this);

	mutex = new QMutex();
	// Interface to Arduino
	interface1 = new InterfaceAvr();
	circuit1 = new Circuit(interface1, mutex);
	speakThread = new SpeakThread();

	// show messages in the GUI log or in the console
	connect(this, SIGNAL( message(QString, bool, bool, bool) ), this, SLOT( appendLog(QString, bool, bool, bool) ));
	connect(interface1, SIGNAL( message(QString,bool,bool,bool)), this, SLOT(appendLog(QString,bool,bool,bool)));
	connect(circuit1, SIGNAL( message(QString, bool, bool, bool)), this, SLOT(appendLog(QString, bool, bool, bool)));

	// show status of hardware in GUI
	connect(circuit1, SIGNAL(batteryStatus(int)), this, SLOT(showBatteryPercent(int)));
	connect(circuit1, SIGNAL(networkStatus(int)), this, SLOT(showNetworkStatus(int)));
	connect(circuit1, SIGNAL(arduinoStatus(QString)), this, SLOT(showArduinoStatus(QString)));
	connect(circuit1, SIGNAL(numberSMS(int)), this, SLOT(showNumberSMS(int)));

	// speech
	connect(this, SIGNAL( speak(QString) ), speakThread, SLOT( speak(QString) ));

	if (speakThread->isRunning() == false)
	{
		emit message("Starting speak thread...");
		speakThread->start();
		emit message("Speak thread started.");
	}

	// forward the speak signal and speak each SMS
	connect(circuit1, SIGNAL(speak(QString)), this, SIGNAL(speak(QString)));

	// let circuit class know if we want to delete every SMS if it was spoken
	connect(this, SIGNAL(deleteSMS(bool)), circuit1, SLOT(setSMSdeletion(bool)));

	// USB port (serial connection) not opened
	arduinoConnnected = false;
}


MainWindow::~MainWindow()
{
	delete interface1;
	delete circuit1;
	delete speakThread;
	delete ui;
}

void MainWindow::on_pushButtonConnect_clicked()
{
	if (arduinoConnnected == false)
	{
		//-------------------------------------------------------
		// Open serial port from Arduino for communication
		//-------------------------------------------------------
		serialPortMicrocontroller = ui->lineEditUSBPort->text();

		emit message(QString("Opening serial port %1...").arg(serialPortMicrocontroller));

		if (interface1->openComPort(serialPortMicrocontroller) == false)
		{
			arduinoConnnected = false;
			emit message("ERROR opeing serial port.");
		}
		else
		{
			//
			// USB Port Okay
			//
			arduinoConnnected = true;

			// change push button text
			ui->pushButtonConnect->setText("Disconnect");

			// disable checkbox for SMS deletion
			ui->checkBoxDeleteSMS->setEnabled(false);

			// send checkbox state to circuit class
			emit deleteSMS(ui->checkBoxDeleteSMS->isChecked());

			emit message("Serial port opened.");
			emit message("Establishing connection to Arduino...");

			if (circuit1->isRunning() == false)
			{
				emit message("Starting cicuit thread...");
				circuit1->start();
				emit message("Cicuit thread started.");
			}
		}
	}
	else
	{
		//------------------------------
		// CLose serial port to Arduino
		//------------------------------
		arduinoConnnected = false;

		if (circuit1->isRunning())
		{
			emit message("Stopping cicuit thread...");
			circuit1->stop();
			emit message("Cicuit thread stopped.");
		}

		emit message("Closing serial port.");
		interface1->closeComPort();

		// change push button text
		ui->pushButtonConnect->setText("Connect");

		// ensable checkbox for SMS deletion
		ui->checkBoxDeleteSMS->setEnabled(true);

		emit message("Serial port closed.");
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

void MainWindow::on_actionQuit_triggered()
{
	close();
}


void MainWindow::on_checkBoxDeleteSMS_stateChanged(int state)
{
	if (state == Qt::Checked)
	{
		emit deleteSMS(true);
	}
	else
	{
		emit deleteSMS(false);
	}
}


void MainWindow::closeEvent(QCloseEvent *)
{
	if (circuit1->isRunning())
	{
		emit message("Stopping circuit thread.");
		circuit1->stop();
	}

	if (speakThread->isRunning())
	{
		emit message("Stopping speech thread.");
		speakThread->stop();
	}

	emit message("Closing serial port.");
	interface1->closeComPort();
}

void MainWindow::on_pushButtonReset_clicked()
{
	appendLog("RESETTING...!");
	circuit1->reset();
	appendLog("Done.");
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

void MainWindow::showArduinoStatus(QString status)
{
	ui->labelArduinoStatus->setText(status);
}

void MainWindow::showBatteryPercent(int percent)
{
	ui->labelBatteryPercent->setText(QString("%1%").arg(percent));
	ui->progressBarBattery->setValue(percent);
}

void MainWindow::showNetworkStatus(int status)
{
	if (status == 0)
	{
		ui->labelNetworkStatus->setText("Not registered [0]");
		return;
	}

	if (status == 1)
	{
		ui->labelNetworkStatus->setText("Registered (home)");
		return;
	}

	if (status == 2)
	{
		ui->labelNetworkStatus->setText("Not registered (searching)");
		return;
	}

	if (status == 3)
	{
		ui->labelNetworkStatus->setText("Denied");
		return;
	}

	if (status == 4)
	{
		ui->labelNetworkStatus->setText("Unknown");
		return;
	}

	if (status == 5)
	{
		ui->labelNetworkStatus->setText("Registered roaming");
		return;
	}
}

void MainWindow::showNumberSMS(int amount)
{
	ui->labelNumberSMS->setText(QString("%1").arg(amount));
}
