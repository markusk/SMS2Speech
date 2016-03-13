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

#include "circuit.h"

Circuit::Circuit(InterfaceAvr *i, QMutex *m)
{
	// get the name of this class (this is for debugging messages)
	className = this->staticMetaObject.className();

	stopped = false;

	// copy the pointer from the original object
	interface1 = i;
	mutex = m;

	circuitState = ON; // We think positive
	firstInitDone = false;

	expectedAtmelAnswer = "error";

	// theAtmelcommands
	commandInitCircuit	= "cstart";
	commandSleep		= "sl";
}


Circuit::~Circuit()
{
}


void Circuit::stop()
{
	stopped = true;
}


void Circuit::run()
{
	//  start "threading"...
	while (!stopped)
	{
		// let the thread sleep some time for having more time for the other threads
		msleep(THREADSLEEPTIME);

		if (circuitState == ON)
		{
			// first, we do an init, if not done before
			if (firstInitDone == false)
			{
				emit message("Initialising Arduino...");
				if (initArduino() == false)
				{
					// fail
				}
				else
				{
					// success. Next steps below...
				}
			}
		} // circuit is on

	}
	stopped = false;
}


bool Circuit::initCircuit()
{
	QString answer = "error";


	if (circuitState) // maybe robot is already recognized as OFF by the interface class (e.g. path to serial port not found)!
	{
		// Lock the mutex. If another thread has locked the mutex then this call will block until that thread has unlocked it.
		mutex->lock();


		// sending RESET (INIT) command
		// check if Arduino sends the inital command that it is "ready"

		do
		{
			// is Arduino sending "*cstart#"?
			if (interface1->receiveStringBlocking(answer, className) == true)
			{
				// check string
				if (answer == "*cstart#")
				{
					// answer with same command to Arduino
					if (interface1->sendString(commandInitCircuit, className) == true)
					{
						// Unlock the mutex
						mutex->unlock();

						// ciruit init okay
						firstInitDone = true;
						circuitState = true;
						emit robotState(true);

						emit message(QString("Arduino communication okay (*%1#).").arg(commandInitCircuit));
						return true;
					}
				}
				else
				{
					emit message("ERROR: Arduino did not sent '*cstart#'.");

					firstInitDone = true;
					circuitState = false;
					emit robotState(false);

					return false;
				}
			}
		} while (1); // endlessly!

		// Unlock the mutex.
		mutex->unlock();

	}

	firstInitDone = true;
	circuitState = false;
	emit robotState(false);

	return false;
}


bool Circuit::initArduino()
{
//	emit message("Arduino check now!");

	// check again!
	firstInitDone = false;
	circuitState = true;

	return initCircuit();
}


bool Circuit::isConnected()
{
	// if not tried to init hardware, do this!
	if (firstInitDone == false)
	{
		initCircuit();
		firstInitDone = true;
	}

	return circuitState;
}


void Circuit::setRobotState(bool state)
{
	// store the state within this class
	circuitState = state;
//	qDebug("Circuit::setRobotState: state=%d", circuitState);
}


bool Circuit::sleep()
{
	QString answer = "error";


	if (circuitState) // maybe robot is already recognized as OFF by the interface class (e.g. path to serial port not found)!
	{
		// Lock the mutex. If another thread has locked the mutex then this call will block until that thread has unlocked it.
		mutex->lock();

		// sending SLEEP command
		if (interface1->sendString("sl", className) == true)
		{
			// check if the robot answers with "sl"
			if ( interface1->receiveString(answer, className) == true)
			{
				// everthing's fine
				if (answer == "*sl#")
				{
					// Unlock the mutex
					mutex->unlock();

					return true;
				}
			}
		}

		// Unlock the mutex.
		mutex->unlock();

	}

	return false;
}
