// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AgentConnectionManager__
#define __DDS__AgentConnectionManager__

// BOOST
#include <boost/asio.hpp>
// DDS
#include "TalkToCommander.h"
#include "Options.h"

namespace dds
{
    class CAgentConnectionManager
    {
      public:
        CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _service);
        virtual ~CAgentConnectionManager();

        void start();
        void stop();

      private:
       // void acceptHandler(CTalkToAgent::connectionPtr_t _agent, const boost::system::error_code& _ec);
        void doAwaitStop();

      private:
        boost::asio::io_service& m_service;
        boost::asio::signal_set m_signals;
        dds::SOptions_t m_options;
        CTalkToCommander::connectionPtrVector_t m_agents;
    };
}

#endif /* defined(__DDS__AgentConnectionManager__) */
