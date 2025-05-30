/*
 * Copyright (c) 2009 CTTC
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#include "spectrum-interference.h"

#include "spectrum-error-model.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SpectrumInterference");

SpectrumInterference::SpectrumInterference()
    : m_receiving(false),
      m_rxSignal(nullptr),
      m_allSignals(nullptr),
      m_noise(nullptr),
      m_errorModel(nullptr)
{
    NS_LOG_FUNCTION(this);
}

SpectrumInterference::~SpectrumInterference()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SpectrumInterference::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SpectrumInterference")
                            .SetParent<Object>()
                            .SetGroupName("Spectrum")
                            .AddConstructor<SpectrumInterference>();
    return tid;
}

void
SpectrumInterference::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_rxSignal = nullptr;
    m_allSignals = nullptr;
    m_noise = nullptr;
    m_errorModel = nullptr;
    Object::DoDispose();
}

void
SpectrumInterference::StartRx(Ptr<const Packet> p, Ptr<const SpectrumValue> rxPsd)
{
    NS_LOG_FUNCTION(this << p << *rxPsd);
    m_rxSignal = rxPsd;
    m_lastChangeTime = Now();
    m_receiving = true;
    m_errorModel->StartRx(p);
}

void
SpectrumInterference::AbortRx()
{
    m_receiving = false;
}

bool
SpectrumInterference::EndRx()
{
    NS_LOG_FUNCTION(this);
    ConditionallyEvaluateChunk();
    m_receiving = false;
    return m_errorModel->IsRxCorrect();
}

void
SpectrumInterference::AddSignal(Ptr<const SpectrumValue> spd, const Time duration)
{
    NS_LOG_FUNCTION(this << *spd << duration);
    DoAddSignal(spd);
    Simulator::Schedule(duration, &SpectrumInterference::DoSubtractSignal, this, spd);
}

void
SpectrumInterference::DoAddSignal(Ptr<const SpectrumValue> spd)
{
    NS_LOG_FUNCTION(this << *spd);
    ConditionallyEvaluateChunk();
    (*m_allSignals) += (*spd);
    m_lastChangeTime = Now();
}

void
SpectrumInterference::DoSubtractSignal(Ptr<const SpectrumValue> spd)
{
    NS_LOG_FUNCTION(this << *spd);
    ConditionallyEvaluateChunk();
    (*m_allSignals) -= (*spd);
    m_lastChangeTime = Now();
}

void
SpectrumInterference::ConditionallyEvaluateChunk()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("m_receiving: " << m_receiving);
    NS_LOG_LOGIC("m_lastChangeTime: " << m_lastChangeTime << " Now: " << Now());
    bool condition = m_receiving && (Now() > m_lastChangeTime);
    NS_LOG_LOGIC("if condition: " << condition);
    if (condition)
    {
        SpectrumValue sinr = (*m_rxSignal) / ((*m_allSignals) - (*m_rxSignal) + (*m_noise));
        Time duration = Now() - m_lastChangeTime;
        NS_LOG_LOGIC("calling m_errorModel->EvaluateChunk (sinr, duration)");
        m_errorModel->EvaluateChunk(sinr, duration);
    }
}

void
SpectrumInterference::SetNoisePowerSpectralDensity(Ptr<const SpectrumValue> noisePsd)
{
    NS_LOG_FUNCTION(this << noisePsd);
    m_noise = noisePsd;
    // we can initialize m_allSignal only now, because earlier we
    // didn't know what spectrum model was going to be used.
    // we'll now create a zeroed SpectrumValue using the same
    // SpectrumModel which is being specified for the noise.
    m_allSignals = Create<SpectrumValue>(noisePsd->GetSpectrumModel());
}

void
SpectrumInterference::SetErrorModel(Ptr<SpectrumErrorModel> e)
{
    NS_LOG_FUNCTION(this << e);
    m_errorModel = e;
}

} // namespace ns3
