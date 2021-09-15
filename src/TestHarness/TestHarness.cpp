// TestHarness.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "observer3.h"

class Sensor;

class SensorEvents : public Events<Sensor>
{
public:
	virtual void Sensor_Changed (Sensor *sensor, int value, const char *name) {}
};

class Sensor : public Notifier<SensorEvents>
{
public:
	void ForceChange ()
	{
		vNotify (SensorEvents::Sensor_Changed, 1, "Foo");
	}
};

class Controller : public Listener<SensorEvents>
{
public:
	void AddSensor (Sensor *pSensor)
	{
		Subscribe(pSensor);
	}

	void RemoveSensor (Sensor *pSensor)
	{
		Unsubscribe(pSensor);
	}

	virtual void Notifier_Deleted (Sensor *sensor)
	{
	}

	virtual void Sensor_Changed (Sensor *sensor, int value, const char *name)
	{
		printf ("value %d, string %s", value, name);
	}
};

void TestFunc()
{
	Controller controller;
	Sensor sensor;
	controller.AddSensor (&sensor);
	sensor.ForceChange();
	controller.RemoveSensor(&sensor);
	sensor.ForceChange();
}

int _tmain(int argc, _TCHAR* argv[])
{
	TestFunc();

	return 0;
}

