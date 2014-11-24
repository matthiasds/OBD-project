/*
 * PID.h
 *
 * Created: 16/11/2014 21:57:50
 *  Author: matthias
 */ 


#ifndef PID_H_
#define PID_H_

//Standars PIDs

//Mode  PID  Data ret......Description...............min_val...max_val..units...formula

#define CALC_ENGINE_LOAD            0X04  //01 	04 	1 	Calculated engine load value 	  0 	100 	 % 	A*100/255
#define ENGINE_COOLANT_TEMP         0x05  //01 	05 	1 	Engine coolant temperature 	-40 	215 	°C 	A-40
#define SHORT_TERM_FUEL_1           0x06  //01 	06 	1 	Short term fuel % trim—Bank 1 	-100 (Rich) 	99.22 (Lean) 	 % 	(A-128) * 100/128
#define LONG_TERM_FUEL_1            0x07  //01 	07 	1 	Long term fuel % trim—Bank 1 	-100 (Rich) 	99.22 (Lean) 	 % 	(A-128) * 100/128
#define SHORT_TERM_FUEL_2           0x08  //01 	08 	1 	Short term fuel % trim—Bank 2 	-100 (Rich) 	99.22 (Lean) 	 % 	(A-128) * 100/128
#define LONG_TERM_FUEL_2            0x09  //01 	09 	1 	Long term fuel % trim—Bank 2 	-100 (Rich) 	99.22 (Lean) 	 % 	(A-128) * 100/128
#define O2_B1_S1_VOLTAGE            0x14  //01 	14 	2 	Bank 1, Sensor 1:Oxygen sensor voltage,Short term fuel trim 	0-100(lean)	1.275 99.2(rich) 	Volts % 	A/200(B-128) * 100/128 (if B==0xFF, sensor is not used in trim calc)
#define FUEL_PRESSURE               0x0A  //01 	0A 	1 	Fuel pressure 	0 	765 	kPa (gauge) 	A*3
#define INTAKE_M_A_PRESSURE         0x0B  //01 	0B 	1 	Intake manifold absolute pressure 	0 	255 	kPa (absolute) 	A
#define ENGINE_RPM                  0x0C  //01 	0C 	2 	Engine RPM 	0 	16,383.75 	rpm 	((A*256)+B)/4
#define VEHICLE_SPEED               0x0D  //01 	0D 	1 	Vehicle speed 	0 	255 	km/h 	A
#define TIMING_ADVANCE              0x0E  //01 	0E 	1 	Timing advance 	-64 	63.5 	° relative to #1 cylinder 	A/2 - 64
#define INTAKE_AIR_TEMP             0x0F  //01 	0F 	1 	Intake air temperature 	-40 	215 	°C 	A-40
#define MAF_AIR_FLOW_RATE           0x10  //01 	10 	2 	MAF air flow rate 	0 	655.35 	g/s 	((A*256)+B) / 100
#define THROTTLE_POSITION           0x11  //01 	11 	1 	Throttle position 	0 	100 	 % 	A*100/255
#define RUNTIME_SINCE_START         0x1F  //01 	1F 	2 	Run time since engine start 	0 	65,535 	seconds 	(A*256)+B
#define DISTANCE_WITH_MALF          0x21  //01 	21 	2 	Distance traveled with malfunction indicator lamp (MIL) on 	0 	65,535 	km 	(A*256)+B
#define FUEL_RAIL_PRESSURE_RELATIVE 0x22  //01 	22 	2 	Fuel Rail Pressure (relative to manifold vacuum) 	0 	5177.265 	kPa 	(((A*256)+B) * 10) / 128
#define FUEL_RAIL_PRESSURE_DIESEL   0x23  //01 	23 	2 	Fuel Rail Pressure (diesel) 	0 	655350 	kPa (gauge) 	((A*256)+B) * 10
#define FUEL_LEVEL                  0x2F   //01 	2F 	1 	Fuel Level Input 	0 	100 	 % 	100*A/255
#define NUM_WARMUPS_SINCE_CODES     0x30   //01 	30 	1 	# of warm-ups since codes cleared 	0 	255 	N/A 	A
#define DISTANCE_SINCE_CODES        0x31   //01 	31 	2 	Distance traveled since codes cleared 	0 	65,535 	km 	(A*256)+B
#define EVAP_SYSTEM_VAPOR_PRESSURE  0x32   //01 	32 	2 	Evap. System Vapor Pressure 	-8,192 	8,192 	Pa 	((A*256)+B)/4 (A is signed)
#define BAROMETRIC_PRESSURE         0x33   //01 	33 	1 	Barometric pressure 	0 	255 	kPa (Absolute) 	A
#define CONTROL_MODULE_VOLTAGE      0x42   //01 	42 	2 	Control module voltage 	0 	65.535 	V 	((A*256)+B)/1000
#define ABSOLUTE_LOAD_VALUE         0x43   //01 	43 	2 	Absolute load value 	0 	25,700 	 % 	((A*256)+B)*100/255
#define COMMAND_EQUIV_RATIO         0x44   //01 	44 	2 	Command equivalence ratio 	0 	2 	N/A 	((A*256)+B)/32768
#define REL_THROTTLE_POSITION       0x45   //01 	45 	1 	Relative throttle position 	0 	100 	 % 	A*100/255
#define AMBIENT_AIR_TEMPERATURE     0x46   //01 	46 	1 	Ambient air temperature 	-40 	215 	°C 	A-40
#define ABS_THROTTLE_POSITION_B     0x47   //01 	47 	1 	Absolute throttle position B 	0 	100 	 % 	A*100/255
#define ABS_THROTTLE_POSITION_C     0x48   //01 	48 	1 	Absolute throttle position C 	0 	100 	 % 	A*100/255
#define ABS_THROTTLE_POSITION_D     0x49   //01 	49 	1 	Accelerator pedal position D 	0 	100 	 % 	A*100/255
#define ABS_THROTTLE_POSITION_E     0x4A   //01 	4A 	1 	Accelerator pedal position E 	0 	100 	 % 	A*100/255
#define ABS_THROTTLE_POSITION_F     0x4B   //01 	4B 	1 	Accelerator pedal position F 	0 	100 	 % 	A*100/255
#define COMMANDED_THROTTLE_ACTUATOR 0x4C   //01 	4C 	1 	Commanded throttle actuator 	0 	100 	 % 	A*100/255
#define TIME_RUN_WITH_MIL_ON        0x4D   //01 	4D 	2 	Time run with MIL on 	0 	65,535 	minutes 	(A*256)+B
#define TIME_SINCE_T_CODES_CLEAR    0x4E   //01 	4E 	2 	Time since trouble codes cleared 	0 	65,535 	minutes 	(A*256)+B
#define FUEL_TYPE                   0x51   //01 	51 	1 	Fuel Type 				From fuel type table see below
#define ETHANOL_FUEL                0x52   //01 	52 	1 	Ethanol fuel % 	0 	100 	 % 	A*100/255
#define ABS_EVAP_SYST_VAPOUR_PRESS  0x53   //01 	53 	2 	Absoulute Evap system Vapour Pressure 	0 	327675 	kpa 	1/200 per bit
#define ENGINE_FUEL_RATE			0x5E  //01 	5E 		2 		Engine fuel rate 0 - 3212.75 		L/h		 ((A*256)+B)*0.05




#endif /* PID_H_ */