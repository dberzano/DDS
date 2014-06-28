// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// STD
#include <iostream>
// BOOST
#include <boost/asio.hpp>
// DDS
#include "CommanderServer.h"
#include "TalkToAgent.h"
#include "INet.h"
#include "Logger.h"

using namespace boost::asio;
using namespace std;
using namespace dds;
using namespace MiscCommon;
namespace sp = std::placeholders;

CCommanderServer::CCommanderServer(const SOptions_t& _options)
    : m_options(_options)
{
    m_service = new io_service();
    // get a free port from a given range
    int port = MiscCommon::INet::get_free_port(m_options.m_userDefaults.getOptions().m_general.m_ddsCommanderPortRangeMin,
                                               m_options.m_userDefaults.getOptions().m_general.m_ddsCommanderPortRangeMax);
    // open admin port
    m_acceptor = new ip::tcp::acceptor(*m_service, ip::tcp::endpoint(ip::tcp::v4(), port));

    // init topo
    m_topo.init(m_options.m_sTopoFile);
}

CCommanderServer::~CCommanderServer()
{
    if (m_service)
        delete m_service;
    if (m_acceptor)
        delete m_acceptor;
}

void CCommanderServer::start()
{
    try
    {
        m_acceptor->listen();
        TalkToAgentPtr_t client = CTalkToAgent::makeNew(*m_service);
        m_acceptor->async_accept(client->socket(), std::bind(&CCommanderServer::acceptHandler, this, client, sp::_1));
        m_service->run();
    }
    catch (exception& e)
    {
        LOG(fatal) << e.what();
    }
}

void CCommanderServer::stop()
{
    m_acceptor->close();
    m_service->stop();
    for (const auto& v : m_agents)
    {
        v->stop();
    }
    m_agents.clear();
}

void CCommanderServer::acceptHandler(TalkToAgentPtr_t _client, const boost::system::error_code& _ec)
{
    if (!_ec) // FIXME: Add proper error processing
    {
        _client->start();
        m_agents.push_back(_client);

        TalkToAgentPtr_t newClient = CTalkToAgent::makeNew(*m_service);
        m_acceptor->async_accept(newClient->socket(), std::bind(&CCommanderServer::acceptHandler, this, newClient, sp::_1));
    }
    else
    {
    }
}
