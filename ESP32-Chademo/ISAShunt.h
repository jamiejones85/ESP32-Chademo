#ifndef ISAShunt_h
#define ISAShunt_h

#include <ACAN_ESP32.h>

class ISA
{

	public:
		ISA();
    ~ISA();
    void handleCANFrame(CANMessage &frame);

		float Amperes;   // Floating point with current in Amperes
		double AH;      //Floating point with accumulated ampere-hours
		double KW;
		double KWH;


		double Voltage;
		double Voltage1;
		double Voltage2;
		double Voltage3;
		double VoltageHI;
		double Voltage1HI;
		double Voltage2HI;
		double Voltage3HI;
		double VoltageLO;
		double Voltage1LO;
		double Voltage2LO;
		double Voltage3LO;

		double Temperature;

		bool debug;
		bool debug2;
		bool firstframe;
		int framecount;
		unsigned long timestamp;
		double milliamps;
		long watt;
		long As;
		long lastAs;
		long wh;
  	long lastWh;
		uint8_t canEnPin;
		int canSpeed;
		uint8_t page;

	private:
		unsigned long elapsedtime;
		double  ampseconds;
		int milliseconds ;
		int seconds;
		int minutes;
		int hours;
		void handle521(CANMessage &frame);
		void handle522(CANMessage &frame);
		void handle523(CANMessage &frame);
		void handle524(CANMessage &frame);
		void handle525(CANMessage &frame);
		void handle526(CANMessage &frame);
		void handle527(CANMessage &frame);
		void handle528(CANMessage &frame);
};

#endif
