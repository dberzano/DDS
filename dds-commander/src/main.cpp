// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
// DDS
#include "ConnectionManager.h"
#include "ErrorCode.h"
#include "INet.h"
#include "SysHelper.h"
// BOOST
#include <boost/property_tree/ptree.hpp>

// silence "Unused typedef" warning using clang 3.7+ and boost < 1.59
#if BOOST_VERSION < 105900
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/property_tree/ini_parser.hpp>
#if BOOST_VERSION < 105900
#pragma clang diagnostic pop
#endif

using namespace std;
using namespace MiscCommon;
using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::user_defaults_api;
using boost::asio::ip::tcp;

//=============================================================================
int main(int argc, char* argv[])
{
    Logger::instance().init(); // Initialize log
    CUserDefaults::instance(); // Initialize user defaults

    vector<string> arguments(argv + 1, argv + argc);
    ostringstream ss;
    copy(arguments.begin(), arguments.end(), ostream_iterator<string>(ss, " "));
    LOG(info) << "Starting with arguments: " << ss.str();

    // Command line parser
    SOptions_t options;
    try
    {
        if (!ParseCmdLine(argc, argv, &options))
            return EXIT_SUCCESS;
    }
    catch (exception& e)
    {
        LOG(log_stderr) << e.what();
        return EXIT_FAILURE;
    }

    // resolving user's home dir from (~/ or $HOME, if present)
    string sWorkDir(CUserDefaults::instance().getOptions().m_server.m_workDir);
    smart_path(&sWorkDir);
    // We need to be sure that there is "/" always at the end of the path
    smart_append<string>(&sWorkDir, '/');
    // pidfile name
    string pidfile_name(sWorkDir);
    pidfile_name += "dds-commander.pid";

    // Checking for "stop" option
    if (SOptions_t::cmd_stop == options.m_Command)
    {
        // TODO: make wait for the process here to check for errors
        const pid_t pid_to_kill = CPIDFile::GetPIDFromFile(pidfile_name);
        if (pid_to_kill > 0 && IsProcessExist(pid_to_kill))
        {
            LOG(log_stdout) << "self exiting (" << pid_to_kill << ")...";
            // TODO: Maybe we need more validations of the process before
            // sending a signal. We don't want to kill someone else.
            kill(pid_to_kill, SIGTERM);

            // Waiting for the process to finish
            size_t iter(0);
            const size_t max_iter = 30;
            while (iter <= max_iter)
            {
                // show "progress dots". Don't use Log, as it will create a new line after each dot.
                if (!IsProcessExist(pid_to_kill))
                {
                    cout << "\n";
                    break;
                }
                cout << ".";
                sleep(1); // sleeping for 1 second
                ++iter;
            }
            if (IsProcessExist(pid_to_kill))
                LOG(log_stderr) << "FAILED to close the process.";
        }

        return EXIT_SUCCESS;
    }

    // Checking for "start" option
    if (SOptions_t::cmd_start == options.m_Command)
    {
        // a visual log marker for a new DDS session
        LOG(info) << "\n"
                  << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
                  << "+++++++++ N E W  D D S  C O M M A N D E R  S E R V E R  S E S S I O N +++++++++\n"
                  << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        // a parsable log marker for a new DDS session
        LOG(info) << "---> DDS commander session <---";

        try
        {
            CPIDFile pidfile(pidfile_name, ::getpid());

            shared_ptr<CConnectionManager> server = make_shared<CConnectionManager>(options);
            server->start();
        }
        catch (exception& e)
        {
            LOG(fatal) << e.what();
            return EXIT_FAILURE;
        }
        catch (...)
        {
            LOG(fatal) << "Unexpected Exception occurred.";
            return EXIT_FAILURE;
        }
    }

    LOG(info) << "DDS commander server is Done. Bye, Bye!";

    return EXIT_SUCCESS;
}
