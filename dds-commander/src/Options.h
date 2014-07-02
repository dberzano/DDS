// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef DDSOPTIONS_H
#define DDSOPTIONS_H
//=============================================================================
// STD
#include <stdexcept>
#include <iostream>
#include <string>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// DDS
#include "version.h"
#include "Res.h"
#include "UserDefaults.h"
#include "Logger.h"
//=============================================================================
using namespace std;
namespace bpo = boost::program_options;
//=============================================================================
namespace dds
{
    /// \brief dds-commander's container of options
    typedef struct SOptions
    {
        enum ECommands
        {
            cmd_unknown,
            cmd_start,
            cmd_stop,
            cmd_status,
            cmd_submit
        };
        SOptions()
            : m_Command(cmd_start)
        {
        }

        static ECommands getCommandByName(const std::string& _name)
        {
            if ("start" == _name)
                return cmd_start;
            if ("stop" == _name)
                return cmd_stop;
            if ("status" == _name)
                return cmd_status;
            if ("submit" == _name)
                return cmd_submit;

            return cmd_unknown;
        }

        ECommands m_Command;
        CUserDefaults m_userDefaults;
        std::string m_sTopoFile;
    } SOptions_t;
    //=============================================================================
    inline void PrintVersion()
    {
        LOG(MiscCommon::console) << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
                                 << "DDS configuration"
                                 << " v" << USER_DEFAULTS_CFG_VERSION << "\n" << MiscCommon::g_cszReportBugsAddr;
    }
    //=============================================================================
    // Command line parser
    inline bool ParseCmdLine(int _argc, char* _argv[], SOptions* _options) throw(std::exception)
    {
        if (nullptr == _options)
            throw std::runtime_error("Internal error: options' container is empty.");

        // Generic options
        bpo::options_description options("dds-commander options");
        options.add_options()("help,h", "Produce help message");
        options.add_options()("version,v", "Version information");
        options.add_options()("config,c", "A DDS configuration file. The commander will look for DDS config. automatically, if this argument is missing.");
        options.add_options()("topo,t", "A topology file.");
        options.add_options()("command",
                              bpo::value<std::string>(),
                              "The command is a name of dds-commander command."
                              " Can be one of the following: start, stop, status.\n"
                              "For user's convenience it is allowed to call dds-commander without \"--command\" option"
                              " by just specifying the command name directly, like:\ndds-commander start or dds-commander status.\n\n"
                              "Commands:\n"
                              "   start: \tStart dds-commander daemon\n"
                              "   stop: \tStop dds-commander daemon\n"
                              "   status: \tQuery current status of dds-command daemon\n"
                              "   submit: \tSubmit a topology to a defined RMS\n");

        //...positional
        bpo::positional_options_description pd;
        pd.add("command", 1);

        // Parsing command-line
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(_argc, _argv).options(options).positional(pd).run(), vm);
        bpo::notify(vm);

        if (vm.count("help") || vm.empty())
        {
            LOG(MiscCommon::console) << options;
            return false;
        }
        if (vm.count("version"))
        {
            PrintVersion();
            return false;
        }

        // initilize DDS user defaults
        string sCfgFile((vm.count("config")) ? vm["command"].as<std::string>() : _options->m_userDefaults.currentUDFile());
        _options->m_userDefaults.init(sCfgFile);

        // Command
        if (vm.count("command"))
        {
            if (SOptions::cmd_unknown == SOptions::getCommandByName(vm["command"].as<std::string>()))
            {
                LOG(MiscCommon::console) << PROJECT_NAME << " error: unknown command: " << vm["command"].as<std::string>() << "\n\n" << options;
                return false;
            }

            if (SOptions::cmd_submit == SOptions::getCommandByName(vm["command"].as<std::string>()) && !vm.count("topo"))
            {
                LOG(MiscCommon::console) << PROJECT_NAME << " error: specify a topo file"
                                         << "\n\n" << options;
                return false;
            }
        }
        else
        {
            LOG(MiscCommon::console) << PROJECT_NAME << ": Nothing to do\n\n" << options;
            return false;
        }

        _options->m_Command = SOptions::getCommandByName(vm["command"].as<std::string>());

        return true;
    }
}
#endif
