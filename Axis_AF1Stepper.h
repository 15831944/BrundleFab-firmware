/*
 * Copyright (C) 2014, Jason S. McMullan
 * All right reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef AXIS_AF1STEPPER_H
#define AXIS_AF1STEPPER_H

#include <Wire.h>
#include <AFMotor.h>

#include "Axis_Stepper.h"

class Axis_AF1Stepper : public Axis_Stepper {
    private:
        AF_Stepper _adaMotor; /* 1 = M1/M2, 2 = M3/M4 */
        AF_Stepper *_motor;

    public:
        Axis_AF1Stepper(int af_motor, int pinStopMin, int pinStopMax,
                        float maxPosMM,
                        unsigned int stepsPerRotation, float mmPerRotation)
            : Axis_Stepper(pinStopMin, pinStopMax,
                           maxPosMM,
                           MICROSTEPS, stepsPerRotation, mmPerRotation),
             _adaMotor(stepsPerRotation, af_motor)
        {
            _motor = &_adaMotor;
        }

        virtual void begin()
        {
            _motor->setSpeed(100);       /* 100 RPM */
            Axis_Stepper::begin();
        }

        virtual void motor_enable(bool enabled = true)
        {
            if (!enabled)
                _motor->release();
            Axis_Stepper::motor_enable(enabled);
        }

        virtual int step(int steps)
        {
            uint8_t dir;
            int neg = 1;

            if (!steps)
                return 0;

            if (steps < 0) {
                dir = BACKWARD;
                steps = -steps;
                neg = -1;
            } else {
                dir = FORWARD;
            }

            if (steps > MICROSTEPS * 2) {
                int dstep = steps / (MICROSTEPS * 2);
                _motor->step(dstep, dir, DOUBLE);
                return neg * dstep * (MICROSTEPS * 2);
            }

            if (steps > MICROSTEPS) {
                int sstep = steps / MICROSTEPS;
                _motor->step(sstep, dir, SINGLE);
                return neg * sstep * (MICROSTEPS * 2);
            }

            _motor->step(steps, dir, MICROSTEP);
            return neg * steps;
        }
};


#endif /* AXIS_AF1STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */
