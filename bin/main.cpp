/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cc
 * Author: alexcon
 *
 * Created on April 14, 2017, 4:04 PM
 */

#include <cstdlib>
#include "def.h"
#include "zlog.h"
#include "zlogpull.h"
using namespace std;
static string tcp_endpoint("tcp://127.0.0.1:33353");
static dmsz::log::zlogpull logpull;

/*
 * 
 */


int main(int argc, char** argv) {
    //getchar();
    auto logger = logpull.logger_s();
    ZTRACE(logger, "  MT{}{}", 6, 7);
    ZTRACE(logger, "  MT{}{}", 6, 7);
    ZDEBUG(logger, "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);


    getchar();
    return 0;
}

