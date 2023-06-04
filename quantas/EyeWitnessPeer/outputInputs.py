import json

def getInputWithNumComms(num, commSize = 13):
    return {
        "algorithm": "EyeWitness",
        "parameters": {
            "validatorNeighborhoods": 4,
            "neighborhoodSize": commSize,
            "walletsPerNeighborhood": 10,
            "submitRate": 5
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
        "tests": 5,
        "rounds": 500
    }

print(json.dumps([getInputWithNumComms(n) for n in range(4, 100, 8)]))
