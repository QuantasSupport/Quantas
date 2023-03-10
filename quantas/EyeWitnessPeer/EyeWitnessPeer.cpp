/*
Copyright 2023

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version. QUANTAS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "EyeWitnessPeer.hpp"
#include <iostream>

namespace quantas {
// -- implementation of underlying consensus algorithm --

void PBFTRequest::updateConsensus() {
    if (leader && status == "pre-prepare") {
        status = "prepare";
        EyeWitnessMessage message;
        message.sequenceNum = sequenceNum;
        message.messageType = "pre-prepare";
        message.trans = transaction;
        // addToConsensus(message);
        outbox.push_back(message);
    } else if (status == "pre-prepare") {
        if (statusCount["pre-prepare"] > 0) {
            status = "prepare";
            EyeWitnessMessage message;
            message.sequenceNum = sequenceNum;
            message.messageType = "prepare";
            message.trans = transaction;
            // addToConsensus(message);
            outbox.push_back(message);
        }
    } else if (status == "prepare") {
        if (statusCount["prepare"] > (neighborhoodSize * 2 / 3)) {
            status = "commit";
            EyeWitnessMessage message;
            message.sequenceNum = sequenceNum;
            message.messageType = "commit";
            message.trans = transaction;
            // addToConsensus(message);
            outbox.push_back(message);
        }
    } else if (status == "commit") {
        if (statusCount["commit"] > (neighborhoodSize * 2 / 3)) {
            status = "committed";
        }
    }
}

void PBFTRequest::addToConsensus(EyeWitnessMessage message) {
    // increment the number of times we've seen this type of message for this
    // request
    statusCount[message.messageType]++;
}

bool PBFTRequest::consensusSucceeded() const { return status == "committed"; }

Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>> *
generateSim() {

    Simulation<quantas::EyeWitnessMessage, quantas::EyeWitnessPeer<PBFTRequest>>
        *sim = new Simulation<quantas::EyeWitnessMessage,
                              quantas::EyeWitnessPeer<PBFTRequest>>;
    return sim;
}
} // namespace quantas
