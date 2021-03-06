// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__API__AgentConnectionManager__
#define __DDS__API__AgentConnectionManager__
// DDS
#include "AgentChannel.h"
// BOOST
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <condition_variable>

namespace dds
{
    namespace internal_api
    {
        // struct SSyncHelper;

        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
          public:
            typedef std::shared_ptr<CAgentConnectionManager> ptr_t;

            CAgentConnectionManager();
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();
            bool stopped()
            {
                return m_service.stopped();
            }
            void sendCustomCmd(const protocol_api::SCustomCmdCmd& _command);

          public:
            void waitCondition();
            void stopCondition();

          private:
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                CAgentChannel::weakConnectionPtr_t _channel);
            CAgentChannel::weakConnectionPtr_t getAgentChannel()
            {
                return m_channel;
            }

          private:
            boost::asio::io_service m_service;
            // Don't use m_channel directly, only via getAgentChannel
            // In case if channel is destoryed, there still could be user calling update key
            // TODO: need to find a way to hide m_channel from direct access
            CAgentChannel::connectionPtr_t m_channel;
            bool m_bStarted;
            boost::thread_group m_workerThreads;

            /// Condition variable used to stop the current thread.
            /// Execution continues in three cases:
            /// 1) 10 minutes timeout
            /// 2) Failed connection or disconnection
            /// 3) Explicit call to stop
            std::mutex m_waitMutex;
            std::condition_variable m_waitCondition;
        };
    }
}

#endif /* defined(__DDS__API__AgentConnectionManager__) */
