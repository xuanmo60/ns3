/*
 * Copyright (c) 2011 CTTC
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef ANTENNA_MODEL_H
#define ANTENNA_MODEL_H

#include "angles.h"

#include "ns3/object.h"

namespace ns3
{

/**
 * @defgroup antenna Antenna module
 * @brief This module contains several antenna
 * radiation pattern models.
 */

/**
 * @ingroup antenna
 * @ingroup tests
 * @defgroup antenna-tests Antenna module tests
 */

/**
 * @ingroup antenna
 *
 * @brief interface for antenna radiation pattern models
 *
 * This class provides an interface for the definition of antenna
 * radiation pattern models. This interface is based on the use of
 * spherical coordinates, in particular of the azimuth and inclination
 * angles. This choice is the one proposed "Antenna Theory - Analysis
 * and Design", C.A. Balanis, Wiley, 2nd Ed., see in particular
 * section 2.2 "Radiation pattern".
 */
class AntennaModel : public Object
{
  public:
    AntennaModel();
    ~AntennaModel() override;

    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * this method is expected to be re-implemented by each antenna model
     *
     * @param a the spherical angles at which the radiation pattern should
     * be evaluated
     *
     * @return the power gain in dBi of the antenna radiation pattern at
     * the specified angles; dBi means dB with respect to the gain of an
     * isotropic radiator. Since a power gain is used, the efficiency of
     * the antenna is expected to be included in the gain value.
     */
    virtual double GetGainDb(Angles a) = 0;
};

} // namespace ns3

#endif // ANTENNA_MODEL_H
