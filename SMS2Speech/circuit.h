/*************************************************************************
 *   Copyright (C) Markus Knapp                                          *
 *   www.direcs.de                                                       *
 *                                                                       *
 *   This file is part of direcs.                                        *
 *                                                                       *
 *   direcs is free software: you can redistribute it and/or modify it   *
 *   under the terms of the GNU General Public License as published      *
 *   by the Free Software Foundation, version 3 of the License.          *
 *                                                                       *
 *   direcs is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of      *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU General Public License for more details.                        *
 *                                                                       *
 *   You should have received a copy of the GNU General Public License   *
 *   along with direcs. If not, see <http://www.gnu.org/licenses/>.      *
 *                                                                       *
 *************************************************************************/

#ifndef CIRCUIT_H
#define CIRCUIT_H

//-------------------------------------------------------------------
#include <QThread>
#include <QMutex>
#include "interfaceAvr.h"
//-------------------------------------------------------------------

/**
\author Markus Knapp
\brief Delivers a initialisation for the robot's circuits

This class delivers a initialisation for the robot's circuits and checks, if the robot is ON or OFF.
*/
class Circuit : public QThread
{
	Q_OBJECT

	public:
		Circuit(InterfaceAvr *i, QMutex *m);
		~Circuit();

		/**
		Stops the thread.
		*/
		void stop();

		/**
		Starts the thread.
		*/
		virtual void run();

		/**
		@return The state of the robot (true when connected (and aswers), false when not).
		*/
		bool isConnected();


	public slots:
		/**
		Initialises the robot's circuits. It also checks, if the robot is ON or OFF.
		@return true, when the initialization was fine, so the robot is ON.
		*/
		bool initCircuit();

		/**
		 * @brief initArduino
		 * @return
		 */
		bool initArduino();

		/**
		Puts the robot's circuits to sleep. This also disables the robot's watchdog!
		@return true, when everything was fine
		*/
		bool sleep();

		/**
		This slots takes the robot (circuit) state, to know if the robot is ON or OFF.
		When the class knows this, unnecessary communication with the interface can be avoided.

		@param state can be ON or OFF
		 */
		void setRobotState(bool state);


	signals:
		/**
		This signal emits the robot (circuit) state to all connected slots, to tell them if the robot is ON or OFF
		@param state can be true or false
		*/
		void robotState(bool state);

		/**
		Emits a info or error message to a slot.
		This slot can be used to display a text on a splash screen, log file, to print it to a console...
		@param text is the message to be emitted
		*/
		void message(QString text, bool CR=true, bool sayIt=false, bool addTimestamp=true);

		/// Emits the SMS text to be spoken
		void speakSMS(QString text);



	private:
		QString className;	/// this will contain the name of _this_ class at runtime for debug messages

		mutable QMutex *mutex; // make this class thread-safe

		// Every thread sleeps some time, for having a bit more time fo the other threads!
		// Time in milliseconds
		static const unsigned long THREADSLEEPTIME = 1000; // Default: 100 ms?!?

		volatile bool stopped;

		InterfaceAvr *interface1;

		QString atmelCommand;
		QString atmelAnswer;
		QString expectedAtmelAnswer;

		QString commandInitCircuit;		///	*re#
		QString commandSleep;			///	*sl#

		bool circuitState; // stores the robot state within this class
		bool firstInitDone;

		static const bool ON  = true;
		static const bool OFF = false;

		/**
		example answer string without value from Atmel: *re#
		example answer string with value from Atmel:    *s7=42#
		*/
		static const char starter    = 42; /// This starts the serial string for the Atmel controller. 42 = *
		static const char terminator = 35; /// This terminates the serial string for the Atmel controller. 35 = #
		static const char divider    = 61; /// This divides the serial string for the Atmel controller. 61 = =
};

#endif
