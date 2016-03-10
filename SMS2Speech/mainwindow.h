#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>

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

protected slots:
	void on_pushButtonQuit_clicked();
	void on_pushButtonSendCommand_clicked();

signals:
	/**
	Emits a speak signal. This signal is sent to the speakThread.
	*/
	void speak(QString text);

private:
	Ui::MainWindow *ui;
	QDateTime now; /// this is for the timestamp in the logs in the gui
};

#endif // MAINWINDOW_H
