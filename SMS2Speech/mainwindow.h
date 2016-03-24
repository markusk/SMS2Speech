#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QMutex>
#include <QTimer>

#include "circuit.h"
#include "interfaceAvr.h"
#include "speakThread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	/**
	Appends text to the main log in the main window.
	@param text is the text to be displayed.
	@param CR adds a carriage return (CR) to the text, if true (default). This parameter is optional.
	@param sayIt If true, the text is also spoken (default=false). This parameter is optional.
	@param addTimestamp If true, the a timestamp is added in front of the text. This parameter is optional.
	 */
	void appendLog(QString text, bool CR=true, bool sayIt=false, bool addTimestamp=true);

	/**
	  Show the Arduino status in the GUI
	  */
	void showArduinoStatus(QString status);

	/**
	  Show the battery status in the GUI
	  */
	void showBatteryPercent(int percent);

	/**
	  Show the network status in the GUI
	  */
	void showNetworkStatus(int status);

	/**
	  Show the number of SMS in the GUI
	  */
	void showNumberSMS(int amount);

protected slots:
	void on_pushButtonConnect_clicked();
	void on_pushButtonSpeak_clicked();
	void on_pushButtonQuit_clicked();
	void on_pushButtonReset_clicked();
	void on_actionQuit_triggered();

signals:
	/**
	Emits a string to the GUI log / console.
	@sa Gui::appendLog()
	@param text is the message to be emitted
	@param CR is true when a CR/LF should be added at the end of the line (default)
	@param sayIt is true when the text for the log also should be spoken (default=false)
	@param addTimestamp If true, the a timestamp is added in front of the text. This parameter is optional.
	*/
	void message(QString text, bool CR=true, bool sayIt=false, bool addTimestamp=true);

	/**
	Emits a speak signal. This signal is sent to the speakThread.
	*/
	void speak(QString text);

	/**
	 * @brief checkArduinoState
	 */
	void checkArduinoState();

protected:
	void closeEvent(QCloseEvent *event);

private:
	Ui::MainWindow *ui;
	Circuit *circuit1;
	InterfaceAvr *interface1;
	SpeakThread *speakThread;
	QDateTime now; /// this is for the timestamp in the logs in the gui
	QString serialPortMicrocontroller;
	mutable QMutex *mutex; // make the threads thread-safe (e.g. senorThread, servo...)
	bool arduinoConnnected;
};

#endif // MAINWINDOW_H
