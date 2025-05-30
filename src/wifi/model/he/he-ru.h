/*
 * Copyright (c) 2018
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Stefano Avallone <stavallo@unina.it>
 */

#ifndef HE_RU_H
#define HE_RU_H

#include "ns3/wifi-phy-common.h"

#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

namespace ns3
{

/**
 * This class stores the subcarrier groups of all the available HE RUs.
 */
class HeRu
{
  public:
    /**
     * The different HE Resource Unit (RU) types.
     */
    enum RuType
    {
        RU_26_TONE = 0,
        RU_52_TONE,
        RU_106_TONE,
        RU_242_TONE,
        RU_484_TONE,
        RU_996_TONE,
        RU_2x996_TONE
    };

    /// (lowest index, highest index) pair defining a subcarrier range
    typedef std::pair<int16_t, int16_t> SubcarrierRange;

    /// a vector of subcarrier ranges defining a subcarrier group
    typedef std::vector<SubcarrierRange> SubcarrierGroup;

    /**
     * RU Specification. Stores the information carried by the RU Allocation subfield
     * of the User Info field of Trigger frames (see 9.3.1.22.1 of 802.11ax D8.0).
     * Note that primary80MHz must be true if ruType is RU_2x996_TONE.
     * Internally, this class also stores the RU PHY index (ranging from 1 to the number
     * of RUs of the given type in a channel of the considered width), so that this class
     * contains all the information needed to locate the RU in a 160 MHz channel.
     */
    class RuSpec
    {
      public:
        /**
         * Default constructor
         */
        RuSpec();
        /**
         * Constructor
         *
         * @param ruType the RU type
         * @param index the RU index (starting at 1)
         * @param primary80MHz whether the RU is allocated in the primary 80MHz channel
         */
        RuSpec(RuType ruType, std::size_t index, bool primary80MHz);

        /**
         * Get the RU type
         *
         * @return the RU type
         */
        RuType GetRuType() const;
        /**
         * Get the RU index
         *
         * @return the RU index
         */
        std::size_t GetIndex() const;
        /**
         * Get the primary 80 MHz flag
         *
         * @return true if the RU is in the primary 80 MHz channel and false otherwise
         */
        bool GetPrimary80MHz() const;
        /**
         * Get the RU PHY index
         *
         * @param bw the width of the channel of which the RU is part
         * @param p20Index the index of the primary20 channel
         * @return the RU PHY index
         */
        std::size_t GetPhyIndex(MHz_u bw, uint8_t p20Index) const;

        /**
         * Compare this RU to the given RU.
         *
         * @param other the given RU
         * @return true if this RU compares equal to the given RU, false otherwise
         */
        bool operator==(const RuSpec& other) const;
        /**
         * Compare this RU to the given RU.
         *
         * @param other the given RU
         * @return true if this RU differs from the given RU, false otherwise
         */
        bool operator!=(const RuSpec& other) const;
        /**
         * Compare this RU to the given RU.
         *
         * @param other the given RU
         * @return true if this RU is smaller than the given RU, false otherwise
         */
        bool operator<(const RuSpec& other) const;

      private:
        RuType m_ruType;     //!< RU type
        std::size_t m_index; /**< RU index (starting at 1) as defined by Tables 27-7
                                  to 27-9 of 802.11ax D8.0 */
        bool m_primary80MHz; //!< true if the RU is allocated in the primary 80MHz channel
    };

    /**
     * Struct providing a function call operator to compare two RUs.
     */
    struct RuSpecCompare
    {
        /**
         * Constructor.
         *
         * @param channelWidth the channel width
         * @param p20Index the index of the primary20 channel
         */
        RuSpecCompare(MHz_u channelWidth, uint8_t p20Index);
        /**
         * Function call operator.
         *
         * @param lhs left hand side RU
         * @param rhs right hand side RU
         * @return true if the left hand side RU has its leftmost tone at a lower
         *         frequency than the leftmost tone of the right hand side RU,
         *         false otherwise
         */
        bool operator()(const RuSpec& lhs, const RuSpec& rhs) const;

      private:
        MHz_u m_channelWidth; ///< The channel width
        uint8_t m_p20Index;   ///< Primary20 channel index
    };

    /**
     * Get the number of distinct RUs of the given type (number of tones)
     * available in a HE PPDU of the given bandwidth.
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ruType the RU type (number of tones)
     * @return the number of distinct RUs available
     */
    static std::size_t GetNRus(MHz_u bw, RuType ruType);

    /**
     * Get the set of distinct RUs of the given type (number of tones)
     * available in a HE PPDU of the given bandwidth.
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ruType the RU type (number of tones)
     * @return the set of distinct RUs available
     */
    static std::vector<HeRu::RuSpec> GetRusOfType(MHz_u bw, HeRu::RuType ruType);

    /**
     * Get the set of 26-tone RUs that can be additionally allocated if the given
     * bandwidth is split in RUs of the given type.
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ruType the RU type (number of tones)
     * @return the set of 26-tone RUs that can be additionally allocated
     */
    static std::vector<HeRu::RuSpec> GetCentral26TonesRus(MHz_u bw, HeRu::RuType ruType);

    /**
     * Get the subcarrier group of the RU having the given PHY index among all the
     * RUs of the given type (number of tones) available in a HE PPDU of the
     * given bandwidth. A subcarrier group is defined as one or more pairs
     * indicating the lowest frequency index and the highest frequency index.
     * Note that for channel width of 160 MHz the returned range is relative to
     * the 160 MHz channel (i.e. -1012 to 1012). The PHY index parameter is used to
     * distinguish between lower and higher 80 MHz subchannels.
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ruType the RU type (number of tones)
     * @param phyIndex the PHY index (starting at 1) of the RU
     * @return the subcarrier range of the specified RU
     */
    static SubcarrierGroup GetSubcarrierGroup(MHz_u bw, RuType ruType, std::size_t phyIndex);

    /**
     * Check whether the given RU overlaps with the given set of RUs.
     * Note that for channel width of 160 MHz the returned range is relative to
     * the 160 MHz channel (i.e. -1012 to 1012).
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ru the given RU allocation
     * @param v the given set of RUs
     * @return true if the given RU overlaps with the given set of RUs.
     */
    static bool DoesOverlap(MHz_u bw, RuSpec ru, const std::vector<RuSpec>& v);

    /**
     * Check whether the given RU overlaps with the given tone ranges.
     * Note that for channel width of 160 MHz the returned range is relative to
     * the 160 MHz channel (i.e. -1012 to 1012).
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param ru the given RU allocation
     * @param toneRanges the given set of tone ranges
     * @param p20Index the index of the primary20 channel
     * @return true if the given RU overlaps with the given set of tone ranges.
     */
    static bool DoesOverlap(MHz_u bw,
                            RuSpec ru,
                            const SubcarrierGroup& toneRanges,
                            uint8_t p20Index);

    /**
     * Find the RU allocation of the given RU type overlapping the given
     * reference RU allocation.
     * Note that an assert is generated if the RU allocation is not found.
     *
     * @param bw the bandwidth of the HE PPDU (20, 40, 80, 160)
     * @param referenceRu the reference RU allocation
     * @param searchedRuType the searched RU type
     * @return the searched RU allocation.
     */
    static RuSpec FindOverlappingRu(MHz_u bw, RuSpec referenceRu, RuType searchedRuType);

    /**
     * Get the approximate bandwidth occupied by a RU.
     *
     * @param ruType the RU type
     * @return the approximate bandwidth occupied by the RU
     */
    static MHz_u GetBandwidth(RuType ruType);

    /**
     * Get the RU corresponding to the approximate bandwidth.
     *
     * @param bandwidth the approximate bandwidth occupied by the RU
     * @return the RU type
     */
    static RuType GetRuType(MHz_u bandwidth);

    /**
     * Given the channel bandwidth and the number of stations candidate for being
     * assigned an RU, maximize the number of candidate stations that can be assigned
     * an RU subject to the constraint that all the stations must be assigned an RU
     * of the same size (in terms of number of tones).
     *
     * @param bandwidth the channel bandwidth
     * @param nStations the number of candidate stations. On return, it is set to
     *                  the number of stations that are assigned an RU
     * \param[out] nCentral26TonesRus the number of additional 26-tone RUs that can be
     *                                allocated if the returned RU size is greater than 26 tones
     * @return the RU type
     */
    static RuType GetEqualSizedRusForStations(MHz_u bandwidth,
                                              std::size_t& nStations,
                                              std::size_t& nCentral26TonesRus);

    /// (bandwidth, number of tones) pair
    typedef std::pair<MHz_u, RuType> BwTonesPair;

    /// map (bandwidth, number of tones) pairs to the group of subcarrier ranges
    typedef std::map<BwTonesPair, std::vector<SubcarrierGroup>> SubcarrierGroups;

    /// Subcarrier groups for all RUs (with indices being applicable to primary 80 MHz channel)
    static const SubcarrierGroups m_heRuSubcarrierGroups;

    /// RU allocation map
    using RuAllocationMap = std::map<uint8_t, std::vector<RuSpec>>;

    /// Table 27-26 of IEEE 802.11ax-2021
    static const RuAllocationMap m_heRuAllocations;

    /// Get the RU specs based on RU_ALLOCATION
    /// @param ruAllocation 8 bit RU_ALLOCATION value
    /// @return RU spec associated with the RU_ALLOCATION
    static std::vector<RuSpec> GetRuSpecs(uint8_t ruAllocation);

    /// Get the RU_ALLOCATION value for equal size RUs
    /// @param ruType equal size RU type (generated by GetEqualSizedRusForStations)
    /// @param isOdd if number of stations is an odd number
    /// @return RU_ALLOCATION value
    static uint8_t GetEqualizedRuAllocation(RuType ruType, bool isOdd);

    /// Empty 242-tone RU identifier
    static constexpr uint8_t EMPTY_242_TONE_RU = 113;
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param ruType the RU type
 * @returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const HeRu::RuType& ruType);

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param ru the RU
 * @returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const HeRu::RuSpec& ru);

} // namespace ns3

#endif /* HE_RU_H */
