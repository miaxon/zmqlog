/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zutils.h
 * Author: alexcon
 *
 * Created on April 18, 2017, 2:48 PM
 */

#ifndef ZUTILS_H
#define ZUTILS_H
#include <czmq.h>
namespace zmqlog {

    class zutils {
    public:

        static std::string zuud() {
            zuuid_t *uuid = zuuid_new();
            std::string ret = std::string(zuuid_str_canonical(uuid));
            zuuid_destroy (&uuid);
            return ret;
        }
    };
}


#endif /* ZUTILS_H */

