/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zpull_event.h
 * Author: alexcon
 *
 * Created on April 27, 2017, 10:42 AM
 */

#ifndef ZPULL_EVENT_H
#define ZPULL_EVENT_H

#include <zmqpp/socket.hpp>

class zpull_event {
public:

    uint16_t event;
    std::string addr;

    zpull_event(zmqpp::message& msg) {
        const uint8_t *ptr = reinterpret_cast<const uint8_t *> (msg.raw_data(0));
        event = *(reinterpret_cast<const uint16_t *> (ptr));
        addr = msg.get(1);
    }

    virtual ~zpull_event() {
    }

    std::string to_string() {
        std::string str;
        switch (event) {
            case zmqpp::event::accept_failed:
                str = "could not accept client connection";
                break;
            case zmqpp::event::accepted:
                str = "connection accepted to bound interface";
                break;
            case zmqpp::event::close_failed:
                str = "connection couldn't be closed";
                break;
            case zmqpp::event::closed:
                str = "onnection closed";
                break;
            case zmqpp::event::bind_failed:
                str = "socket could not bind to an address";
                break;
            case zmqpp::event::connect_delayed:
                str = "synchronous connect failed, it's being polled";
                break;
            case zmqpp::event::connect_retried:
                str = "asynchronous connect / reconnection attempt";
                break;
            case zmqpp::event::connected:
                str = "connection established";
                break;
            case zmqpp::event::disconnected:
                str = "broken session";
                break;
            case zmqpp::event::listening:
                str = "socket bound to an address, ready to accept connections";
                break;
            case zmqpp::event::monitor_stopped:
                str = "this monitor socket will not receive event anymore";
                break;
            default:
                str = "unknown event type";
                break;
        }
        //return fmt::format("zpull_event: [{}] {}, addr: {}", event, str, addr);
        return fmt::format("zpull_event: [0x{0:x}] {1}, addr: {2}", event, str, addr);
    }
};

#endif /* ZPULL_EVENT_H */

