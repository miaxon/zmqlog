/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogmsg.h
 * Author: alexcon
 *
 * Created on April 17, 2017, 12:55 PM
 */

#ifndef ZLOGMSG_H
#define ZLOGMSG_H

#include <iostream>
#include <chrono>
#include <mutex>
#include <uuid/uuid.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>
#include <msgpack.hpp>
#include <zmqpp/zmqpp.hpp>
namespace dmsz {
    namespace log {

        class zlogmsg : public zmqpp::message{
        public:
            zlogmsg();
            virtual ~zlogmsg();
        private:
            void set_timestamp();
        public:
            //MSGPACK_DEFINE(m_remote_class, m_remote_method, m_uuid);

        };
    }
}
#endif /* ZLOGMSG_H */

