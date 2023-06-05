import json

def getInputWithNumComms(num, commSize = 13):
    return {
        "algorithm": "EyeWitness",
        "parameters": {
            "validatorNeighborhoods": 4,
            "neighborhoodSize": commSize,
            "walletsPerNeighborhood": 10,
            "submitRate": 10.0
        },
        "logFile": f"quantas/EyeWitnessPeer/varyingCommsLogs/{num}Committees.json",
        "distribution": {
            "type": "uniform",
            "maxDelay": 1
        },
        "topology": {
            "type": "complete",
            "initialPeers": num*commSize,
            "totalPeers": num*commSize
        },
        "tests": 10,
        "rounds": 200
    }

def getInputsWithChangingCommSize(peers: int, data_points: int):
    assert int(peers/data_points) == peers/data_points
    experiments = []
    for i in range(0, peers//4, peers//data_points):
        if i == 0:
            i = 1
        experiments.append(
            {
                "algorithm": "EyeWitness",
                "parameters": {
                    "validatorNeighborhoods": 4,
                    "neighborhoodSize": i,
                    "walletsPerNeighborhood": 10,
                    "submitRate": 10.0
                },
                "logFile": f"quantas/EyeWitnessPeer/varyingCommSizesLogs/{i}PerCommittee.json",
                "distribution": {
                    "type": "uniform",
                    "maxDelay": 1
                },
                "topology": {
                    "type": "complete",
                    "initialPeers": peers,
                    "totalPeers": peers
                },
                "tests": 10,
                "rounds": 200
            }
        )
    return experiments

# print(json.dumps({"experiments": [getInputWithNumComms(n) for n in range(20, 200, 16)]}))
print(json.dumps({"experiments": getInputsWithChangingCommSize(2000, 10)}))
