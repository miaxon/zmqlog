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


#define LOG_INTERNAL_ST(x, y) auto x = y.logger_st()
#define LOG_INTERNAL_MT(x, y) auto x = y.logger_mt()

#define LOG_IPC_ST(x) zlogst_ptr x(new zlogst)
#define LOG_IPC_MT(x) zlogmt_ptr x(new zlogmt)

#define LOG_TCP_ST(x, addr) zlogst_ptr x(new zlogst(addr))
#define LOG_TCP_MT(x, addr) zlogmt_ptr x(new zlogmt(addr))


#define ZSTR_H(x) #x
#define ZSTR_HELPER(x) ZSTR_H(x)
#define ZTRACE(logger, ...) logger->trace("[" __FILE__ " line #" ZSTR_HELPER(__LINE__) "] " __VA_ARGS__)
#define ZDEBUG(logger, ...) logger->debug(__VA_ARGS__)

    
#endif /* MACRODEF_H */