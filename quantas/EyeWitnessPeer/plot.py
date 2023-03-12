import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict

class EventTimelineMuxer:
    """
    this class takes as input a series of integer timestamps (in this
    application, round numbers), which are assumed to correspond to arbitrary
    events that happened at those timestamps, and will then compute how many
    events happened *by* each timestamp in a given range; in other words, it
    outputs how many events happened at each time plus how many events happened
    before that time, for each time; this turns per-round data into
    total-progress data. also, it can store multiple timelines (in this
    application, each corresponding to an experiment/test), where each timeline
    is a separate sequence of events, and average together the data for each
    timeline.
    """

    def __init__(self, lowest_timestamp: int, highest_timestamp: int):
        """
        min: lowest event timestamp to include in a timeline; inclusive
        max: highest event timestamp to include in a timeline; exclusive

        these bounds are used later to output complete cumulative timelines
        where each valid timestamp maps to some number of events, even if no
        events were directly input for that timestamp. half-open range.
        """
        self.min = lowest_timestamp
        self.max = highest_timestamp
        self.timelines = defaultdict(lambda: defaultdict(lambda: 0))
        self.events_by_round_cache = None
    
    def add_event(self, time: int, timeline_id: int=0):
        """
        time: integer timestamp between lowest and highest timestamps
        timeline_id: id of the timeline to store the event under; ignore if you
        only have one timeline
        """
        assert self.min <= time < self.max
        self.events_by_round_cache = None
        self.timelines[timeline_id][time] += 1
    
    def get_cumulative_timelines(self):
        if self.events_by_round_cache is not None:
            return self.events_by_round_cache
        self.events_by_round_cache = {}
        for timeline_id, timeline in self.timelines.items():
            so_far = 0
            cumulative = {}
            for i in range(self.min, self.max):
                so_far += timeline[i]
                cumulative[i] = so_far
            self.events_by_round_cache[timeline_id] = cumulative
        return self.events_by_round_cache

    def get_cumulative_timeline(self, timeline_id=0):
        return self.get_cumulative_timelines()[timeline_id]

    def get_average_cumulative_timeline(self):
        average_timeline = {}
        for i in range(self.min, self.max):
            sum = 0
            for timeline_id in self.timelines.keys():
                sum += self.get_cumulative_timeline(timeline_id)[i]
            average_timeline[i] = sum / len(self.timelines.keys())
        return average_timeline

# todo: include how many rounds are being used in log and then retrieve it here
NUM_ROUNDS = 100

tx_starts = EventTimelineMuxer(0, NUM_ROUNDS)
tx_completes = EventTimelineMuxer(0, NUM_ROUNDS)
local_messages = EventTimelineMuxer(0, NUM_ROUNDS)
all_messages = EventTimelineMuxer(0, NUM_ROUNDS)

validators_still_needed = {}
tolerance_fraction = 2/3


def plot(logfile: str):
    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    for test_index, test in enumerate(log["tests"]):
        # they are probably sorted already?
        transactions = sorted(test["transactions"], key=lambda t: t["round"])
        validations = sorted(test["validations"], key=lambda t: t["round"])
        messages = test["messages"]

        for transaction in transactions:
            tx_starts.add_event(transaction["round"], test_index)
            validators_still_needed[transaction["seqNum"]] = (
                tolerance_fraction*transaction["validatorCount"]
            )
        
        for validation in validations:
            if validators_still_needed[validation["seqNum"]] > 0:
                validators_still_needed[validation["seqNum"]] -= 1
                if validators_still_needed[validation["seqNum"]] <= 0:
                    tx_completes.add_event(validation["round"], test_index)
        
        for message in messages:
            all_messages.add_event(message["round"], test_index)
            if message["transactionType"] == "local":
                local_messages.add_event(message["round"], test_index)
        

    tx_starts_avg_tl = tx_starts.get_average_cumulative_timeline()
    plt.plot(tx_starts_avg_tl.keys(), tx_starts_avg_tl.values(),
             color="blue", label="Transactions started")
    tx_completes_avg_tl = tx_completes.get_average_cumulative_timeline()
    plt.plot(tx_completes_avg_tl.keys(), tx_completes_avg_tl.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.show()

    local_messages_avg_tl = local_messages.get_average_cumulative_timeline()
    plt.plot(local_messages_avg_tl.keys(), local_messages_avg_tl.values(),
             color="blue", label="Messages Sent for Local Transactions")
    all_messages_avg_tl = all_messages.get_average_cumulative_timeline()
    plt.plot(all_messages_avg_tl.keys(), all_messages_avg_tl.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="upper left")
    plt.show()

if __name__ == "__main__":
    plot(Path(__file__).parent / "LargeLog.json")
