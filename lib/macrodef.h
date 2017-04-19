/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   macrodef.h
 * Author: alexcon
 *
 * Created on April 14, 2017, 10:23 AM
 */

#ifndef MACRODEF_H
#define MACRODEF_H

#define o(x) std::cout << x << std::endl
#define e(x) std::cerr << x << std::endl

#define ZLOG_SEM "zmq_logpull_semaphore_one"
#define ZNODE_IPC "ipc:///tmp/.zlog_ipc_pipe"
#define ZNODE_INTERNAL "inproc:///zlog_inproc_pipe"
#define ZNODE_TCP "tcp://0.0.0.0:33353"
#define ZNODE_TCP_CTL "tcp://0.0.0.0:33355"

#define LOG_INTERNAL_ST(x, y) auto x = y.logger_st()
#define LOG_INTERNAL_MT(x, y) auto x = y.logger_mt()

#define LOG_TCP_ST(x, y) zlogst_ptr x(new zlogst(y))
#define LOG_TCP_MT(x, y) zlogmt_ptr x(new zlogmt(y))

#define LOG_IPC_ST(x) zlogst_ptr x(new zlogst())
#define LOG_IPC_MT(x) zlogmt_ptr x(new zlogmt())


#define ZSTR_H(x) #x
#define ZSTR_HELPER(x) ZSTR_H(x)
#define ZTRACE(logger, ...) logger->trace("[" __FILE__ " line #" ZSTR_HELPER(__LINE__) "] " __VA_ARGS__)
#define ZDEBUG(logger, ...) logger->debug(__VA_ARGS__)

    
#endif /* MACRODEF_H */