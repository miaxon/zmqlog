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
using namespace zmqlog;


static string tcp_endpoint("tcp://127.0.0.1:33353");


ZLOGPULL(logpull);

/*
 * 
 */


int main(int argc, char** argv) {
    //getchar();
    //auto log = logpull.logger_s();
    //LOG_INTERNAL_MT(log, logpull);
    LOG_TCP_ST(log, tcp_endpoint);
    //LOG_IPC_MT(log);
    LOG_IPC_ST(logger);    
    ZTRACE(logger, "  MT{}{}", 6, 7);
    //ZTRACE(logger, "  MT{}{}", 6, 7);
    ZDEBUG(log, "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);


    getchar();
    return 0;
}

