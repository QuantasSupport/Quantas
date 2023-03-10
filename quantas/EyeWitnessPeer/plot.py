import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict

started_on_round = defaultdict(lambda: 0)
completed_on_round = defaultdict(lambda: 0)
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
        print("adding", completion[i], "for round", i)
        print("recording", cumulative[i], "for round", i)
    return cumulative


def plot(logfile="FastLog.json"):
    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    # they are probably sorted already?
    transactions = sorted(log["tests"][0]["transactions"], key=lambda t: t["round"])
    validations = sorted(log["tests"][0]["validations"], key=lambda t: t["round"])

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
    
    started_by_round = make_completions_cumulative(started_on_round, 50)
    completed_by_round = make_completions_cumulative(completed_on_round, 50)

    plt.plot(started_by_round.keys(), started_by_round.values(),
             color="blue", label="Transactions started")
    plt.plot(completed_by_round.keys(), completed_by_round.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.show()

if __name__ == "__main__":
    plot()
