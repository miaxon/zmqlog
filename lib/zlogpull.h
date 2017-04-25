/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogproxy.h
 * Author: alexcon
 *
 * Created on April 11, 2017, 2:18 PM
 */

#ifndef ZLOGPULL_H
#define ZLOGPULL_H

#include <zmqpp/zmqpp.hpp>
#include <zmqpp/proxy.hpp>
#include <uuid/uuid.h>
#include <fmt/format.h>
#include <chrono>
#include <atomic>
#include <future>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <semaphore.h>
#include <csignal>

#include "def.h"
#include "macrodef.h"
#include "zlog.h"



namespace zmqlog {

    class zlogpull {
    public:
        static zlogpull& instance() { // i'm singleton
            static zlogpull s;
            return s;
        }
        void start();
        void stop();
        zlogst_ptr logger_st();
        zlogmt_ptr logger_mt();

    private:
        zlogpull();
        virtual ~zlogpull();
        zlogpull(zlogpull&) = delete;
        zlogpull& operator=(zlogpull const&) = delete;
        bool run();
        void route(zmqpp::message& msg);
        void in_tcp();
        void in_ipc();
        void in_inp();
        void in_ctl();
        void poll_reset();
        void poll_cancel();
        void set_endpoints();
        void handle_cmd(const zmqpp::message& req, zmqpp::message& rsp);
        bool self_run(zmqpp::socket* pipe);
        void self_log(std::string msg);
        std::string about();

        static void sighandler(int sig);
    private:
        // initialization list
        zmqpp::context m_ctx;
        zmqpp::socket m_tcp;
        zmqpp::socket m_ipc;
        zmqpp::socket m_inp;
        zmqpp::socket m_ctl; // control channel   
        std::atomic<bool> m_run;
        zmqpp::actor m_self;
        // end of inititalization list
        std::future<bool> m_fut;
        std::string m_tcp_endpoint;
        std::string m_ipc_endpoint;
        std::string m_ctl_endpoint;
        std::string m_inp_endpoint;
        sem_t* m_sem;
        // static members 
        int m_pipe[2];

    };
}
#endif /* ZLOGPULL_H */

