/*
 * Copyright (C) 2015, Jason S. McMullan
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

#ifndef TOOLFUSER_H
#define TOOLFUSER_H

#include "Tool.h"


// r0: 250000
// t0: 25
// r1: 100000000
// r2: 1800
// beta: 4066
// max adc: 1023
#define NUMTEMPS 20
static struct {
        short adc;
        short celsius;
} const temptable[NUMTEMPS] = {
   {1, -1},
   {54, 78},
   {107, 101},
   {160, 118},
   {213, 132},
   {266, 144},
   {319, 155},
   {372, 165},
   {425, 176},
   {478, 187},
   {531, 198},
   {584, 209},
   {637, 222},
   {690, 236},
   {743, 253},
   {796, 272},
   {849, 298},
   {902, 334},
   {955, 397},
   {1008, 631}
};

class ToolFuser : public Tool {
    private:
        static const int DEBUG = 0;
        int _enable_pin;
        int _temp_pin;
        short _limit_min;
        short _limit_max;
        short _precision;
        short _temp;
        bool _ready;

    public:
        ToolFuser(int enable_pin, int temp_pin)
        {
            _enable_pin = enable_pin;
            _temp_pin = temp_pin;
            _limit_min = 170;
            _limit_max = 180;
            _precision = 3;
            _ready = false;
        }

        virtual void begin()
        {
            digitalWrite(_enable_pin, 0);
            pinMode(_enable_pin, OUTPUT);
            pinMode(_temp_pin, INPUT);
            Tool::begin();
        }

        virtual void start(void)
        {
            digitalWrite(_enable_pin, HIGH);
            Tool::start();
        }

        virtual void stop(void)
        {
            digitalWrite(_enable_pin, LOW);
            Tool::stop();
        }

        virtual void parm_set(enum parm_e p, float val = 0.0)
        {
            switch (p) {
            case PARM_P:
                _limit_max = (short)val;
                if (_limit_max < _limit_min)
                    _limit_min = _limit_max - _precision;
                break;
            case PARM_Q:
                _limit_min = (short)val;
                if (_limit_min > _limit_max)
                    _limit_max = _limit_min + _precision;
                break;
            case PARM_R:
                break;
            case PARM_S:
                break;
            }
        }

        virtual bool ready(void)
        {
            update(micros());
            return _ready;
        }

        virtual float kelvin()
        {
            return (float)_temp + 273.16;
        }

        virtual bool update(unsigned long us_now)
        {
            /* Overheat protection */
            short adc = analogRead(_temp_pin);

            for (int i = 1; i < NUMTEMPS; i++) {
                if (temptable[i-1].adc <= adc &&
                    temptable[i].adc > adc) {
                    _temp = map(adc, temptable[i-1].adc,
                                     temptable[i].adc,
                                     temptable[i-1].celsius,
                                     temptable[i].celsius);
                    break;
                }
            }

            if (_temp > _limit_max) {
                digitalWrite(_enable_pin, LOW);
                _ready = false;
            } else if (_temp < _limit_min) {
                digitalWrite(_enable_pin, HIGH);
            }

            if ((_limit_min - _precision) <= _temp &&
                _temp <= (_limit_max + _precision)) {
                _ready = true;
            } else {
                _ready = false;
            }

if (DEBUG) {
    Serial.print(adc);Serial.print("A: ");
    Serial.print(_temp);Serial.print("C, (");
    Serial.print(_limit_min);Serial.print("-");
    Serial.print(_limit_max);Serial.print(") ");
    Serial.print(_temp*9/5+32);Serial.print("F");
    if (_ready) Serial.print("R");
    Serial.print("\r");
}

            return Tool::update(us_now);
        }
};

#endif /* TOOLFUSER_H */
/* vim: set shiftwidth=4 expandtab:  */
