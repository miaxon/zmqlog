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

#define ZSTR_H(x) #x
#define ZSTR_HELPER(x) ZSTR_H(x)
#define ZTRACE(logger, ...) logger->trace("[" __FILE__ " line #" ZSTR_HELPER(__LINE__) "] " __VA_ARGS__)
#define ZDEBUG(logger, ...) logger->debug(__VA_ARGS__)

    
#endif /* MACRODEF_H */