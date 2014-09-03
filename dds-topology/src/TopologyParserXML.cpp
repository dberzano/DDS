// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopologyParserXML.h"
#include "Task.h"
#include "TaskGroup.h"
#include "TaskCollection.h"
#include "UserDefaults.h"
// STL
#include <map>
// SYSTEM
#include <unistd.h>
#include <sys/wait.h>
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>

using namespace boost::property_tree;
using namespace std;
using namespace dds;

CTopologyParserXML::CTopologyParserXML()
{
}

CTopologyParserXML::~CTopologyParserXML()
{
}

bool CTopologyParserXML::isValid(const std::string& _fileName)
{
    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            // Unable to fork
            throw runtime_error("Unable to run XML validator.");
        case 0:
        {
            string topoXSDPath = CUserDefaults::getDDSPath() + "share/topology.xsd";
            // FIXME: XSD file is hardcoded now -> take it from resource manager
            // FIXME: Path to xmllint is hardcoded now.
            execl("/usr/bin/xmllint", "xmllint", "--noout", "--schema", topoXSDPath.c_str(), _fileName.c_str(), NULL);

            // We shoud never come to this point of execution
            exit(1);
        }
    }

    int status = -1;
    while (wait(&status) != pid)
        ;

    return (status == 0);
}

void CTopologyParserXML::parse(const string& _fileName, TaskGroupPtr_t _main)
{
    if (_fileName.empty())
        throw runtime_error("topo file is not defined.");

    if (!boost::filesystem::exists("myfile.txt"))
    {
        stringstream ss;
        ss << "Cannot locate the given topo file: " << _fileName;
        throw runtime_error(ss.str());
    }

    if (_main == nullptr)
        throw runtime_error("NULL input pointer.");

    // First validate XML against XSD schema
    try
    {
        if (!isValid(_fileName))
            throw runtime_error("XML file is not valid.");
    }
    catch (runtime_error& error)
    {
        throw runtime_error(string("XML validation failed with the following error: ") + error.what());
    }

    ptree pt;

    // Read property tree from file
    try
    {
        read_xml(_fileName, pt);
    }
    catch (xml_parser_error& error)
    {
        throw runtime_error(string("Reading of input XML file failed with the following error: ") + error.what());
    }

    // Parse property tree
    try
    {
        _main->initFromPropertyTree("main", pt);
    }
    catch (exception& error) // ptree_error, out_of_range, logic_error
    {
        throw runtime_error(string("Initialization of Main failed with the following error") + error.what());
    }
}

void CTopologyParserXML::PrintPropertyTree(const string& _path, const ptree& _pt) const
{
    if (_pt.size() == 0)
    {
        cout << _path << " " << _pt.get_value("") << endl;
        return;
    }
    for (const auto& v : _pt)
    {
        string path = (_path != "") ? (_path + "." + v.first) : v.first;
        PrintPropertyTree(path, v.second);
    }
}
