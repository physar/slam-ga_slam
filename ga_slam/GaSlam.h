/*
 * This file is part of GA SLAM.
 * Copyright (C) 2018 Dimitris Geromichalos,
 * Planetary Robotics Lab (PRL), European Space Agency (ESA)
 *
 * GA SLAM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GA SLAM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GA SLAM. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// GA SLAM
#include "ga_slam/TypeDefs.h"
#include "ga_slam/mapping/Map.h"
#include "ga_slam/mapping/DataRegistration.h"
#include "ga_slam/mapping/DataFusion.h"
#include "ga_slam/localization/PoseEstimation.h"
#include "ga_slam/localization/PoseCorrection.h"

// Eigen
#include <Eigen/Geometry>

// PCL
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

// STL
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>

namespace ga_slam {

class GaSlam {
  public:
    GaSlam(void);

    GaSlam(const GaSlam&) = delete;
    GaSlam& operator=(const GaSlam&) = delete;
    GaSlam(GaSlam&&) = delete;
    GaSlam& operator=(GaSlam&&) = delete;

    Pose getPose(void) const { return poseEstimation_.getPose(); }

    std::mutex& getPoseMutex(void) { return poseEstimation_.getPoseMutex(); }

    const Map& getRawMap(void) const { return dataRegistration_.getMap(); }

    std::mutex& getRawMapMutex(void) { return dataRegistration_.getMapMutex(); }

    const Map& getFusedMap(void) const { return dataFusion_.getFusedMap(); }

    const Map& getGlobalMap(void) const {
        return poseCorrection_.getGlobalMap(); }

    std::mutex& getGlobalMapMutex(void) {
        return poseCorrection_.getGlobalMapMutex(); }

    void configure(
            double mapLength, double mapResolution,
            double minElevation, double maxElevation, double voxelSize,
            int numParticles, int resampleFrequency,
            double initialSigmaX, double initialSigmaY, double initialSigmaYaw,
            double predictSigmaX, double predictSigmaY, double predictSigmaYaw,
            double traversedDistanceThreshold, double minSlopeThreshold,
            double slopeSumThresholdMultiplier, double matchAcceptanceThreshold,
            double globalMapLength, double globalMapResolution);

    void poseCallback(const Pose& poseGuess, const Pose& bodyToGroundTF);

    void cloudCallback(
            const Cloud::ConstPtr& cloud,
            const Pose& sensorToBodyTF);

    void matchLocalMapToRawCloud(const Cloud::ConstPtr& rawCloud);

    void matchLocalMapToGlobalMap(void);

    void createGlobalMap(
            const Cloud::ConstPtr& globalCloud,
            const Pose& globalCloudPose);

    template<typename T>
    bool isFutureReady(const std::future<T>& future) const {
        if (!future.valid()) return true;
        return (future.wait_for(std::chrono::milliseconds(0)) ==
                std::future_status::ready);
    }

  protected:
    PoseEstimation poseEstimation_;
    PoseCorrection poseCorrection_;
    DataRegistration dataRegistration_;
    DataFusion dataFusion_;

    std::future<void> scanToMapMatchingFuture_;
    std::future<void> mapToMapMatchingFuture_;

    std::atomic<bool> poseInitialized_;

    double voxelSize_;
};

}  // namespace ga_slam

