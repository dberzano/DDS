// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

// DDS
#include "Task.h"
#include "TaskCollection.h"
#include "TaskGroup.h"
#include "TopoBase.h"
#include "TopoElement.h"
#include "TopoFactory.h"
#include "TopoIndex.h"
#include "TopoProperty.h"
#include "TopoUtils.h"
#include "Topology.h"
#include "TopologyParserXML.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace boost::property_tree;
using boost::test_tools::output_test_stream;
using namespace dds;
using namespace dds::topology_api;

BOOST_AUTO_TEST_SUITE(test_dds_topology)

template <class T>
void check_topology_map_task(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second.m_task->getPath() << "\n";
        // std::cout << v.first << " " << v.second.m_task->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_map(const T& _map, output_test_stream& _output)
{
    for (const auto& v : _map)
    {
        _output << v.first << " " << v.second->getPath() << "\n";
        // std::cout << v.first << " " << v.second->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

void check_topology_maps(const string& _topoName)
{
    CTopology topology;
    string topoFile(_topoName + ".xml");
    topology.init(topoFile, true);

    output_test_stream output1(_topoName + "_maps_1.txt", true);
    for (const auto& v : topology.getTopoIndexToTopoElementMap())
    {
        output1 << v.first.getPath() << " " << v.second->getPath() << "\n";
        // std::cout << v.first.getPath() << " " << v.second->getPath() << "\n";
    }
    BOOST_CHECK(output1.match_pattern());

    output_test_stream output2(_topoName + "_maps_2.txt", true);
    check_topology_map_task(topology.getHashToTaskInfoMap(), output2);

    output_test_stream output3(_topoName + "_maps_3.txt", true);
    check_topology_map(topology.getHashToTaskCollectionMap(), output3);

    output_test_stream output4(_topoName + "_maps_4.txt", true);
    check_topology_map(topology.getHashPathToTaskMap(), output4);

    output_test_stream output5(_topoName + "_maps_5.txt", true);
    check_topology_map(topology.getHashPathToTaskCollectionMap(), output5);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_maps)
{
    check_topology_maps("topology_test_1");
    check_topology_maps("topology_test_7");
}

template <class T>
void check_topology_iterator_task(const T& _iterator, output_test_stream& _output)
{
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        _output << it->first << " " << it->second.m_task->getPath() << "\n";
        // std::cout << it->first << " " << it->second.m_task->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

template <class T>
void check_topology_iterator_collection(const T& _iterator, output_test_stream& _output)
{
    for (auto it = _iterator.first; it != _iterator.second; it++)
    {
        _output << it->first << " " << it->second->getPath() << "\n";
        // std::cout << it->first << " " << it->second->getPath() << "\n";
    }
    BOOST_CHECK(_output.match_pattern());
}

BOOST_AUTO_TEST_CASE(test_dds_topology_iterators)
{
    CTopology topology;
    topology.init("topology_test_1.xml");

    // Task iterators
    output_test_stream output1("topology_test_1_iterators_1.txt", true);
    CTopology::TaskInfoIteratorPair_t taskIt1 =
        topology.getTaskInfoIterator([](const CTopology::TaskInfoIterator_t::value_type& value) -> bool {
            TaskPtr_t task = value.second.m_task;
            return (task->getId() == "task1");
        });
    check_topology_iterator_task(taskIt1, output1);

    output_test_stream output2("topology_test_1_iterators_2.txt", true);
    check_topology_iterator_task(topology.getTaskInfoIterator(), output2);

    // Task collection iterators
    output_test_stream output3("topology_test_1_iterators_3.txt", true);
    CTopology::TaskCollectionIteratorPair_t tcIt1 =
        topology.getTaskCollectionIterator([](CTopology::TaskCollectionIterator_t::value_type value) -> bool {
            TaskCollectionPtr_t tc = value.second;
            return (tc->getId() == "collection1");
        });
    check_topology_iterator_collection(tcIt1, output3);

    output_test_stream output4("topology_test_1_iterators_4.txt", true);
    check_topology_iterator_collection(topology.getTaskCollectionIterator(), output4);

    // Task iterators for property
    output_test_stream output5("topology_test_1_iterators_5.txt", true);
    check_topology_iterator_task(topology.getTaskInfoIteratorForPropertyId("property4"), output5);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_iterators_for_property)
{
    CTopology topology;
    topology.init("topology_test_1.xml");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_1)
{
    CTopology topology;
    topology.init("topology_test_1.xml");

    TopoElementPtr_t e1 = topology.getTopoElementByTopoIndex(CTopoIndex("main/task1"));
    BOOST_CHECK(e1->getPath() == "main/task1");

    TopoElementPtr_t e2 = topology.getTopoElementByTopoIndex(CTopoIndex("main/collection1"));
    BOOST_CHECK(e2->getPath() == "main/collection1");

    TopoElementPtr_t e3 = topology.getTopoElementByTopoIndex(CTopoIndex("main/group1"));
    BOOST_CHECK(e3->getPath() == "main/group1");

    TopoElementPtr_t e4 = topology.getTopoElementByTopoIndex(CTopoIndex("main/group1/collection1"));
    BOOST_CHECK(e4->getPath() == "main/group1/collection1");

    TopoElementPtr_t e5 = topology.getTopoElementByTopoIndex(CTopoIndex("main/group2/collection2/task5"));
    BOOST_CHECK(e5->getPath() == "main/group2/collection2/task5");

    BOOST_CHECK_THROW(topology.getTopoElementByTopoIndex(CTopoIndex("wrong_path")), runtime_error);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_1)
{
    CTopology topology;
    topology.init("topology_test_1.xml");
    // std::cout << topology.toString();
    TaskGroupPtr_t main = topology.getMainGroup();

    //  CTopologyParserXML parser;
    //  TaskGroupPtr_t main = make_shared<CTaskGroup>();
    //  parser.parse("topology_test_1.xml", main);

    BOOST_CHECK(main != nullptr);

    BOOST_CHECK(main->getNofTasks() == 22);
    BOOST_CHECK(main->getTotalNofTasks() == 220);
    BOOST_CHECK(main->getN() == 1);
    BOOST_CHECK(main->getId() == "main");
    BOOST_CHECK(main->getNofElements() == 4);
    BOOST_CHECK(main->getParent() == nullptr);
    BOOST_CHECK(main->getPath() == "main");
    BOOST_CHECK_THROW(main->getElement(4), std::out_of_range);

    TopoElementPtr_t element1 = main->getElement(0);
    BOOST_CHECK(element1->getId() == "task1");
    BOOST_CHECK(element1->getType() == ETopoType::TASK);
    BOOST_CHECK(element1->getParent() == main.get());
    BOOST_CHECK(element1->getPath() == "main/task1");
    BOOST_CHECK(element1->getNofTasks() == 1);
    BOOST_CHECK(element1->getTotalNofTasks() == 1);
    TaskPtr_t casted1 = dynamic_pointer_cast<CTask>(element1);
    BOOST_CHECK(casted1->getNofProperties() == 3);
    BOOST_CHECK(casted1->getProperty(0)->getId() == "property1");
    BOOST_CHECK(casted1->getProperty(1)->getId() == "property4");
    BOOST_CHECK(casted1->getProperty(2)->getId() == "property1");
    BOOST_CHECK(casted1->getProperty(0)->getAccessType() == EPropertyAccessType::READ);
    BOOST_CHECK(casted1->getProperty(1)->getAccessType() == EPropertyAccessType::WRITE);
    BOOST_CHECK(casted1->getProperty(2)->getAccessType() == EPropertyAccessType::READWRITE);
    BOOST_CHECK(casted1->getExe() == "app1 -l -n");
    BOOST_CHECK(casted1->getEnv() == "env1");
    BOOST_CHECK(casted1->isExeReachable() == true);
    BOOST_CHECK(casted1->isEnvReachable() == false);
    BOOST_CHECK(casted1->getNofRequirements() == 1);
    RequirementPtr_t requirement = casted1->getRequirements()[0];
    BOOST_CHECK(requirement->getId() == "requirement1");
    BOOST_CHECK(requirement->getType() == ETopoType::REQUIREMENT);
    BOOST_CHECK(requirement->getParent() == element1.get());
    BOOST_CHECK(requirement->getValue() == ".+.gsi.de");
    BOOST_CHECK(requirement->getRequirementType() == ERequirementType::HostName);
    BOOST_CHECK(casted1->getNofTriggers() == 2);
    TriggerPtr_t trigger = casted1->getTriggers()[0];
    BOOST_CHECK(trigger->getId() == "trigger1");
    BOOST_CHECK(trigger->getType() == ETopoType::TRIGGER);
    BOOST_CHECK(trigger->getParent() == element1.get());
    BOOST_CHECK(trigger->getCondition() == EConditionType::TaskCrashed);
    BOOST_CHECK(trigger->getAction() == EActionType::RestartTask);
    BOOST_CHECK(trigger->getArgument() == "5");
    TopoElementPtr_t element2 = main->getElement(1);
    BOOST_CHECK(element2->getId() == "collection1");
    BOOST_CHECK(element2->getType() == ETopoType::COLLECTION);
    BOOST_CHECK(element2->getParent() == main.get());
    BOOST_CHECK(element2->getPath() == "main/collection1");
    BOOST_CHECK(element2->getNofTasks() == 4);
    BOOST_CHECK(element2->getTotalNofTasks() == 4);
    TaskCollectionPtr_t casted2 = dynamic_pointer_cast<CTaskCollection>(element2);
    BOOST_CHECK(casted2->getNofElements() == 4);
    BOOST_CHECK_THROW(casted2->getElement(4), std::out_of_range);

    const auto& casted2_elements = casted2->getElements();
    for (const auto& v : casted2_elements)
    {
        BOOST_CHECK(v->getParent() == element2.get());
    }

    TopoElementPtr_t element3 = main->getElement(2);
    BOOST_CHECK(element3->getId() == "group1");
    BOOST_CHECK(element3->getType() == ETopoType::GROUP);
    BOOST_CHECK(element3->getParent() == main.get());
    BOOST_CHECK(element3->getPath() == "main/group1");
    BOOST_CHECK(element3->getNofTasks() == 8);
    BOOST_CHECK(element3->getTotalNofTasks() == 80);
    TaskGroupPtr_t casted3 = dynamic_pointer_cast<CTaskGroup>(element3);
    BOOST_CHECK(casted3->getNofElements() == 3);
    BOOST_CHECK_THROW(casted3->getElement(3), std::out_of_range);
    BOOST_CHECK(casted3->getN() == 10);

    TopoElementPtr_t element4 = main->getElement(3);
    BOOST_CHECK(element4->getId() == "group2");
    BOOST_CHECK(element4->getType() == ETopoType::GROUP);
    BOOST_CHECK(element4->getParent() == main.get());
    BOOST_CHECK(element4->getPath() == "main/group2");
    BOOST_CHECK(element4->getNofTasks() == 9);
    BOOST_CHECK(element4->getTotalNofTasks() == 135);
    TaskGroupPtr_t casted4 = dynamic_pointer_cast<CTaskGroup>(element4);
    BOOST_CHECK(casted4->getNofElements() == 4);
    BOOST_CHECK_THROW(casted4->getElement(4), std::out_of_range);
    BOOST_CHECK(casted4->getN() == 15);

    TaskCollectionPtr_t casted5 = dynamic_pointer_cast<CTaskCollection>(casted4->getElement(2));
    BOOST_CHECK(casted5->getId() == "collection1");
    BOOST_CHECK(casted5->getType() == ETopoType::COLLECTION);
    BOOST_CHECK(casted5->getParent() == casted4.get());
    BOOST_CHECK(casted5->getPath() == "main/group2/collection1");
    BOOST_CHECK(casted5->getNofTasks() == 4);
    BOOST_CHECK(casted5->getTotalNofTasks() == 4);
    BOOST_CHECK(casted5->getNofElements() == 4);
    BOOST_CHECK(casted5->getRequirements()[0]->getId() == "requirement1");
    BOOST_CHECK_THROW(casted5->getElement(4), std::out_of_range);
    BOOST_CHECK(casted5->getTotalCounter() == 15);

    TaskPtr_t casted6 = dynamic_pointer_cast<CTask>(casted5->getElement(0));
    BOOST_CHECK(casted6->getId() == "task1");
    BOOST_CHECK(casted6->getType() == ETopoType::TASK);
    BOOST_CHECK(casted6->getParent() == casted5.get());
    BOOST_CHECK(casted6->getPath() == "main/group2/collection1/task1");
    BOOST_CHECK(casted6->getNofTasks() == 1);
    BOOST_CHECK(casted6->getTotalNofTasks() == 1);
    BOOST_CHECK(casted6->getNofProperties() == 3);
    BOOST_CHECK(casted6->getExe() == "app1 -l -n");
    BOOST_CHECK(casted6->getEnv() == "env1");
    BOOST_CHECK(casted6->isExeReachable() == true);
    BOOST_CHECK(casted6->isEnvReachable() == false);

    // Test getElementsByType and getTotalCounter
    TopoElementPtrVector_t elements1 = main->getElementsByType(ETopoType::TASK);
    BOOST_CHECK(elements1.size() == 4);
    for (const auto& v : elements1)
    {
        BOOST_CHECK(v->getType() == ETopoType::TASK);
    }
    TaskPtr_t castedTask = dynamic_pointer_cast<CTask>(elements1[0]);
    BOOST_CHECK(castedTask->getId() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 1);
    castedTask = dynamic_pointer_cast<CTask>(elements1[1]);
    BOOST_CHECK(castedTask->getId() == "task1");
    BOOST_CHECK(castedTask->getPath() == "main/group1/task1");
    BOOST_CHECK(castedTask->getTotalCounter() == 10);
    castedTask = dynamic_pointer_cast<CTask>(elements1[2]);
    BOOST_CHECK(castedTask->getId() == "task3");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task3");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);
    castedTask = dynamic_pointer_cast<CTask>(elements1[3]);
    BOOST_CHECK(castedTask->getId() == "task4");
    BOOST_CHECK(castedTask->getPath() == "main/group2/task4");
    BOOST_CHECK(castedTask->getTotalCounter() == 15);

    TopoElementPtrVector_t elements2 = main->getElementsByType(ETopoType::COLLECTION);
    BOOST_CHECK(elements2.size() == 5);
    for (const auto& v : elements2)
    {
        BOOST_CHECK(v->getType() == ETopoType::COLLECTION);
    }
    TaskCollectionPtr_t castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[0]);
    BOOST_CHECK(castedCollection->getId() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 1);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[1]);
    BOOST_CHECK(castedCollection->getId() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[2]);
    BOOST_CHECK(castedCollection->getId() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group1/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 10);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[3]);
    BOOST_CHECK(castedCollection->getId() == "collection1");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection1");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);
    castedCollection = dynamic_pointer_cast<CTaskCollection>(elements2[4]);
    BOOST_CHECK(castedCollection->getId() == "collection2");
    BOOST_CHECK(castedCollection->getPath() == "main/group2/collection2");
    BOOST_CHECK(castedCollection->getTotalCounter() == 15);

    /// test getIndicesByType
    TopoIndexVector_t ids1 = main->getTopoIndicesByType(ETopoType::COLLECTION);
    BOOST_CHECK(ids1.size() == 5);
    BOOST_CHECK(ids1[0].getPath() == "main/collection1");
    BOOST_CHECK(ids1[1].getPath() == "main/group1/collection1");
    BOOST_CHECK(ids1[2].getPath() == "main/group1/collection2");
    BOOST_CHECK(ids1[3].getPath() == "main/group2/collection1");
    BOOST_CHECK(ids1[4].getPath() == "main/group2/collection2");

    TopoIndexVector_t ids2 = main->getTopoIndicesByType(ETopoType::TASK);
    BOOST_CHECK(ids2.size() == 4);
    BOOST_CHECK(ids2[0].getPath() == "main/task1");
    BOOST_CHECK(ids2[1].getPath() == "main/group1/task1");
    BOOST_CHECK(ids2[2].getPath() == "main/group2/task3");
    BOOST_CHECK(ids2[3].getPath() == "main/group2/task4");
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_1)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_1.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_2)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_2.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_3)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_3.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_4)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_4.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_5)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_5.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_6)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("topology_test_6.xml");
    BOOST_CHECK(result == true);
}

BOOST_AUTO_TEST_CASE(test_dds_topology_parser_xml_validation_wrong)
{
    CTopologyParserXML parser;
    bool result = parser.isValid("wrong_file.xml");
    BOOST_CHECK(result == false);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_factory)
{
    // DDSCreateTopoBase
    BOOST_CHECK_THROW(CreateTopoBase(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoBase(ETopoType::TOPO_ELEMENT), runtime_error);
    TopoBasePtr_t baseTopoProperty = CreateTopoBase(ETopoType::TOPO_PROPERTY);
    BOOST_CHECK(baseTopoProperty != nullptr);
    TopoBasePtr_t baseTask = CreateTopoBase(ETopoType::TASK);
    BOOST_CHECK(baseTask != nullptr);
    TopoBasePtr_t baseCollection = CreateTopoBase(ETopoType::COLLECTION);
    BOOST_CHECK(baseCollection != nullptr);
    TopoBasePtr_t baseGroup = CreateTopoBase(ETopoType::GROUP);
    BOOST_CHECK(baseGroup != nullptr);

    // DDSCreateTopoElement
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK_THROW(CreateTopoElement(ETopoType::TOPO_PROPERTY), runtime_error);
    TopoElementPtr_t elementTask = CreateTopoElement(ETopoType::TASK);
    BOOST_CHECK(elementTask != nullptr);
    TopoElementPtr_t elementCollection = CreateTopoElement(ETopoType::COLLECTION);
    BOOST_CHECK(elementCollection != nullptr);
    TopoElementPtr_t elementGroup = CreateTopoElement(ETopoType::GROUP);
    BOOST_CHECK(elementGroup != nullptr);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_utils)
{
    // TopoTypeToUseTag
    BOOST_CHECK_THROW(TopoTypeToUseTag(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToUseTag(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::TASK) == "task");
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::COLLECTION) == "collection");
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::GROUP) == "group");
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::TOPO_PROPERTY) == "property");
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::REQUIREMENT) == "requirement");
    BOOST_CHECK(TopoTypeToUseTag(ETopoType::TRIGGER) == "trigger");

    // UseTagToTopoType
    BOOST_CHECK_THROW(UseTagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(UseTagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(UseTagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK(UseTagToTopoType("task") == ETopoType::TASK);
    BOOST_CHECK(UseTagToTopoType("collection") == ETopoType::COLLECTION);
    BOOST_CHECK(UseTagToTopoType("group") == ETopoType::GROUP);
    BOOST_CHECK(UseTagToTopoType("property") == ETopoType::TOPO_PROPERTY);
    BOOST_CHECK(UseTagToTopoType("requirement") == ETopoType::REQUIREMENT);
    BOOST_CHECK(UseTagToTopoType("trigger") == ETopoType::TRIGGER);

    // TopoTypeToDeclTag
    BOOST_CHECK_THROW(TopoTypeToDeclTag(ETopoType::TOPO_BASE), runtime_error);
    BOOST_CHECK_THROW(TopoTypeToDeclTag(ETopoType::TOPO_ELEMENT), runtime_error);
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::TASK) == "decltask");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::COLLECTION) == "declcollection");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::GROUP) == "group");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::TOPO_PROPERTY) == "property");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::REQUIREMENT) == "declrequirement");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::TOPO_VARS) == "var");
    BOOST_CHECK(TopoTypeToDeclTag(ETopoType::TRIGGER) == "decltrigger");

    // TagToPropertyAccessType
    BOOST_CHECK(TagToPropertyAccessType("read") == EPropertyAccessType::READ);
    BOOST_CHECK(TagToPropertyAccessType("write") == EPropertyAccessType::WRITE);
    BOOST_CHECK(TagToPropertyAccessType("readwrite") == EPropertyAccessType::READWRITE);
    BOOST_CHECK_THROW(TagToPropertyAccessType("readread"), runtime_error);

    // TagToRequirementType
    BOOST_CHECK(TagToRequirementType("wnname") == ERequirementType::WnName);
    BOOST_CHECK(TagToRequirementType("hostname") == ERequirementType::HostName);
    BOOST_CHECK(TagToRequirementType("gpu") == ERequirementType::Gpu);
    BOOST_CHECK_THROW(TagToRequirementType("wn_name"), runtime_error);

    // RequirementTypeToTag
    BOOST_CHECK(RequirementTypeToTag(ERequirementType::WnName) == "WnName");
    BOOST_CHECK(RequirementTypeToTag(ERequirementType::HostName) == "HostName");
    BOOST_CHECK(RequirementTypeToTag(ERequirementType::Gpu) == "Gpu");

    // TagToConditionType
    BOOST_CHECK(TagToConditionType("TaskCrashed") == EConditionType::TaskCrashed);

    // ConditionTypeToTag
    BOOST_CHECK(ConditionTypeToTag(EConditionType::TaskCrashed) == "TaskCrashed");

    // TagToActionType
    BOOST_CHECK(TagToActionType("RestartTask") == EActionType::RestartTask);

    // ActionTypeToTag
    BOOST_CHECK(ActionTypeToTag(EActionType::RestartTask) == "RestartTask");

    // DeclTagToTopoType
    BOOST_CHECK_THROW(DeclTagToTopoType(""), runtime_error);
    BOOST_CHECK_THROW(DeclTagToTopoType("topobase"), runtime_error);
    BOOST_CHECK_THROW(DeclTagToTopoType("topoelement"), runtime_error);
    BOOST_CHECK(DeclTagToTopoType("decltask") == ETopoType::TASK);
    BOOST_CHECK(DeclTagToTopoType("declcollection") == ETopoType::COLLECTION);
    BOOST_CHECK(DeclTagToTopoType("group") == ETopoType::GROUP);
    BOOST_CHECK(DeclTagToTopoType("property") == ETopoType::TOPO_PROPERTY);
    BOOST_CHECK(DeclTagToTopoType("declrequirement") == ETopoType::REQUIREMENT);
    BOOST_CHECK(DeclTagToTopoType("var") == ETopoType::TOPO_VARS);
    BOOST_CHECK(DeclTagToTopoType("decltrigger") == ETopoType::TRIGGER);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_base_find_element)
{
    ptree pt;
    read_xml("topology_test_6.xml", pt);

    const ptree& pt1 = CTopoBase::findElement(ETopoType::TASK, "task1", pt.get_child("topology"));
    BOOST_CHECK(pt1.get<string>("<xmlattr>.id") == "task1");

    const ptree& pt2 = CTopoBase::findElement(ETopoType::COLLECTION, "collection1", pt.get_child("topology"));
    BOOST_CHECK(pt2.get<string>("<xmlattr>.id") == "collection1");

    const ptree& pt3 = CTopoBase::findElement(ETopoType::GROUP, "group1", pt.get_child("topology.main"));
    BOOST_CHECK(pt3.get<string>("<xmlattr>.id") == "group1");

    const ptree& pt4 = CTopoBase::findElement(ETopoType::TOPO_PROPERTY, "property1", pt.get_child("topology"));
    BOOST_CHECK(pt4.get<string>("<xmlattr>.id") == "property1");

    const ptree& pt5 = CTopoBase::findElement(ETopoType::REQUIREMENT, "requirement1", pt.get_child("topology"));
    BOOST_CHECK(pt5.get<string>("<xmlattr>.id") == "requirement1");

    // Wrong path to property tree
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "task1", pt), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "group1", pt.get_child("topology")), logic_error);

    // Wrong element names
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::COLLECTION, "NO", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "NO", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TOPO_PROPERTY, "NO", pt.get_child("topology")), logic_error);

    // Dublicated names
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TASK, "task2", pt.get_child("topology")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::COLLECTION, "collection2", pt.get_child("topology")),
                      logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::GROUP, "group2", pt.get_child("topology.main")), logic_error);
    BOOST_CHECK_THROW(CTopoBase::findElement(ETopoType::TOPO_PROPERTY, "property3", pt.get_child("topology")),
                      logic_error);
}

BOOST_AUTO_TEST_CASE(test_dds_topo_difference)
{
    CTopology topo;
    topo.init("topology_test_diff_1.xml", true);

    CTopology newTopo;
    newTopo.init("topology_test_diff_2.xml", true);

    CTopology::HashSet_t removedTasks;
    CTopology::HashSet_t removedCollections;
    CTopology::HashSet_t addedTasks;
    CTopology::HashSet_t addedCollections;

    topo.getDifference(newTopo, removedTasks, removedCollections, addedTasks, addedCollections);

    output_test_stream output1("topology_test_diff.txt", true);

    output1 << "----- Removed tasks -----\n";
    for (auto& v : removedTasks)
    {
        const STaskInfo& info = topo.getTaskInfoByHash(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Removed collections -----\n";
    for (auto& v : removedCollections)
    {
        TaskCollectionPtr_t collection = topo.getTaskCollectionByHash(v);
        output1 << v << " " << collection->getPath() << "\n";
    }

    output1 << "----- Added tasks -----\n";
    for (auto& v : addedTasks)
    {
        const STaskInfo& info = newTopo.getTaskInfoByHash(v);
        output1 << v << " " << info.m_task->getPath() << " " << info.m_taskPath << "\n";
    }

    output1 << "----- Added collections -----\n";
    for (auto& v : addedCollections)
    {
        TaskCollectionPtr_t collection = newTopo.getTaskCollectionByHash(v);
        output1 << v << " " << collection->getPath() << "\n";
    }

    BOOST_CHECK(output1.match_pattern());
}

BOOST_AUTO_TEST_SUITE_END()
