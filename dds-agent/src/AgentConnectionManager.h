// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__AGENT__AgentConnectionManager__
#define __DDS__AGENT__AgentConnectionManager__

// DDS
#include "CommanderChannel.h"
#include "Options.h"
#include "SMUIChannel.h"
// BOOST
#include <boost/asio.hpp>

namespace dds
{
    namespace agent_cmd
    {
        class CAgentConnectionManager : public std::enable_shared_from_this<CAgentConnectionManager>
        {
            typedef std::vector<pid_t> childrenPidContainer_t;
            typedef std::vector<protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t>
                updateKeyAttachmentQueue_t;

          public:
            CAgentConnectionManager(const SOptions_t& _options, boost::asio::io_service& _io_service);
            virtual ~CAgentConnectionManager();

          public:
            void start();
            void stop();

          private:
            void doAwaitStop();
            void onNewUserTask(pid_t _pid);
            void terminateChildrenProcesses();
            bool on_cmdSHUTDOWN(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSHUTDOWN>::ptr_t _attachment,
                                CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdUPDATE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment,
                                  CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdUPDATE_KEY_ERROR(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY_ERROR>::ptr_t _attachment,
                CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdDELETE_KEY(protocol_api::SCommandAttachmentImpl<protocol_api::cmdDELETE_KEY>::ptr_t _attachment,
                                  CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdSIMPLE_MSG(protocol_api::SCommandAttachmentImpl<protocol_api::cmdSIMPLE_MSG>::ptr_t _attachment,
                                  CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdSTOP_USER_TASK(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdSTOP_USER_TASK>::ptr_t _attachment,
                CCommanderChannel::weakConnectionPtr_t _channel);
            bool on_cmdCUSTOM_CMD(protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment,
                                  CCommanderChannel::weakConnectionPtr_t _channel);

            // Messages from shared memory
            bool on_cmdUPDATE_KEY_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdUPDATE_KEY>::ptr_t _attachment);
            bool on_cmdCUSTOM_CMD_SM(
                protocol_api::SCommandAttachmentImpl<protocol_api::cmdCUSTOM_CMD>::ptr_t _attachment);

            void taskExited(int _pid, int _exitCode);

          private:
            boost::asio::io_service& m_service;
            boost::asio::signal_set m_signals;
            SOptions_t m_options;
            CCommanderChannel::connectionPtr_t m_agent;
            CSMUIChannel::connectionPtr_t m_SMChannel;
            childrenPidContainer_t m_children;
            std::mutex m_childrenContainerMutex;
            bool m_bStarted;
            boost::thread_group m_workerThreads;
        };
    }
}

#endif /* defined(__DDS__AGENT__AgentConnectionManager__) */
