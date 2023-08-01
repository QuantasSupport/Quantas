import json

def getInputWithNumComms(num, commSize = 13):
    return [{
        "algorithm": "Trail",
        "parameters": {
            "validatorNeighborhoods": 3*F+1,
            "neighborhoodSize": commSize,
            "walletsPerNeighborhood": 10,
            "submitRate": 50.0
        },
        "logFile": f"quantas/TrailPeer/varyingCommsLogs/F{F}/{num}Committees.json",
        "distribution": {
            "type": "uniform",
            "maxDelay": 1
        },
        "topology": {
            "type": "complete",
            "initialPeers": num*commSize,
            "totalPeers": num*commSize,
            "maxMsgsRec": 1
        },
        "tests": 5,
        "rounds": 200
    } for F in [0, 1, 2]]

def getInputsWithChangingCommSize():
    peers = 2000
    experiments = []
    # factors of the number of peers
    sizes = [4, 8, 16, 20, 40, 80, 125, 250, 400]
    for size in sizes:
        experiments.append(
            {
                "algorithm": "Trail",
                "parameters": {
                    "validatorNeighborhoods": 4,
                    "neighborhoodSize": size,
                    "walletsPerNeighborhood": 10,
                    "submitRate": 100.0
                },
                "logFile": f"quantas/TrailPeer/varyingCommSizesLogs/{size}PerCommittee.json",
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

# print(json.dumps({"experiments": list(sum([getInputWithNumComms(n) for n in range(20, 200, 16)], []))}))
print(json.dumps({"experiments": getInputsWithChangingCommSize()}))
