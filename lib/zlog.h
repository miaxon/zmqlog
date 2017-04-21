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
#include <fcntl.h>           
#include <sys/stat.h> 
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/time.h>
#include <zmqpp/zmqpp.hpp>

#include "macrodef.h"
#include "zlogmsg.h"
//#include "zutils.h"


namespace zmqlog {

    typedef enum {
        get_frontend = 1
    } command;

    typedef enum {
        stdout = 1,
        file = 2,
        network = 4,
        database = 8
    } sink;

    typedef enum {
        startup,
        core
    } facility;

    typedef enum {
        trace = 0,
        debug,
        info,
        warn,
        err,
        critical,
        off = 6
    } level;

    typedef enum {
        inproc = 1,
        ipc = 2,
        tcp = 4
    } frontend;

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

    class zlog_ex : public std::exception {
    public:

        zlog_ex(const std::string& msg) : _msg(msg) {
        }

        zlog_ex(const std::string& msg, int last_errno) {
            _msg = msg + ": " + ::strerror(last_errno);
        }

        const char* what() const noexcept override {
            return _msg.c_str();
        }
    private:
        std::string _msg;

    };

    template < typename LOCK >
    class zlog {
    public:

        // inproc logger

        zlog(zmqpp::context* pull_ctx, zmqpp::endpoint_t& endpoint) :
        m_sock(*pull_ctx, zmqpp::socket_type::push),
        m_frontend(zmqlog::frontend::inproc) {
            m_sock.connect(endpoint);
        }

        // ipc logger

        zlog() :
        m_ctx(),
        m_sock(m_ctx, zmqpp::socket_type::push),
        m_frontend(zmqlog::frontend::ipc) {
            m_sock.connect(ZNODE_IPC);
        }
        
        // tcp logger

        zlog(zmqpp::endpoint_t endpoint) :
        m_ctx(),
        m_sock(m_ctx, zmqpp::socket_type::push),
        m_frontend(zmqlog::frontend::tcp) {
            m_sock.connect(endpoint);
        }

        virtual ~zlog() {
            o("~zlog");
        };

        zmqlog::frontend
        type() {
            return m_frontend;
        }

        std::string
        typestring() {
            switch (m_frontend) {
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
        zmqpp::socket m_sock;
        zmqlog::frontend m_frontend;
        std::shared_ptr< LOCK > m_lock{ std::make_shared< LOCK >()};
        fmt::MemoryWriter m_fmt;
    private:

        template <typename... Args>
        void log(const char* fmt, const Args&... args) {
            std::unique_lock< LOCK > lock(*m_lock);
            m_fmt.write(fmt, args...);
            m_sock.send(m_fmt.str(), true);
            m_fmt.clear();
        }
    };
    using zlogst = zlog<null_lock_t>;
    using zlogmt = zlog<std::mutex>;
    using zlogst_ptr = std::shared_ptr<zmqlog::zlogst>;
    using zlogmt_ptr = std::shared_ptr<zmqlog::zlogmt>;

}// namespace zmqlog



#endif /* ZLOGGER_H */

