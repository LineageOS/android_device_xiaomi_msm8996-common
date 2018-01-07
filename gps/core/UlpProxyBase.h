/* Copyright (c) 2013-2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef ULP_PROXY_BASE_H
#define ULP_PROXY_BASE_H

#include <gps_extended.h>
#include <LocationAPI.h>

namespace loc_core {

class LocAdapterBase;

class UlpProxyBase {
public:
    LocPosMode mPosMode;
    bool mFixSet;
    inline UlpProxyBase() {
        mPosMode.mode = LOC_POSITION_MODE_INVALID;
        mFixSet = false;
    }
    inline virtual ~UlpProxyBase() {}
    inline virtual bool sendStartFix() { mFixSet = true; return false; }
    inline virtual bool sendStopFix() { mFixSet = false; return false; }
    inline virtual bool sendFixMode(LocPosMode &params) {
        mPosMode = params;
        return false;
    }

    inline virtual bool reportPosition(const UlpLocation &location,
                                       const GpsLocationExtended &locationExtended,
                                       enum loc_sess_status status,
                                       LocPosTechMask loc_technology_mask) {
        (void)location;
        (void)locationExtended;
        (void)status;
        (void)loc_technology_mask;
        return false;
    }
    inline virtual bool reportSv(const GnssSvNotification& svNotify) {
        (void)svNotify;
        return false;
    }
    inline virtual bool reportSvMeasurement(GnssSvMeasurementSet &svMeasurementSet) {
        (void)svMeasurementSet;
        return false;
    }

    inline virtual bool reportSvPolynomial(GnssSvPolynomial &svPolynomial)
    {
       (void)svPolynomial;
       return false;
    }
    inline virtual bool reportStatus(LocGpsStatusValue status) {

        (void)status;
        return false;
    }
    inline virtual void setAdapter(LocAdapterBase* adapter) {

        (void)adapter;
    }
    inline virtual void setCapabilities(unsigned long capabilities) {

        (void)capabilities;
    }
    inline virtual bool reportBatchingSession(const LocationOptions& options, bool active)
    {
         (void)options;
         (void)active;
         return false;
    }
    inline virtual bool reportPositions(const UlpLocation* ulpLocations,
                                        const GpsLocationExtended* extendedLocations,
                                        const uint32_t* techMasks,
                                        const size_t count)
    {
        (void)ulpLocations;
        (void)extendedLocations;
        (void)techMasks;
        (void)count;
        return false;
    }
    inline virtual bool reportDeleteAidingData(LocGpsAidingData aidingData)
    {
       (void)aidingData;
       return false;
    }
    inline virtual bool reportNmea(const char* nmea, int length)
    {
        (void)nmea;
        (void)length;
        return false;
    }
};

} // namespace loc_core

#endif // ULP_PROXY_BASE_H
