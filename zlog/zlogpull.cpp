/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogproxy.cpp
 * Author: alexcon
 * 
 * Created on April 11, 2017, 2:18 PM
 */

#include <unistd.h>

#include "zlogpull.h"
#include "zlog.h"
namespace dmsz {
    namespace log {

        zlogpull::zlogpull(long pool_timeout) :
        m_ctx(),
        m_tcp(m_ctx, zmqpp::socket_type::pull),
        m_ipc(m_ctx, zmqpp::socket_type::pull),
        m_inp(m_ctx, zmqpp::socket_type::pull),
        m_ctl(m_ctx, zmqpp::socket_type::reply),
        m_run(true),
        m_poll_timeout(pool_timeout) {
            ::pipe2(m_pipe, O_CLOEXEC);
            set_endpoints();
            m_fut = std::async(std::launch::async, &dmsz::log::zlogpull::run, this);            
        }

        void
        zlogpull::set_endpoints() {
            m_inp_endpoint = fmt::format("inproc://{}", uuid());
            m_tcp_endpoint = "tcp://0.0.0.0:33353";
            m_ctl_endpoint = "tcp://0.0.0.0:33355";
            m_ipc_endpoint = fmt::format("ipc:///tmp/{}", uuid());
        }

        dmsz::log::zlogs_ptr
        zlogpull::logger_s() {

            dmsz::log::zlogs_ptr p(new dmsz::log::zlogs(&m_ctx, m_inp_endpoint));
            return p;
        }

        dmsz::log::zlogm_ptr
        zlogpull::logger_m() {
            dmsz::log::zlogm_ptr p(new dmsz::log::zlogm(&m_ctx, m_inp_endpoint));
            return p;
        }

        std::string
        zlogpull::uuid() {
            uuid_t uuid;
            uuid_generate(uuid);
            char uuid_str[37];
            uuid_unparse_lower(uuid, uuid_str);
            return std::string(uuid_str);
        }

        void
        zlogpull::start(long pull_timeout) {
            if (!m_run) {
                m_poll_timeout = pull_timeout;
                //poll_reset();
                m_run = true;
                m_fut = std::async(std::launch::async, &dmsz::log::zlogpull::run, this);

            }
        }

        void
        zlogpull::stop() {
            if (m_run) {
                o("stopping ...");
                m_run = false;
                poll_cancel();
                m_fut.wait();
                m_tcp.unbind(m_tcp_endpoint);
                m_ipc.unbind(m_ipc_endpoint);
                m_inp.unbind(m_inp_endpoint);
                m_ctl.unbind(m_ctl_endpoint);
            }
        }

        zlogpull::~zlogpull() {
            stop();
            ::close(m_pipe[0]);
            ::close(m_pipe[1]);
        }

        bool
        zlogpull::run() {

            m_tcp.bind(m_tcp_endpoint);
            m_ipc.bind(m_ipc_endpoint);
            m_inp.bind(m_inp_endpoint);
            m_ctl.bind(m_ctl_endpoint);

            zmqpp::reactor reactor;
            reactor.add(m_tcp, std::bind(&dmsz::log::zlogpull::in_tcp, this));
            reactor.add(m_ipc, std::bind(&dmsz::log::zlogpull::in_ipc, this));
            reactor.add(m_inp, std::bind(&dmsz::log::zlogpull::in_inp, this));
            reactor.add(m_ctl, std::bind(&dmsz::log::zlogpull::in_ctl, this));
            reactor.add(m_pipe[0], std::bind(&dmsz::log::zlogpull::poll_reset, this));
            while (m_run) {
                try {
                    reactor.poll(m_poll_timeout);
                } catch (zmqpp::exception& e) {
                    std::cout << "~poll " << e.what() << std::endl;
                }
            }
            o("~run returned");
            //reactor.remove(m_pipe[0]); //crash with sigseg why??
            reactor.remove(m_ctl);
            reactor.remove(m_inp);
            reactor.remove(m_ipc);
            reactor.remove(m_tcp);
            return true;
        }

        void
        zlogpull::poll_cancel() {
            o("poll_cancel ...");
            char dummy = 1;
            ::write(m_pipe[1], &dummy, 1);
        }

        void
        zlogpull::poll_reset() {
            o("poll_reset ...");
            char dummy = 0;
            ::read(m_pipe[0], &dummy, 1);
        }

        void
        zlogpull::route(zmqpp::message & msg) const {
            o(msg.get(0));
        }

        void
        zlogpull::in_tcp() {
            if (!m_run) return;
            zmqpp::message msg;
            m_tcp.receive(msg);
            if (msg.parts())
                route(msg);
        }

        void
        zlogpull::in_ipc() {
            if (!m_run) return;
            zmqpp::message msg;
            m_ipc.receive(msg);
            if (msg.parts())
                route(msg);
        }

        void
        zlogpull::in_inp() {
            if (!m_run) return;
            zmqpp::message msg;
            m_inp.receive(msg);
            if (msg.parts())
                route(msg);
        }

        void
        zlogpull::in_ctl() {

            if (!m_run) return;
            std::cout << "in_ctl" << std::endl;
            zmqpp::message msg;
            m_ctl.receive(msg);
            if (msg.is_signal() || !msg.parts()) return;
            zmqpp::message ret;
            int cmd;
            msg.get(cmd, 0);
            if (cmd == (int) dmsz::log::cmd::endpoint) {
                int proto;
                msg.get(proto, 1);
                if (proto == (int) dmsz::log::proto::ipc)
                    ret << m_ipc_endpoint;
                if (proto == (int) dmsz::log::proto::tcp)
                    ret << m_tcp_endpoint;
                m_ctl.send(ret);
            }


        }
    }
}
