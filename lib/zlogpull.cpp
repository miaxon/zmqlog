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

using namespace std;
using namespace zmqpp;
using namespace zmqlog;

namespace zmqlog {

    zlogpull::zlogpull() :
    m_ctx(),
    m_tcp(m_ctx, socket_type::pull),
    m_ipc(m_ctx, socket_type::pull),
    m_inp(m_ctx, socket_type::pull),
    m_ctl(m_ctx, socket_type::reply),
    m_run(true)
    {
        if(!::access("/tmp/zmqlog.ipc", 0))
            throw std::logic_error("zlogpull can run one instance only.");
        ::pipe2(m_pipe, O_CLOEXEC);
        set_endpoints();
        m_fut = async(launch::async, &zlogpull::run, this);
    }

    void
    zlogpull::set_endpoints()
    {
        //char name[7] = "XXXXXX";
        //m_inp_endpoint = fmt::format("inproc://{}", ::mktemp(name));
        m_inp_endpoint = "inproc://" + uuid();
        m_tcp_endpoint = "tcp://0.0.0.0:33353";
        m_ctl_endpoint = "tcp://0.0.0.0:33355";
        m_ipc_endpoint = "ipc:///tmp/zmqlog.ipc";
    }

    zlogst_ptr
    zlogpull::logger_st()
    {
        zlogst_ptr p(new zlogst(&m_ctx, m_inp_endpoint));
        return p;
    }

    zlogmt_ptr
    zlogpull::logger_mt()
    {
        zlogmt_ptr p(new zlogmt(&m_ctx, m_inp_endpoint));
        return p;
    }

    string
    zlogpull::uuid()
    {
        uuid_t uuid;
        uuid_generate(uuid);
        char uuid_str[37];
        uuid_unparse_lower(uuid, uuid_str);
        return string(uuid_str);
    }

    void
    zlogpull::start()
    {
        if (!m_run) {
            m_run = true;
            m_fut = async(launch::async, &zlogpull::run, this);
        }
    }

    void
    zlogpull::stop()
    {
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

    zlogpull::~zlogpull()
    {
        stop();
        ::close(m_pipe[0]);
        ::close(m_pipe[1]);
    }

    bool
    zlogpull::run()
    {

        m_tcp.bind(m_tcp_endpoint);
        m_ipc.bind(m_ipc_endpoint);
        m_inp.bind(m_inp_endpoint);
        m_ctl.bind(m_ctl_endpoint);

        zmqpp::reactor reactor;
        reactor.add(m_tcp, bind(&zlogpull::in_tcp, this));
        reactor.add(m_ipc, bind(&zlogpull::in_ipc, this));
        reactor.add(m_inp, bind(&zlogpull::in_inp, this));
        reactor.add(m_ctl, bind(&zlogpull::in_ctl, this));
        reactor.add(m_pipe[0], bind(&zlogpull::poll_reset, this));
        while (m_run) {
            try {
                reactor.poll();
            } catch (zmqpp::exception& e) {
                cout << "~poll " << e.what() << endl;
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
    zlogpull::poll_cancel()
    {
        o("poll_cancel ...");
        char dummy = 1;
        ::write(m_pipe[1], &dummy, 1);
    }

    void
    zlogpull::poll_reset()
    {
        o("poll_reset ...");
        char dummy = 0;
        ::read(m_pipe[0], &dummy, 1);
    }

    void
    zlogpull::route(message & msg) const
    {
        o(msg.get(0));
    }

    void
    zlogpull::in_tcp()
    {
        if (!m_run) return;
        message msg;
        m_tcp.receive(msg);
        if (msg.parts())
            route(msg);
    }

    void
    zlogpull::in_ipc()
    {
        if (!m_run) return;
        zmqpp::message msg;
        m_ipc.receive(msg);
        if (msg.parts())
            route(msg);
    }

    void
    zlogpull::in_inp()
    {
        if (!m_run) return;
        message msg;
        m_inp.receive(msg);
        if (msg.parts())
            route(msg);
    }

    void
    zlogpull::in_ctl()
    {
        if (!m_run) return;
        o("in_ctl");
        message msg;
        m_ctl.receive(msg);
        if (msg.is_signal() || !msg.parts()) return;
        message ret;
        int cmd;
        msg.get(cmd, 0);
        if (cmd == (int) cmd::endpoint) {
            int proto;
            msg.get(proto, 1);
            if (proto == (int) proto::ipc)
                ret << m_ipc_endpoint;
            if (proto == (int) proto::tcp)
                ret << m_tcp_endpoint;
            m_ctl.send(ret);
        }


    }
}
