// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/thread/thread.hpp>
// DDS
#include "AgentConnectionManager.h"
#include "AgentChannel.h"
#include "Logger.h"
#include "MonitoringThread.h"

using namespace boost::asio;
using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace sp = std::placeholders;
using boost::asio::ip::tcp;

const std::chrono::milliseconds g_interval(100);
const size_t g_maxWait = 600;

CAgentConnectionManager::CAgentConnectionManager()
    : m_syncHelper(nullptr)
    , m_signals(m_service)
    , m_bStarted(false)
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

    doAwaitStop();
}

CAgentConnectionManager::~CAgentConnectionManager()
{
    stop();
}

void CAgentConnectionManager::doAwaitStop()
{
    m_signals.async_wait([this](boost::system::error_code /*ec*/, int /*signo*/)
                         {
                             // Stop transport engine
                             stop();
                         });
}

void CAgentConnectionManager::start()
{
    if (m_bStarted)
        return;

    m_bStarted = true;
    try
    {
        const float maxIdleTime = CUserDefaults::instance().getOptions().m_server.m_idleTime;

        CMonitoringThread::instance().start(maxIdleTime, []()
                                            {
            LOG(info) << "Idle callback called";
        });

        // Read server info file
        const string sSrvCfg(CUserDefaults::instance().getAgentInfoFileLocation());
        LOG(info) << "Reading server info from: " << sSrvCfg;
        if (sSrvCfg.empty())
            throw runtime_error("Cannot find agent info file.");

        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(sSrvCfg, pt);
        const string sHost(pt.get<string>("agent.host"));
        const string sPort(pt.get<string>("agent.port"));

        LOG(info) << "Contacting DDS agent on " << sHost << ":" << sPort;

        // Resolve endpoint iterator from host and port
        tcp::resolver resolver(m_service);
        tcp::resolver::query query(sHost, sPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Create new communication channel and push handshake message
        m_channel = CAgentChannel::makeNew(m_service);
        // Subscribe to Shutdown command
        std::function<bool(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel * _channel)>
            fSHUTDOWN = [this](SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment, CAgentChannel* _channel) -> bool
        {
            // TODO: adjust the algorithm if we would need to support several agents
            // we have only one agent (newAgent) at the moment
            return this->on_cmdSHUTDOWN(_attachment, m_channel);
        };
        m_channel->registerMessageHandler<cmdSHUTDOWN>(fSHUTDOWN);

        boost::asio::async_connect(m_channel->socket(), endpoint_iterator,
                                   [this](boost::system::error_code ec, tcp::resolver::iterator)
                                   {
            if (!ec)
            {
                // Create handshake message which is the first one for all agents
                SVersionCmd ver;
                m_channel->pushMsg<cmdHANDSHAKE_KEY_VALUE_GUARD>(ver);
                m_channel->m_syncHelper = m_syncHelper;
                m_channel->start();
            }
            else
            {
                LOG(fatal) << "Cannot connect to DDS agent: " << ec.message();
            }
        });

        m_service.run();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CAgentConnectionManager::stop()
{
    if (!m_bStarted)
        return;

    m_bStarted = false;

    LOG(info) << "Shutting down DDS transport...";

    try
    {
        m_service.stop();
        m_channel->stop();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
    LOG(info) << "Shutting down DDS transport - DONE";
}

bool CAgentConnectionManager::on_cmdSHUTDOWN(SCommandAttachmentImpl<cmdSHUTDOWN>::ptr_t _attachment,
                                             CAgentChannel::weakConnectionPtr_t _channel)
{
    stop();
    return true;
}

int CAgentConnectionManager::updateKey(const SUpdateKeyCmd& _cmd)
{
    size_t i(0);
    while (i < g_maxWait)
    {
        if (m_channel && m_channel->m_mtxChannelReady.try_lock())
        {
            m_channel->pushMsg<cmdUPDATE_KEY>(_cmd);
            m_channel->m_mtxChannelReady.unlock();
            return 0;
        }
        else
        {
            ++i;
            this_thread::sleep_for(g_interval);
        }
    }
    return 1;
}