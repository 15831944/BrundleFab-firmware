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

#ifndef AXIS_Y_H
#define AXIS_Y_H

#include <Wire.h>
#include <AFMotor.h>
#include <Encoder.h>

#include "pinout.h"
#include "Axis.h"

class Axis_Y : public Axis {
    private:
        static const int _overshoot = 10;
        static const int _adaMotor = Y_MOTOR;
        static const int _pinEncoderA = YENC_A;
        static const int _pinEncoderB = YENC_B;

        static const int _pwmMinimum = 98;
        static const int _pwmMaximum = 200;

        static const int _maxPos = 5250;
        static const int _minPos = 0;

        float _mm_to_position;

        AF_DCMotor _motor;
        Encoder _encoder;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE, HOMING_BACKOFF,
            MOVING, MOVING_OVERSHOOT,
        } mode;
        struct {
            unsigned long timeout;
            int32_t position;
            int32_t target;
        } _homing;
        struct {
            unsigned long timeout;
            int overshoot;
        } _moving;

    public:
        Axis_Y() : Axis(),
                 _motor(_adaMotor), _encoder(_pinEncoderA, _pinEncoderB)
        {
                _mm_to_position = (float)(_maxPos - _minPos)/(float)(230);
        }
        virtual void begin()
        {
            pinMode(_pinEncoderA, INPUT_PULLUP);
            pinMode(_pinEncoderB, INPUT_PULLUP);

            _encoder.write(0);
            Axis::begin();
        }

        virtual void home(int32_t position)
        {
            mode = HOMING;
            _homing.target = position;
            _motor.setSpeed(_pwmMaximum);
            _motor.run(BACKWARD);
        }

        virtual const float mm_to_position()
        {
            return _mm_to_position;
        }

        virtual const int32_t position_min()
        {
            return _minPos;
        }

        virtual const int32_t position_max()
        {
            return _maxPos;
        }

        virtual void motor_disable()
        {
            _motor.run(RELEASE);
            Axis::motor_disable();
        }

        virtual void motor_halt()
        {
            _motor.setSpeed(0);
            Axis::motor_halt();
        }

        virtual int32_t position_get(void)
        {
            return _encoder.read();
        }

        virtual bool update()
        {
            int32_t pos = position_get();
            int32_t tar = target_get();

            if (tar >= position_max())
                tar = position_max() - 1;

            if (tar < position_min())
                tar = position_min();

            switch (mode) {
            case IDLE:
                if (tar != pos)
                    mode = MOVING;
                break;
            case HOMING:
                _homing.position = _encoder.read();
                _homing.timeout = millis()+10;
                mode = HOMING_QUIESCE;
                break;
            case HOMING_QUIESCE:
                if (millis() >= _homing.timeout) {
                    if (_encoder.read() == _homing.position) {
                        _motor.setSpeed(0);
                        _motor.run(RELEASE);
                        _homing.timeout = millis() + 100;
                        mode = HOMING_BACKOFF;
                       break;
                    }
                    mode = HOMING;
                }
                break;
            case HOMING_BACKOFF:
                if (millis() >= _homing.timeout) {
                    _encoder.write(_minPos);
                    Axis::home(_homing.target);
                    mode = IDLE;
                }
             case MOVING:
            case MOVING_OVERSHOOT:
                int32_t distance = (pos > tar) ? (pos - tar) : (tar - pos);

                if (distance == 0) {
                    _motor.run(BRAKE);
                    _motor.setSpeed(0);
                    if (mode == MOVING) {
                        _moving.timeout = millis() + 1;
                        _moving.overshoot = _overshoot;
                        mode = MOVING_OVERSHOOT;
                    } else {
                        if (millis() < _moving.timeout)
                            break;
                        if (_moving.overshoot) {
                            _moving.overshoot--;
                            _moving.timeout = millis() + 1;
                        } else {
                            mode = IDLE;
                        }
                    }
                    break;
                }

                int speed;

                if (distance > 50)
                    speed = _pwmMaximum;
                else
                    speed = _pwmMinimum + distance;

                _motor.setSpeed(speed);
                _motor.run((pos < tar) ? FORWARD : BACKWARD);
                break;
                if (millis() > _moving.timeout) {
                    if (_moving.overshoot) {
                        _moving.overshoot--;
                        _motor.run(BRAKE);
                        _motor.setSpeed(0);
                        mode = MOVING;
                    }
                }
                break;
            }

             return (mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_Y_H */
/* vim: set shiftwidth=4 expandtab:  */
