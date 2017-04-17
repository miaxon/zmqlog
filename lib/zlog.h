/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogger.h
 * Author: alexcon
 *
 * Created on April 10, 2017, 1:49 PM
 */

#ifndef ZLOG_H
#define ZLOG_H

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <mutex>
#include <uuid/uuid.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>
#include <zmqpp/zmqpp.hpp>

#include "macrodef.h"
#include "zlogmsg.h"



namespace zmqlog {

    enum FACILITY {
        STARTUP,
        CORE
    };

    enum LEVEL {
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    enum proto {
        tcp,
        ipc,
        inproc
    };

    struct null_lock_t {

        void
        lock() const {
        }

        bool
        try_lock() const {
            return true;
        }

        void
        unlock() const {
        }
    };

    template < typename LOCK >
    class zlog {
    public:

        // inproc logger

        zlog(zmqpp::context* pull_ctx, zmqpp::endpoint_t& endpoint) :
        m_push(*pull_ctx, zmqpp::socket_type::push),
        m_proto(zmqlog::proto::inproc) {
            m_push.connect(endpoint);
        }

        // ipc logger

        zlog() :
        m_ctx(),
        m_push(m_ctx, zmqpp::socket_type::push),
        m_proto(zmqlog::proto::ipc) {
            m_push.connect("ipc:///tmp/zmqlog.ipc");
        }
        
        // tcp logger

        zlog(zmqpp::endpoint_t& endpoint) :
        m_ctx(),
        m_push(m_ctx, zmqpp::socket_type::push),
        m_proto(zmqlog::proto::tcp) {
            m_push.connect(endpoint);
        }

        virtual ~zlog() {
            o("~zlog");
        };

        zmqlog::proto
        proto() {
            return m_proto;
        }

        std::string
        protostring() {
            switch (m_proto) {
                case tcp:
                    return "tcp";
                case ipc:
                    return "ipc";
                case inproc:
                    return "inproc";
            }
            return "undefined";
        }

        template <typename Arg1, typename... Args>
        void
        debug(const char* fmt, const Arg1& arg1, const Args&... args) {

            /*using namespace std;
            using namespace chrono;
            auto now = system_clock::now();
            auto ms = duration_cast< milliseconds >(now.time_since_epoch());
            time_t unix_time = duration_cast< seconds >(ms).count();
            std::string text = fmt::format("[ {:%Y-%m-%d %H:%M:%S}] {}", *localtime(&unix_time), str);
            dmsz::log::zlogmsg msg;
            msg << text;
            m_push.send(msg);*/

            log(fmt, arg1, args...);
        }

        template <typename Arg1, typename... Args>
        void
        trace(const char* fmt, const Arg1& arg1, const Args&... args) {
            log(fmt, arg1, args...);
        }

    private:
        zmqpp::context m_ctx;
        zmqpp::socket m_push;
        zmqlog::proto m_proto;
        std::shared_ptr< LOCK > m_lock{ std::make_shared< LOCK >()};
        fmt::MemoryWriter m_fmt;
    private:

        template <typename... Args>
        void log(const char* fmt, const Args&... args) {
            std::unique_lock< LOCK > lock(*m_lock);
            m_fmt.write(fmt, args...);
            m_push.send(m_fmt.str(), true);
            m_fmt.clear();
        }
    };
    using zlogst = zlog<null_lock_t>;
    using zlogmt = zlog<std::mutex>;
    using zlogst_ptr = std::shared_ptr<zmqlog::zlogst>;
    using zlogmt_ptr = std::shared_ptr<zmqlog::zlogmt>;

}// namespace zmqlog



#endif /* ZLOGGER_H */

