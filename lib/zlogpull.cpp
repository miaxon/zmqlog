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
    init();
    o(zutils::zuud());
}

void
zlogpull::init()
{
    m_sem = sem_open(ZLOG_SEM, O_CREAT | O_EXCL);
    if (m_sem != SEM_FAILED) {
        throw zlog_ex("zlogpull can run one instance only", errno);
    } else {
        ::pipe2(m_pipe, O_CLOEXEC);
        set_endpoints();
        m_fut = async(launch::async, &zlogpull::run, this);
    }
}

void
zlogpull::set_endpoints()
{
    m_inp_endpoint = ZNODE_INTERNAL;
    m_tcp_endpoint = ZNODE_TCP;
    m_ctl_endpoint = ZNODE_TCP_CTL;
    m_ipc_endpoint = ZNODE_IPC;
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
    sem_unlink(ZLOG_SEM);
    sem_close(m_sem);
}

bool
zlogpull::run()
{
    try {
        m_tcp.bind(m_tcp_endpoint);
        m_ipc.bind(m_ipc_endpoint);
        m_inp.bind(m_inp_endpoint);
        m_ctl.connect(m_ctl_endpoint);
    } catch (zmqpp::exception& e) {
        o(e.what());
    }
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
            o(e.what());
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
zlogpull::handle_cmd(const message& req, message& rsp)
{
    int n;
    req.get(n, 0);
    command cmd = static_cast<command> (n);
    switch (cmd) {
        case command::get_frontend:
        {
            int n;
            req.get(n, 1);
            frontend fend = static_cast<frontend> (n);
            if (fend == frontend::ipc)
                rsp << m_ipc_endpoint;
            if (fend == frontend::tcp)
                rsp << "tcp://192.168.255.251:33353";
        }
        default:
            rsp << "";
    }
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
    message rsp;
    m_ctl.receive(msg);
    if (msg.parts())
        handle_cmd(msg, rsp);
    m_ctl.send(rsp);

}
}
