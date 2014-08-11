// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "CommanderChannel.h"
#include "UserDefaults.h"
#include "Process.h"
// BOOST
#include "boost/crc.hpp"

using namespace MiscCommon;
using namespace dds;
using namespace std;

CCommanderChannel::CCommanderChannel(boost::asio::io_service& _service)
    : CConnectionImpl<CCommanderChannel>(_service)
    , m_isHandShakeOK(false)
{
}

void CCommanderChannel::onHeaderRead()
{
    m_headerReadTime = std::chrono::steady_clock::now();
}

int CCommanderChannel::on_cmdREPLY_HANDSHAKE_OK(const CProtocolMessage& _msg)
{
    m_isHandShakeOK = true;

    return 0;
}

int CCommanderChannel::on_cmdSIMPLE_MSG(const CProtocolMessage& _msg)
{
    return 0;
}

int CCommanderChannel::on_cmdGET_HOST_INFO(const CProtocolMessage& _msg)
{
    // Create the command's attachment
    string pidFileName(CUserDefaults::getDDSPath());
    pidFileName += "dds-agent.pid";
    pid_t pid = CPIDFile::GetPIDFromFile(pidFileName);

    SHostInfoCmd cmd;
    get_cuser_name(&cmd.m_username);
    get_hostname(&cmd.m_host);
    cmd.m_version = PROJECT_VERSION_STRING;
    cmd.m_DDSPath = CUserDefaults::getDDSPath();
    cmd.m_agentPort = 0;
    cmd.m_agentPid = pid;
    cmd.m_timeStamp = 0;

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdREPLY_HOST_INFO>(cmd);
    pushMsg(msg);
    return 0;
}

int CCommanderChannel::on_cmdDISCONNECT(const CProtocolMessage& _msg)
{
    stop();
    LOG(info) << "The Agent disconnected...Bye";
    return 0;
}

int CCommanderChannel::on_cmdSHUTDOWN(const CProtocolMessage& _msg)
{
    stop();
    LOG(info) << "The Agent exited.";
    exit(EXIT_SUCCESS);
    return 0;
}

int CCommanderChannel::on_cmdBINARY_ATTACHMENT(const CProtocolMessage& _msg)
{
    SBinaryAttachmentCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    chrono::milliseconds downloadTime = chrono::duration_cast<chrono::milliseconds>(now - m_headerReadTime);

    // Calculate CRC32 of the recieved file data
    boost::crc_32_type crc32;
    crc32.process_bytes(&cmd.m_fileData[0], cmd.m_fileData.size());

    if (crc32.checksum() == cmd.m_crc32)
    {
        // Do something if file is correctly downloaded
    }

    // Form reply command
    SBinaryDownloadStatCmd reply_cmd;
    reply_cmd.m_recievedCrc32 = crc32.checksum();
    reply_cmd.m_recievedFileSize = cmd.m_fileData.size();
    reply_cmd.m_downloadTime = downloadTime.count();

    CProtocolMessage msg;
    msg.encodeWithAttachment<cmdBINARY_DOWNLOAD_STAT>(reply_cmd);
    pushMsg(msg);

    return 0;
}

int CCommanderChannel::on_cmdSET_UUID(const CProtocolMessage& _msg)
{
    SUUIDCmd cmd;
    cmd.convertFromData(_msg.bodyToContainer());

    m_id = cmd.m_id;

    // Write this UUID to file
    //
    //

    LOG(info) << "Recieved a cmdSET_UUID [" << cmd
              << "] command from: " << socket().remote_endpoint().address().to_string();

    return 0;
}
