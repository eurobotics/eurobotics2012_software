/*  
 *  Copyright Robotics Association of Coslada, Eurobotics Engineering (2010)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Revision : $Id$
 *
 *  Javier Bali�as Santos <javier@arc-robots.org>
 */

#ifndef _MAIN_H_
#define _MAIN_H_


#include <aversive.h>
#include <aversive/error.h>

#include <time.h>
#include <rdline.h>

#include <encoders_dspic.h>
#include <pwm_mc.h>
#include <pwm_servo.h>
#include <ax12.h>

#include <pid.h>
#include <quadramp.h>
#include <control_system_manager.h>
#include <blocking_detection_manager.h>
#include <robot_system.h>
#include <position_manager.h>
#include <trajectory_manager.h>

#include "../common/i2c_commands.h"


/* SOME USEFUL MACROS AND VALUES  *********************************************/

/* NUMBER OF ROBOTS TO TRACK */
#define TWO_OPPONENTS
/*#define ROBOT_2ND*/

/* uart 0 is for cmds and uart 1 is 
 * multiplexed between beacon and slavedspic */
#define CMDLINE_UART 	0
#define MUX_UART 			1

/* generic led toggle macro */
#define LED_TOGGLE(port, bit) do {		\
		if (port & _BV(bit))		\
			port &= ~_BV(bit);	\
		else				\
			port |= _BV(bit);	\
	} while(0)


/* brake motors */
#define BRAKE_ON()      do {} while(0)
#define BRAKE_OFF()     do {} while(0)

/* only 90 seconds, don't forget it :) */
#define MATCH_TIME 89


/* ROBOT PARAMETERS *************************************************/

/* distance between encoders weels,
 * decrease track to decrease angle */
#define EXT_TRACK_MM 	   212.55199035831 //211.0
#define VIRTUAL_TRACK_MM 	EXT_TRACK_MM

/* robot dimensions */
#define ROBOT_LENGTH    150.0
#define ROBOT_WIDTH 	   250.0

/* Some calculus:
 * it is a 500 imps -> 109200 because we see 1/4 period and encoder is before motor 54,6:1 gears.
 * and diameter: 48mm -> perimeter 150,796 mm 
 * 109200/150,8 -> 7240 imps / 10 mm */

/* increase it to go further */
#define MOTOR_GEAR_RATIO	54.6		
#define IMP_ENCODERS 		(500.0 * MOTOR_GEAR_RATIO)
#define WHEEL_DIAMETER_MM 	48.0


#define WHEEL_PERIM_MM 	(WHEEL_DIAMETER_MM * M_PI)
#define IMP_COEF 			1.0
#define DIST_IMP_MM 		(((IMP_ENCODERS*4) / WHEEL_PERIM_MM) * IMP_COEF)

/* ax12 */
#define AX12_ID_ARM		1
#define AX12_ID_COMB		2
#define AX12_ID_TEETH	3

/* encoders handlers */
#define RIGHT_ENCODER       ((void *)1) // decrements going fordwars
#define LEFT_ENCODER        ((void *)2) // increments with fordward

/* motor handles */
#define LEFT_MOTOR          ((void *)&gen.pwm_mc_left)
#define RIGHT_MOTOR         ((void *)&gen.pwm_mc_right)

/** ERROR NUMS */
#define E_USER_STRAT        194
#define E_USER_I2C_PROTO    195
#define E_USER_SENSOR       196
#define E_USER_CS           197
#define E_USER_BEACON       198
#define E_USER_AX12         199

/* EVENTS PRIORITIES */
#ifdef old_version
	#define EVENT_PRIORITY_LED 			  170
	#define EVENT_PRIORITY_TIME           160
	#define EVENT_PRIORITY_I2C_POLL       140
	#define EVENT_PRIORITY_SENSORS        120
	#define EVENT_PRIORITY_CS             100
	#define EVENT_PRIORITY_BEACON_POLL     80
	#define EVENT_PRIORITY_STRAT         	70
#else
	#define EVENT_PRIORITY_LED 			  170	#define EVENT_PRIORITY_TIME           160	#define EVENT_PRIORITY_I2C_POLL       140	#define EVENT_PRIORITY_SENSORS        120	#define EVENT_PRIORITY_CS             100	#define EVENT_PRIORITY_STRAT         	30	#define EVENT_PRIORITY_BEACON_POLL     20
#endif

/* EVENTS PERIODS */
#define EVENT_PERIOD_LED 			1000000L#define EVENT_PERIOD_STRAT			  25000L#define EVENT_PERIOD_BEACON_PULL	  10000L#define EVENT_PERIOD_SENSORS		  10000L#define EVENT_PERIOD_I2C_POLL		   8000L#define EVENT_PERIOD_CS 			   5000L

/* dynamic logs */
#define NB_LOGS 10

/* MAIN DATA STRUCTURES **************************************/

/* cs data */
struct cs_block {
	uint8_t on;
	struct cs cs;
  	struct pid_filter pid;
	struct quadramp_filter qr;
	struct blocking_detection bd;
};

/* genboard */
struct genboard
{
	/* command line interface */
	struct rdline rdl;
	char prompt[RDLINE_PROMPT_SIZE];

	/* motors */
	struct pwm_mc pwm_mc_left;
	struct pwm_mc pwm_mc_right;

	/* ax12 servos */
	AX12 ax12;

	/* i2c gpios */
#ifdef notuse
	uint8_t i2c_gpio0;
	uint8_t i2c_gpio1;
#endif

	/* log */
	uint8_t logs[NB_LOGS+1];
	uint8_t log_level;
	uint8_t debug;
};

/* maindspic */
struct mainboard 
{
	/* events flags */
	uint8_t flags;                
#define DO_ENCODERS  1
#define DO_CS        2
#define DO_RS        4
#define DO_POS       8
#define DO_BD       16
#define DO_TIMER    32
#define DO_POWER    64
#define DO_OPP     128

	/* control systems */
	struct cs_block angle;
	struct cs_block distance;

	/* x,y positionning and traj*/
	struct robot_system rs;
	struct robot_position pos;
   struct trajectory traj;

	/* robot status */
	uint8_t our_color;
	volatile int16_t speed_a;     /* current angle speed */
	volatile int16_t speed_d;     /* current dist speed */
	volatile int32_t pwm_l;       /* current left dac */
	volatile int32_t pwm_r;       /* current right dac */
};


/* state of beaconboard, synchronized through i2c */
struct beaconboard 
{
	/* status and color */
	uint8_t status;
	uint8_t color;
	
	/* opponent pos */
	int16_t opponent_x;
	int16_t opponent_y;
	int16_t opponent_a;
	int16_t opponent_d;

#ifdef TWO_OPPONENTS
	int16_t opponent2_x;
	int16_t opponent2_y;
	int16_t opponent2_a;
	int16_t opponent2_d;
#endif

#ifdef ROBOT_2ND
	int16_t robot_2nd_x;
	int16_t robot_2nd_y;
	int16_t robot_2nd_a;
	int16_t robot_2nd_d;
#endif

};

extern struct genboard gen;
extern struct mainboard mainboard;
extern struct beaconboard beaconboard;

///* start the bootloader */
//void bootloader(void);

#define WAIT_COND_OR_TIMEOUT(cond, timeout)                   \
({                                                            \
        microseconds __us = time_get_us2();                   \
        uint8_t __ret = 1;                                    \
        while(! (cond)) {                                     \
                if (time_get_us2() - __us > (timeout)*1000L) {\
                        __ret = 0;                            \
                        break;                                \
                }                                             \
        }                                                     \
	if (__ret)					      \
		DEBUG(E_USER_STRAT, "cond is true at line %d",\
		      __LINE__);			      \
	else						      \
		DEBUG(E_USER_STRAT, "timeout at line %d",     \
		      __LINE__);			      \
							      \
        __ret;                                                \
})
#endif
