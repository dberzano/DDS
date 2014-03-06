//
//  DDSTaskContainer.cpp
//  DDS
//
//  Created by Andrey Lebedev on 3/5/14.
//
//

#include "DDSTaskContainer.h"
#include "DDSTaskGroup.h"
#include "DDSTaskCollection.h"
#include "DDSTask.h"
#include <memory>

using namespace std;

DDSTaskContainer::DDSTaskContainer(const DDSTaskContainer& rhs) : DDSTopoElement(rhs)
{
    deepCopy(rhs);
}

DDSTaskContainer& DDSTaskContainer::operator=(const DDSTaskContainer& rhs)
{
    if (this != &rhs)
    {
        DDSTopoElement::operator=(rhs);
        deepCopy(rhs);
    }
    return *this;
}

void DDSTaskContainer::setElements(const DDSTopoElementPtrVector_t& _elements)
{
    m_elements.clear();
    for (const auto& v : _elements)
    {
        addElement(v);
    }
}

void DDSTaskContainer::addElement(DDSTopoElementPtr_t _element)
{
    switch (_element->getType())
    {
        case DDSTopoElementType::GROUP:
        {
            DDSTaskGroupPtr_t p = make_shared<DDSTaskGroup>();
            *(p.get()) = *(static_cast<DDSTaskGroup*>(_element.get()));
            m_elements.push_back(p);
        }
        break;

        case DDSTopoElementType::COLLECTION:
        {
            DDSTaskCollectionPtr_t p = make_shared<DDSTaskCollection>();
            *(p.get()) = *(static_cast<DDSTaskCollection*>(_element.get()));
            m_elements.push_back(p);
        }
        break;

        case DDSTopoElementType::TASK:
        {
            DDSTaskPtr_t p = make_shared<DDSTask>();
            *(p.get()) = *(static_cast<DDSTask*>(_element.get()));
            m_elements.push_back(p);
        }
        break;

        default:
            break;
    }
}

void DDSTaskContainer::deepCopy(const DDSTaskContainer& rhs)
{
    m_n = rhs.getN();
    m_minimumRequired = rhs.getMinimumRequired();
    setElements(rhs.getElements());
}