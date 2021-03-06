// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#include "SSHScheduler.h"
#include "TimeMeasure.h"
#include <boost/regex.hpp>
#include <set>

using namespace dds;
using namespace dds::commander_cmd;
using namespace dds::topology_api;
using namespace dds::protocol_api;
using namespace std;
using namespace MiscCommon;

CSSHScheduler::CSSHScheduler()
{
}

CSSHScheduler::~CSSHScheduler()
{
}

void CSSHScheduler::makeSchedule(const CTopology& _topology, const CAgentChannel::weakConnectionPtrVector_t& _channels)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels]() { makeScheduleImpl(_topology, _channels, nullptr, nullptr); });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CSSHScheduler::makeSchedule(const topology_api::CTopology& _topology,
                                 const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                 const CTopology::HashSet_t& _addedTasks,
                                 const CTopology::HashSet_t& _addedCollections)
{
    auto execTime = STimeMeasure<chrono::microseconds>::execution(
        [this, &_topology, &_channels, &_addedTasks, &_addedCollections]() {
            makeScheduleImpl(_topology, _channels, &_addedTasks, &_addedCollections);
        });
    LOG(info) << "Made schedule for tasks in " << execTime << " microsec.";
}

void CSSHScheduler::makeScheduleImpl(const CTopology& _topology,
                                     const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                     const CTopology::HashSet_t* _addedTasks,
                                     const CTopology::HashSet_t* _addedCollections)
{
    m_schedule.clear();

    size_t nofChannels = _channels.size();
    // Map pair<host name, worker id> to vector of channel indeces.
    // This is needed in order to reduce number of regex matches and speed up scheduling.
    hostToChannelMap_t hostToChannelMap;
    for (size_t iChannel = 0; iChannel < nofChannels; ++iChannel)
    {
        const auto& v = _channels[iChannel];
        if (v.expired())
            continue;
        auto ptr = v.lock();

        // Only idle DDS agents
        if (ptr->getState() != EAgentState::idle)
            continue;

        const SHostInfoCmd& hostInfo = ptr->getRemoteHostInfo();
        hostToChannelMap[make_pair(hostInfo.m_host, hostInfo.m_workerId)].push_back(iChannel);
    }

    // TODO: before scheduling the collections we have to sort them by a number of tasks in the collection.
    // TODO: for the moment we are not able to schedule collection without requirements.

    // Collect all tasks that belong to collections
    set<uint64_t> tasksInCollections;
    CollectionMap_t collectionMap;
    CTopology::TaskCollectionIteratorPair_t collections = _topology.getTaskCollectionIterator();
    for (auto it = collections.first; it != collections.second; it++)
    {
        // Only collections that were added has to be scheduled
        if (_addedCollections != nullptr && _addedCollections->find(it->first) == _addedCollections->end())
            continue;

        const vector<uint64_t>& taskHashes = _topology.getTaskHashesByTaskCollectionHash(it->first);
        tasksInCollections.insert(taskHashes.begin(), taskHashes.end());
        collectionMap[it->second->getNofTasks()].push_back(it->first);
    }

    set<uint64_t> scheduledTasks;

    scheduleCollections(_topology, _channels, hostToChannelMap, scheduledTasks, collectionMap, true);
    scheduleTasks(_topology, _channels, hostToChannelMap, scheduledTasks, tasksInCollections, true, _addedTasks);
    scheduleCollections(_topology, _channels, hostToChannelMap, scheduledTasks, collectionMap, false);
    scheduleTasks(_topology, _channels, hostToChannelMap, scheduledTasks, tasksInCollections, false, _addedTasks);

    size_t totalNofTasks =
        (_addedTasks == nullptr) ? _topology.getMainGroup()->getTotalNofTasks() : _addedTasks->size();
    if (totalNofTasks != m_schedule.size())
    {
        LOG(debug) << toString();
        stringstream ss;
        ss << "Unable to make a schedule for tasks. Number of requested tasks: " << totalNofTasks
           << ". Number of scheduled tasks: " << m_schedule.size();
        throw runtime_error(ss.str());
    }

    LOG(debug) << toString();
}

void CSSHScheduler::scheduleTasks(const CTopology& _topology,
                                  const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                  hostToChannelMap_t& _hostToChannelMap,
                                  set<uint64_t>& _scheduledTasks,
                                  const set<uint64_t>& _tasksInCollections,
                                  bool useRequirement,
                                  const CTopology::HashSet_t* _addedTasks)
{
    CTopology::TaskInfoIteratorPair_t tasks = _topology.getTaskInfoIterator();
    for (auto it = tasks.first; it != tasks.second; it++)
    {
        uint64_t id = it->first;

        // Check if tasks is in the added tasks
        if (_addedTasks != nullptr && _addedTasks->find(it->first) == _addedTasks->end())
            continue;

        // Check if task has to be scheduled in the collection
        if (_tasksInCollections.find(id) != _tasksInCollections.end())
            continue;

        // Check if task was already scheduled in the collection
        if (_scheduledTasks.find(id) != _scheduledTasks.end())
            continue;

        TaskPtr_t task = it->second.m_task;

        // First path only for tasks with requirements;
        // Second path for tasks without requirements.

        // SSH scheduler doesn't support multiple requirements
        if (task->getNofRequirements() > 1)
        {
            stringstream ss;
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath()
               << ": SSH scheduler doesn't support multiple requirements.";
            throw runtime_error(ss.str());
        }

        RequirementPtr_t requirement = (task->getNofRequirements() == 1) ? task->getRequirements()[0] : nullptr;
        if ((useRequirement && requirement == nullptr) || (!useRequirement && requirement != nullptr))
            continue;

        bool taskAssigned = false;

        for (auto& v : _hostToChannelMap)
        {
            if (!useRequirement ||
                (useRequirement &&
                 CSSHScheduler::hostPatternMatches(requirement->getValue(),
                                                   (requirement->getRequirementType() == ERequirementType::HostName)
                                                       ? v.first.first
                                                       : v.first.second)))
            {
                if (!v.second.empty())
                {
                    size_t channelIndex = v.second.back();
                    const auto& channel = _channels[channelIndex];

                    SSchedule schedule;
                    schedule.m_channel = channel;
                    schedule.m_taskInfo = it->second;
                    schedule.m_taskID = id;
                    m_schedule.push_back(schedule);

                    v.second.pop_back();

                    taskAssigned = true;

                    break;
                }
            }
        }
        if (!taskAssigned)
        {
            LOG(debug) << toString();
            stringstream ss;
            string requirementStr = (useRequirement)
                                        ? ("Requirement " + requirement->getId() + " couldn't be satisfied.")
                                        : "Not enough worker nodes.";
            ss << "Unable to schedule task <" << id << "> with path " << task->getPath() << ".\n" << requirementStr;
            throw runtime_error(ss.str());
        }
    }
}

void CSSHScheduler::scheduleCollections(const CTopology& _topology,
                                        const CAgentChannel::weakConnectionPtrVector_t& _channels,
                                        hostToChannelMap_t& _hostToChannelMap,
                                        set<uint64_t>& _scheduledTasks,
                                        const CollectionMap_t& _collectionMap,
                                        bool useRequirement)
{
    for (const auto& it_col : _collectionMap)
    {
        for (auto id : it_col.second)
        {
            TaskCollectionPtr_t collection = _topology.getTaskCollectionByHash(id);

            // First path only for collections with requirements;
            // Second path for collections without requirements.

            // SSH scheduler doesn't support multiple requirements
            if (collection->getNofRequirements() > 1)
            {
                stringstream ss;
                ss << "Unable to schedule collection <" << id << "> with path " << collection->getPath()
                   << ": SSH scheduler doesn't support multiple requirements.";
                throw runtime_error(ss.str());
            }

            RequirementPtr_t requirement =
                (collection->getNofRequirements() == 1) ? collection->getRequirements()[0] : nullptr;
            if ((useRequirement && requirement == nullptr) || (!useRequirement && requirement != nullptr))
                continue;

            bool collectionAssigned = false;

            for (auto& v : _hostToChannelMap)
            {
                if (v.second.size() >= collection->getNofTasks() &&
                    (!useRequirement ||
                     (useRequirement &&
                      CSSHScheduler::hostPatternMatches(
                          requirement->getValue(),
                          (requirement->getRequirementType() == ERequirementType::HostName) ? v.first.first
                                                                                            : v.first.second))))
                {
                    const vector<uint64_t>& taskHashes = _topology.getTaskHashesByTaskCollectionHash(id);
                    for (auto hash : taskHashes)
                    {
                        const STaskInfo& info = _topology.getTaskInfoByHash(hash);

                        size_t channelIndex = v.second.back();
                        const auto& channel = _channels[channelIndex];

                        SSchedule schedule;
                        schedule.m_channel = channel;
                        schedule.m_taskInfo = info;
                        schedule.m_taskID = hash;
                        m_schedule.push_back(schedule);

                        v.second.pop_back();

                        _scheduledTasks.insert(hash);
                    }
                    collectionAssigned = true;
                    break;
                }
            }

            if (!collectionAssigned)
            {
                LOG(debug) << toString();
                stringstream ss;
                ss << "Unable to schedule collection <" << id << "> with path " << collection->getPath();
                throw runtime_error(ss.str());
            }
        }
    }
}

const CSSHScheduler::ScheduleVector_t& CSSHScheduler::getSchedule() const
{
    return m_schedule;
}

bool CSSHScheduler::hostPatternMatches(const std::string& _hostPattern, const std::string& _host)
{
    if (_hostPattern.empty())
        return true;
    const boost::regex e(_hostPattern);
    return boost::regex_match(_host, e);
}

string CSSHScheduler::toString()
{
    stringstream ss;
    ss << "Scheduled tasks: " << m_schedule.size() << endl;
    for (const auto& s : m_schedule)
    {
        if (s.m_channel.expired())
            continue;
        auto ptr = s.m_channel.lock();

        ss << "<" << s.m_taskID << ">"
           << " <" << s.m_taskInfo.m_task->getPath() << "> ---> " << ptr->getRemoteHostInfo().m_host << endl;
    }
    return ss.str();
}
