import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict

started_on_round = defaultdict(lambda: 0)
completed_on_round = defaultdict(lambda: 0)
local_messages_on_round = defaultdict(lambda: 0)
all_messages_on_round = defaultdict(lambda: 0)

sors = []
cors = []
lmors = []
amors = []

validators_still_needed = {}
tolerance_fraction = 2/3

def make_completions_cumulative(completion: dict, rounds: int):
    """takes a dict mapping round numbers to some value and produces a new
    dict that maps all round numbers in the range to the sum of that value
    across that round number and all previous round numbers. if the input dict
    stores how many events happened in each round, the output dict stores how
    many events happened by the end of each round."""
    so_far = 0
    cumulative = {}
    for i in range(1, rounds+1):
        so_far += completion[i]
        cumulative[i] = so_far
    return cumulative


def plot(logfile="FastLog.json"):
    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    for test in log["tests"]:
        # they are probably sorted already?
        transactions = sorted(test["transactions"], key=lambda t: t["round"])
        validations = sorted(test["validations"], key=lambda t: t["round"])
        messages = test["messages"]

        for transaction in transactions:
            started_on_round[transaction["round"]] += 1
            validators_still_needed[transaction["seqNum"]] = (
                tolerance_fraction*transaction["validatorCount"]
            )
        
        for validation in validations:
            if validators_still_needed[validation["seqNum"]] > 0:
                validators_still_needed[validation["seqNum"]] -= 1
                if validators_still_needed[validation["seqNum"]] <= 0:
                    completed_on_round[validation["round"]] += 1
        
        for message in messages:
            if message["transactionType"] == "local":
                local_messages_on_round[message["round"]] += 1
                all_messages_on_round[message["round"]] += 1
            else:
                all_messages_on_round[message["round"]] += 1
        
        sors.append(make_completions_cumulative(started_on_round, 100))
        cors.append(make_completions_cumulative(completed_on_round, 100))
        lmors.append(make_completions_cumulative(local_messages_on_round, 100))
        amors.append(make_completions_cumulative(all_messages_on_round, 100))
    
    started_by_round = {}
    completed_by_round = {}
    for key in sors[0].keys():
        started_by_round[key] = sum(sor[key] for sor in sors) / len(sors)
    for key in cors[0].keys():
        completed_by_round[key] = sum(cor[key] for cor in cors) / len(cors)
        

    plt.plot(started_by_round.keys(), started_by_round.values(),
             color="blue", label="Transactions started")
    plt.plot(completed_by_round.keys(), completed_by_round.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.show()

    local_messages_by_round = {}
    all_messages_by_round = {}
    for key in lmors[0].keys():
        local_messages_by_round[key] = sum(lmor[key] for lmor in lmors) / len(lmors)
    for key in amors[0].keys():
        all_messages_by_round[key] = sum(amor[key] for amor in amors) / len(amors)

    plt.plot(local_messages_by_round.keys(), local_messages_by_round.values(),
             color="blue", label="Messages Sent for Local Transactions")
    plt.plot(all_messages_by_round.keys(), all_messages_by_round.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="upper left")
    plt.show()

if __name__ == "__main__":
    plot("LargeLog.json")
