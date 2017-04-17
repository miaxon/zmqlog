/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zlogworker.cpp
 * Author: alexcon
 * 
 * Created on April 11, 2017, 2:35 PM
 */

#include "zlogworker.h"
namespace dmsz {
    namespace log {

        zlogworker::zlogworker(zmqpp::context& ctx, std::string& endpoint) :
        m_endpoint(endpoint),
        m_ctx(ctx),
        m_zsock(ctx, zmqpp::socket_type::dealer)
        {

        }

        zlogworker::~zlogworker()
        {
        }

        void
        zlogworker::work()
        {
            m_zsock.connect(m_endpoint);
            while (true) {
                zmqpp::message msg;
                m_zsock.receive(msg);
                if (msg.parts())
                    log(msg);
            }
        }

        void
        zlogworker::log(zmqpp::message& msg)
        {
            std::string text;
            std::string iden;
            msg >> iden >> text;
            std::cout << iden << " " << text << std::endl;
        }
    }
}