#include "ISAShunt.h"

ISA::ISA()  // Define the constructor.
{
  
	 timestamp = millis(); 
	 debug=false;
	 debug2=false;
	 framecount=0;
	firstframe=true;
}


ISA::~ISA() //Define destructor
{
}

void ISA::handle521(CANMessage &frame)  //AMperes

{	
	framecount++;
	long current=0;
  current = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  milliamps=current;
  Amperes=current/1000.0f;    	
}

void ISA::handle522(CANMessage &frame)  //Voltage

{	
	framecount++;
	long volt=0;
  volt = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  Voltage=volt/1000.0f;
	Voltage1=Voltage-(Voltage2+Voltage3);
  if(framecount<150)
  {
    VoltageLO=Voltage;
    Voltage1LO=Voltage1;
  }
  if(Voltage<VoltageLO &&  framecount>150)VoltageLO=Voltage;
  if(Voltage>VoltageHI && framecount>150)VoltageHI=Voltage;
  if(Voltage1<Voltage1LO && framecount>150)Voltage1LO=Voltage1;
  if(Voltage1>Voltage1HI && framecount>150)Voltage1HI=Voltage1;
       	
}

void ISA::handle523(CANMessage &frame) //Voltage2

{	
	framecount++;
	long volt=0;
  volt = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  Voltage2=volt/1000.0f;
  if(Voltage2>3)Voltage2-=Voltage3;
  if(framecount<150)Voltage2LO=Voltage2;
  if(Voltage2<Voltage2LO  && framecount>150)Voltage2LO=Voltage2;
  if(Voltage2>Voltage2HI&& framecount>150)Voltage2HI=Voltage2; 
   	
}

void ISA::handle524(CANMessage &frame)  //Voltage3
{	
	framecount++;
	long volt=0;
  volt = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  Voltage3=volt/1000.0f;
  if(framecount<150)Voltage3LO=Voltage3;
  if(Voltage3<Voltage3LO && framecount>150 && Voltage3>10)Voltage3LO=Voltage3;
  if(Voltage3>Voltage3HI && framecount>150)Voltage3HI=Voltage3;
}

void ISA::handle525(CANMessage &frame)  //Temperature

{	
	framecount++;
	long temp=0;
  temp = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  Temperature=temp/10;
	   
}



void ISA::handle526(CANMessage &frame) //Kilowatts
{	
	framecount++;
	watt=0;
  watt = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
    
  KW=watt/1000.0f;
		    
}


void ISA::handle527(CANMessage &frame) //Ampere-Hours
{	
	framecount++;
	As=0;
  As = (frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]);
    
  AH+=(As-lastAs)/3600.0f;
  lastAs=As;
    
}

void ISA::handle528(CANMessage &frame)  //kiloWatt-hours
{	
	framecount++;
	
  wh = (long)((frame.data[5] << 24) | (frame.data[4] << 16) | (frame.data[3] << 8) | (frame.data[2]));
  KWH+=(wh-lastWh)/1000.0f;
	lastWh=wh;
   
}



void ISA::handleCANFrame(CANMessage &frame) {

   switch (frame.id)
     {
     case 0x511:
      
      break;
      
     case 0x521:    
      	handle521(frame);
      break;
      
     case 0x522:    
      	handle522(frame);
      break;
      
      case 0x523:    
      	handle523(frame);
      break;
      
      case 0x524:    
      	handle524(frame);
      break;
      
      case 0x525:    
      	handle525(frame);	
      break;
      
      case 0x526:    
      	handle526(frame);	
      break;
      
      case 0x527:    
      	handle527(frame);	
      break;
      
      case 0x528:
       	handle528(frame);   
        break;
     } 
}
