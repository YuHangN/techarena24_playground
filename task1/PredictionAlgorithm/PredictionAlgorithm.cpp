#include "PredictionAlgorithm.hpp"
#include <unordered_map>
#include <optional>
#include <tuple>

// Custom hash function to support std::pair as keys in unordered_map
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

// Custom hash function to support std::tuple as keys in unordered_map
struct triple_hash {
    template <class T1, class T2, class T3>
    std::size_t operator()(const std::tuple<T1, T2, T3>& triple) const {
        return std::hash<T1>()(std::get<0>(triple)) ^ std::hash<T2>()(std::get<1>(triple)) ^ std::hash<T3>()(std::get<2>(triple));
    }
};

// Structure to store day/night statistics for each planet
struct TimeStats {
    // The day time of this planet
    int dayCount = 0;
    // The night time of this planet
    int nightCount = 0;
};

// RoboMemory structure to store Robo's memory data
struct RoboPredictor::RoboMemory {
    std::unordered_map<std::uint64_t, TimeStats> planetObservations; // Single planet records
    std::unordered_map<std::pair<std::uint64_t, std::uint64_t>, bool, pair_hash> planetPairObservations; // Pair records
    std::unordered_map<std::tuple<std::uint64_t, std::uint64_t, std::uint64_t>, bool, triple_hash> planetTripleObservations; // Triple records
    std::optional<std::uint64_t> lastPlanetID; // Last visited planet ID
    std::optional<std::uint64_t> lastLastPlanetID; // Second-to-last visited planet ID
    int consecutiveDayCount = 0; // Counter for consecutive days
};

// Function to predict whether it will be day or night on the next planet
bool RoboPredictor::predictTimeOfDayOnNextPlanet(
    std::uint64_t nextPlanetID, bool spaceshipComputerPrediction) {

    // Step 1: Check the consecutive day count
    if (roboMemory_ptr->consecutiveDayCount >= 2) {
        // If there were three consecutive days, predict night
        return false;
    }

    // Step 2: Try to get a prediction from the triple record
    bool hasTriplePrediction = false;
    bool triplePrediction = false;
    if (roboMemory_ptr->lastLastPlanetID.has_value() && roboMemory_ptr->lastPlanetID.has_value()) {
        auto triple = std::make_tuple(*roboMemory_ptr->lastLastPlanetID, *roboMemory_ptr->lastPlanetID, nextPlanetID);
        auto tripleIt = roboMemory_ptr->planetTripleObservations.find(triple);
        if (tripleIt != roboMemory_ptr->planetTripleObservations.end()) {
            hasTriplePrediction = true;
            triplePrediction = tripleIt->second;
        }
    }

    // Step 3: Try to get a prediction from the pair record
    bool hasPairPrediction = false;
    bool pairPrediction = false;
    if (roboMemory_ptr->lastPlanetID.has_value()) {
        auto pair = std::make_pair(*roboMemory_ptr->lastPlanetID, nextPlanetID);
        auto pairIt = roboMemory_ptr->planetPairObservations.find(pair);
        if (pairIt != roboMemory_ptr->planetPairObservations.end()) {
            hasPairPrediction = true;
            pairPrediction = pairIt->second;
        }
    }

    // Step 4: Check if triple, pair, and spaceship predictions are consistent
    if (hasTriplePrediction && hasPairPrediction && 
        triplePrediction == pairPrediction && pairPrediction == spaceshipComputerPrediction) {
        // If all three predictions agree, return that prediction
        return spaceshipComputerPrediction;
    }

    // Step 5: If no consistent prediction, use single planet record
    auto it = roboMemory_ptr->planetObservations.find(nextPlanetID);
    if (it != roboMemory_ptr->planetObservations.end()) {
        bool historyPrediction = it->second.dayCount > it->second.nightCount;
        return historyPrediction;
    }

    // Step 6: If no single planet record, return spaceship prediction as default
    return spaceshipComputerPrediction;
}

// Function to record whether it was day or night on the last visited planet
void RoboPredictor::observeAndRecordTimeofdayOnNextPlanet(
    std::uint64_t nextPlanetID, bool timeOfDayOnNextPlanet) {

    // Update the consecutive day count
    if (timeOfDayOnNextPlanet) {
        roboMemory_ptr->consecutiveDayCount++;
    } else {
        // Reset consecutive day count if it is night
        roboMemory_ptr->consecutiveDayCount = 0;
    }

    // Record the triple pattern if the last two planets exist
    if (roboMemory_ptr->lastLastPlanetID.has_value() && roboMemory_ptr->lastPlanetID.has_value()) {
        auto triple = std::make_tuple(*roboMemory_ptr->lastLastPlanetID, *roboMemory_ptr->lastPlanetID, nextPlanetID);
        roboMemory_ptr->planetTripleObservations[triple] = timeOfDayOnNextPlanet;
    }

    // Record the pair pattern if the last planet exists
    if (roboMemory_ptr->lastPlanetID.has_value()) {
        auto pair = std::make_pair(*roboMemory_ptr->lastPlanetID, nextPlanetID);
        roboMemory_ptr->planetPairObservations[pair] = timeOfDayOnNextPlanet;
    }

    // Update last visited planet IDs
    roboMemory_ptr->lastLastPlanetID = roboMemory_ptr->lastPlanetID;
    roboMemory_ptr->lastPlanetID = nextPlanetID;

    // Update single planet record
    auto& stats = roboMemory_ptr->planetObservations[nextPlanetID];
    if (timeOfDayOnNextPlanet) {
        stats.dayCount++;
    } else {
        stats.nightCount++;
    }
}

//------------------------------------------------------------------------------
// Please don't modify this file below
//
// Check if RoboMemory does not exceed 64KiB
static_assert(
    sizeof(RoboPredictor::RoboMemory) <= 65536,
    "Robo's memory exceeds 65536 bytes (64KiB) in your implementation. "
    "Prediction algorithms using so much "
    "memory are ineligible. Please reduce the size of your RoboMemory struct.");

// Declare constructor/destructor for RoboPredictor
RoboPredictor::RoboPredictor() {
  roboMemory_ptr = new RoboMemory;
}
RoboPredictor::~RoboPredictor() {
  delete roboMemory_ptr;
}
