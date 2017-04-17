/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogmsg.cpp
 * Author: alexcon
 * 
 * Created on April 17, 2017, 12:55 PM
 */

#include "zlogmsg.h"
namespace dmsz {
    namespace log {

        zlogmsg::zlogmsg() {
        }

        zlogmsg::~zlogmsg() {
        }

        void
        zlogmsg::set_timestamp() {
            using namespace std;
            using namespace chrono;
            auto now = system_clock::now();
            auto ms = duration_cast< milliseconds >(now.time_since_epoch());
            time_t unix_time = duration_cast< seconds >(ms).count();
            *this <<  fmt::format("[ {:%Y-%m-%d %H:%M:%S}]", *localtime(&unix_time));
        }
    }
}
