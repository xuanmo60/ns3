/*
 * Copyright (c) 2007,2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Jahanzeb Farooq <jahanzeb.farooq@sophia.inria.fr>
 */

#ifndef UPLINK_SCHEDULER_H
#define UPLINK_SCHEDULER_H

#include "service-flow-record.h"
#include "service-flow.h"
#include "ul-mac-messages.h"
#include "wimax-phy.h"

#include "ns3/nstime.h"

#include <stdint.h>

namespace ns3
{

class BaseStationNetDevice;
class SSRecord;
class ServiceFlow;
class ServiceFlowRecord;

/**
 * @ingroup wimax
 * @brief Virtual class for uplink scheduler.
 */
class UplinkScheduler : public Object
{
  public:
    UplinkScheduler();
    /**
     * Constructor
     *
     * @param bs base station device
     */
    UplinkScheduler(Ptr<BaseStationNetDevice> bs);
    ~UplinkScheduler() override;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @return the number of IR opps allocated
     */
    virtual uint8_t GetNrIrOppsAllocated() const;
    /**
     * @brief Set the number of IR opps allocated
     * @param nrIrOppsAllocated number of IR opps allocated
     */
    virtual void SetNrIrOppsAllocated(uint8_t nrIrOppsAllocated);

    /**
     * @return true if the IR interval is allocated
     */
    virtual bool GetIsIrIntrvlAllocated() const;
    /**
     * @brief Set if the IR interval is allocated
     * @param isIrIntrvlAllocated
     */
    virtual void SetIsIrIntrvlAllocated(bool isIrIntrvlAllocated);

    /**
     * @return true if the Inv IR interval is allocated
     */
    virtual bool GetIsInvIrIntrvlAllocated() const;
    /**
     * @brief Set if the Inv IR interval is allocated
     * @param isInvIrIntrvlAllocated
     */
    virtual void SetIsInvIrIntrvlAllocated(bool isInvIrIntrvlAllocated);

    /**
     * @return uplink allocations
     */
    virtual std::list<OfdmUlMapIe> GetUplinkAllocations() const;

    /**
     * @return the timestamp of the IR interval
     */
    virtual Time GetTimeStampIrInterval();
    /**
     * @brief Set timestamp IR interval
     * @param timeStampIrInterval
     */
    virtual void SetTimeStampIrInterval(Time timeStampIrInterval);

    /**
     * @return the DCD timestamp
     */
    virtual Time GetDcdTimeStamp() const;
    /**
     * @brief Set DCD timestamp
     * @param dcdTimeStamp
     */
    virtual void SetDcdTimeStamp(Time dcdTimeStamp);

    /**
     * @return the UCD timestamp
     */
    virtual Time GetUcdTimeStamp() const;
    /**
     * @brief Set UCD timestamp
     * @param ucdTimeStamp
     */
    virtual void SetUcdTimeStamp(Time ucdTimeStamp);

    /**
     * @return the base station device
     */
    virtual Ptr<BaseStationNetDevice> GetBs();
    /**
     * @brief Set base station device
     * @param bs
     */
    virtual void SetBs(Ptr<BaseStationNetDevice> bs);
    /**
     * Determines if channel descriptors sent in the current frame are
     * required to be updated
     */
    virtual void GetChannelDescriptorsToUpdate(bool&, bool&, bool&, bool&) = 0;
    /**
     * @return the calculated allocation start time
     */
    virtual uint32_t CalculateAllocationStartTime() = 0;
    /**
     * Add uplink allocation
     * @param ulMapIe the UL map IE
     * @param allocationSize the allocation size
     * @param symbolsToAllocation the symbols to allocation
     * @param availableSymbols the available symbols
     */
    virtual void AddUplinkAllocation(OfdmUlMapIe& ulMapIe,
                                     const uint32_t& allocationSize,
                                     uint32_t& symbolsToAllocation,
                                     uint32_t& availableSymbols) = 0;
    /**
     * Schedule function
     */
    virtual void Schedule() = 0;
    /**
     * Service unsolicited grants function
     * @param ssRecord the SS record
     * @param schedulingType the scheduling type
     * @param ulMapIe the UL map IE
     * @param modulationType the modulation type
     * @param symbolsToAllocation the symbols to allocation
     * @param availableSymbols the available symbols
     */
    virtual void ServiceUnsolicitedGrants(const SSRecord* ssRecord,
                                          ServiceFlow::SchedulingType schedulingType,
                                          OfdmUlMapIe& ulMapIe,
                                          const WimaxPhy::ModulationType modulationType,
                                          uint32_t& symbolsToAllocation,
                                          uint32_t& availableSymbols) = 0;
    /**
     * Service bandwidth request function
     * @param ssRecord the SS record
     * @param schedulingType the scheduling type
     * @param ulMapIe the UL map IE
     * @param modulationType the modulation type
     * @param symbolsToAllocation the symbols to allocation
     * @param availableSymbols the available symbols
     */
    virtual void ServiceBandwidthRequests(const SSRecord* ssRecord,
                                          ServiceFlow::SchedulingType schedulingType,
                                          OfdmUlMapIe& ulMapIe,
                                          const WimaxPhy::ModulationType modulationType,
                                          uint32_t& symbolsToAllocation,
                                          uint32_t& availableSymbols) = 0;
    /**
     * Service bandwidth request function
     * @param serviceFlow the service flow
     * @param schedulingType the scheduling type
     * @param ulMapIe the UL map IE
     * @param modulationType the modulation type
     * @param symbolsToAllocation the symbols to allocation
     * @param availableSymbols the available symbols
     * @returns true if successful
     */
    virtual bool ServiceBandwidthRequests(ServiceFlow* serviceFlow,
                                          ServiceFlow::SchedulingType schedulingType,
                                          OfdmUlMapIe& ulMapIe,
                                          const WimaxPhy::ModulationType modulationType,
                                          uint32_t& symbolsToAllocation,
                                          uint32_t& availableSymbols) = 0;
    /**
     * Allocate initial ranging interval function
     * @param symbolsToAllocation the symbols to allocation
     * @param availableSymbols the available symbols
     */
    virtual void AllocateInitialRangingInterval(uint32_t& symbolsToAllocation,
                                                uint32_t& availableSymbols) = 0;
    /**
     * Setp service flow function
     * @param ssRecord the SS record
     * @param serviceFlow the service flow
     */
    virtual void SetupServiceFlow(SSRecord* ssRecord, ServiceFlow* serviceFlow) = 0;
    /**
     * Process bandwidth request function
     * @param bwRequestHdr
     */
    virtual void ProcessBandwidthRequest(const BandwidthRequestHeader& bwRequestHdr) = 0;

    /**
     * one time initialization function
     */
    virtual void InitOnce() = 0;

    /**
     * @brief Set requested bandwidth
     * @param sfr service flow record
     */
    virtual void OnSetRequestedBandwidth(ServiceFlowRecord* sfr) = 0;

  private:
    Ptr<BaseStationNetDevice> m_bs;             ///< the base station
    std::list<OfdmUlMapIe> m_uplinkAllocations; ///< uplink allocations
    Time m_timeStampIrInterval;                 ///< timestamp IR interval
    uint8_t m_nrIrOppsAllocated;                ///< number IR opps allocated
    bool m_isIrIntrvlAllocated;                 ///< is IR interval allocated
    bool m_isInvIrIntrvlAllocated;              ///< is Inv IR interval allocated
    Time m_dcdTimeStamp;                        ///< DCD timestamp
    Time m_ucdTimeStamp;                        ///< UCD timestamp
};

} // namespace ns3

#endif /* UPLINK_SCHEDULER_H */
