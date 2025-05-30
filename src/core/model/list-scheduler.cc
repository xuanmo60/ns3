/*
 * Copyright (c) 2005 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "list-scheduler.h"

#include "assert.h"
#include "event-impl.h"
#include "log.h"

#include <string>
#include <utility>

/**
 * @file
 * @ingroup scheduler
 * ns3::ListScheduler implementation.
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ListScheduler");

NS_OBJECT_ENSURE_REGISTERED(ListScheduler);

TypeId
ListScheduler::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ListScheduler")
                            .SetParent<Scheduler>()
                            .SetGroupName("Core")
                            .AddConstructor<ListScheduler>();
    return tid;
}

ListScheduler::ListScheduler()
{
    NS_LOG_FUNCTION(this);
}

ListScheduler::~ListScheduler()
{
}

void
ListScheduler::Insert(const Event& ev)
{
    NS_LOG_FUNCTION(this << &ev);
    for (auto i = m_events.begin(); i != m_events.end(); i++)
    {
        if (ev.key < i->key)
        {
            m_events.insert(i, ev);
            return;
        }
    }
    m_events.push_back(ev);
}

bool
ListScheduler::IsEmpty() const
{
    NS_LOG_FUNCTION(this);
    return m_events.empty();
}

Scheduler::Event
ListScheduler::PeekNext() const
{
    NS_LOG_FUNCTION(this);
    return m_events.front();
}

Scheduler::Event
ListScheduler::RemoveNext()
{
    NS_LOG_FUNCTION(this);
    Event next = m_events.front();
    m_events.pop_front();
    return next;
}

void
ListScheduler::Remove(const Event& ev)
{
    NS_LOG_FUNCTION(this << &ev);
    for (auto i = m_events.begin(); i != m_events.end(); i++)
    {
        if (i->key.m_uid == ev.key.m_uid)
        {
            NS_ASSERT(ev.impl == i->impl);
            m_events.erase(i);
            return;
        }
    }
    NS_ASSERT(false);
}

} // namespace ns3
