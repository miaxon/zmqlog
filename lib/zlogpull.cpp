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
using namespace std::placeholders;

namespace zmqlog {

zlogpull::zlogpull() :
m_ctx(),
m_tcp(m_ctx, socket_type::pull),
m_ipc(m_ctx, socket_type::pull),
m_inp(m_ctx, socket_type::pull),
m_ctl(m_ctx, socket_type::reply),
m_run(true),
m_self(bind(&zlogpull::self_run, this, placeholders::_1)),
m_mon_ipc(m_ctx, zmqpp::socket_type::pair),
m_mon_tcp(m_ctx, zmqpp::socket_type::pair),
m_mon_ctl(m_ctx, zmqpp::socket_type::pair)
{
    if ((m_sem = sem_open(ZLOG_SEM, O_CREAT | O_EXCL)) == SEM_FAILED) {
        throw zlog_ex("zlogpull can run one instance only", errno);
        return;
    }

    std::signal(SIGINT, zlogpull::sighandler);
    std::signal(SIGILL, zlogpull::sighandler);
    std::signal(SIGTERM, zlogpull::sighandler);
    std::signal(SIGSEGV, zlogpull::sighandler);
    std::signal(SIGABRT, zlogpull::sighandler);

    ::pipe2(m_pipe, O_CLOEXEC);
    set_endpoints();
    m_fut = async(launch::async, &zlogpull::pull, this);
    self_log("now i'm starting ...\n");
}

void
zlogpull::sighandler(int sig)
{
    zlogpull& this_pull = zlogpull::instance();
    this_pull.self_log(fmt::format("sig handler: {} ({})", sig, ::strsignal(sig)));
    this_pull.stop();
    ::close(this_pull.m_pipe[0]);
    ::close(this_pull.m_pipe[1]);
    sem_unlink(ZLOG_SEM);
    sem_close(this_pull.m_sem);
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
        m_fut = async(launch::async, &zlogpull::pull, this);
    }
}

void
zlogpull::stop()
{
    if (m_run) {
        self_log("\ni'm stopping ...");
        m_run = false;
        poll_cancel();
        m_fut.wait();
        self_log("now i'm stopped.");
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
zlogpull::pull()
{
    try {
        m_tcp.monitor(ZMON_TCP, event::all);
        m_mon_tcp.connect(ZMON_TCP);
        m_ipc.monitor(ZMON_IPC, event::all);
        m_mon_ipc.connect(ZMON_IPC);
        m_ctl.monitor(ZMON_CTL, event::all);
        m_mon_ctl.connect(ZMON_CTL);
    } catch (zmqpp::exception& e) {
        o(e.what());
    }

    try {
        m_tcp.bind(m_tcp_endpoint);
        m_ipc.bind(m_ipc_endpoint);
        m_inp.bind(m_inp_endpoint);
        m_ctl.bind(m_ctl_endpoint);
    } catch (zmqpp::exception& e) {
        o(e.what());
    }



    zmqpp::reactor reactor;
    reactor.add(m_tcp, bind(&zlogpull::in_tcp, this));
    reactor.add(m_ipc, bind(&zlogpull::in_ipc, this));
    reactor.add(m_inp, bind(&zlogpull::in_inp, this));
    reactor.add(m_ctl, bind(&zlogpull::in_ctl, this));
    reactor.add(m_pipe[0], bind(&zlogpull::poll_reset, this));
    reactor.add(m_mon_ipc, bind(&zlogpull::mon_ipc, this));
    reactor.add(m_mon_tcp, bind(&zlogpull::mon_tcp, this));
    reactor.add(m_mon_ctl, bind(&zlogpull::mon_ctl, this));
    while (m_run) {
        try {
            reactor.poll();
        } catch (zmqpp::exception& e) {
            o(e.what());
        }
    }
    //reactor.remove(m_pipe[0]); //crash with sigseg why??
    reactor.remove(m_ctl);
    reactor.remove(m_inp);
    reactor.remove(m_ipc);
    reactor.remove(m_tcp);
    reactor.remove(m_mon_ctl);
    reactor.remove(m_mon_ipc);
    reactor.remove(m_mon_tcp);
    m_tcp.unmonitor();
    m_ipc.unmonitor();
    m_ctl.unmonitor();
    m_mon_tcp.disconnect(ZMON_TCP);
    m_mon_ipc.disconnect(ZMON_IPC);
    m_mon_ctl.disconnect(ZMON_CTL);
    m_tcp.unbind(m_tcp_endpoint);
    m_ipc.unbind(m_ipc_endpoint);
    m_inp.unbind(m_inp_endpoint);
    m_ctl.unbind(m_ctl_endpoint);
    self_log("poll has stopped");
    return true;
}

void
zlogpull::poll_cancel()
{
    char dummy = 1;
    ::write(m_pipe[1], &dummy, 1);
}

void
zlogpull::poll_reset()
{
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
zlogpull::route(message & msg)
{
    o(msg.get(0));
}

void
zlogpull::mon_ipc()
{
    if (!m_run) return;
    message msg;
    m_mon_ipc.receive(msg);
    zpull_event ev(msg);
    self_log(ev.to_string());
}

void
zlogpull::mon_ctl()
{
    if (!m_run) return;
    message msg;
    m_mon_ctl.receive(msg);
    zpull_event ev(msg);
    self_log(ev.to_string());
}

void
zlogpull::mon_tcp()
{
    if (!m_run) return;
    message msg;
    m_mon_tcp.receive(msg);
    zpull_event ev(msg);
    self_log(ev.to_string());
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
    self_log("in_ctl");
    message msg;
    message rsp;
    m_ctl.receive(msg);
    if (msg.parts())
        handle_cmd(msg, rsp);
    m_ctl.send(rsp);

}

void
zlogpull::self_log(string msg)
{
    m_self.pipe()->send(msg, true);
}

string
zlogpull::about()
{
    uint8_t major, minor, patch;
    zmqpp::zmq_version(major, minor, patch);
    return fmt::format("{} {}\ngit version:{}\nzmq version: {}.{}.{}\n{} {} \n",
            APP_NAME, APP_VER, GIT_VERSION, major, minor, patch, __DATE__, __TIME__);
}

bool
zlogpull::self_run(zmqpp::socket* pipe)
{
    message msg;
    zmqpp::signal sig;
    pipe->send(signal::ok);
    o(about());
    while (pipe->receive(msg)) {
        if (msg.is_signal()) {
            msg.get(sig, 0);

            if (sig == zmqpp::signal::stop)
                break;
        }
        o(msg.get(0));
    }
    pipe->send(signal::ok);
    return true;
}
}
