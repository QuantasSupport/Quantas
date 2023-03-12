import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict
from time import time

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

    Timeline = dict[int, int|float]
    """
    A timeline is here defined as a dict mapping each timestamp in a range to
    the number (int) or the average number of events (float) known to have
    happened by that timestamp.
    """

    def __init__(self, lowest_timestamp: int, highest_timestamp: int):
        """
        min: lowest event timestamp to include in a timeline; inclusive
        max: highest event timestamp to include in a timeline; exclusive

        these bounds are used later to output the complete, cumulative timelines
        where every valid timestamp maps to some quantity of events, even if no
        events were directly input for that timestamp. half-open range.
        """
        self.min = lowest_timestamp
        self.max = highest_timestamp
        self.timelines = defaultdict(lambda: defaultdict(lambda: 0))
        self.events_by_round_cache = None
    
    def add_event(self, time: int, timeline_id: int=0) -> None:
        """
        time: integer timestamp between lowest and highest timestamps
        timeline_id: id of the timeline the event belongs to
        """
        assert self.min <= time < self.max
        self.events_by_round_cache = None
        self.timelines[timeline_id][time] += 1
    
    def get_cumulative_timelines(self) -> dict[int, Timeline]:
        """
        Crunches the numbers for the data passed to `add_event` so far. Returns
        a dict that maps each known timeline_id to a complete cumulative timeline.
        """
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

    def get_average_cumulative_timeline(self) -> Timeline:
        """
        Returns a dict that maps each valid timestamp to the average number of
        events known to have occurred by that timestamp across each distinct
        timeline seen by `add_event`.
        """
        average_timeline = {}
        for i in range(self.min, self.max):
            sum = 0
            for timeline_id in self.timelines.keys():
                sum += self.get_cumulative_timelines()[timeline_id][i]
            average_timeline[i] = sum / len(self.timelines.keys())
        return average_timeline


EYEWITNESS_PATH = Path(__file__).parent


def plot(logfile: str):

    # todo: include how many rounds are being used in log and then retrieve it
    # below instead of having a hard-coded constant
    NUM_ROUNDS = 100

    # graph 1:
    tx_starts = EventTimelineMuxer(0, NUM_ROUNDS)
    tx_completes = EventTimelineMuxer(0, NUM_ROUNDS)

    # graph 2:
    local_messages = EventTimelineMuxer(0, NUM_ROUNDS)
    all_messages = EventTimelineMuxer(0, NUM_ROUNDS)

    validators_still_needed = {}
    """maps sequence numbers of initiated transactions to the number of validation
    events that are needed before they can be marked as completed"""

    tolerance_fraction = 2/3
    """fraction of validators that need to confirm a transaction for it to be marked
    as completed"""

    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    for test_index, test in enumerate(log["tests"]):
        transactions = test["transactions"]
        validations = test["validations"]
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
        
    graph_timestamp = round(time())

    plt.title("Transactions Over Time")
    tx_starts_avg_tl = tx_starts.get_average_cumulative_timeline()
    plt.plot(tx_starts_avg_tl.keys(), tx_starts_avg_tl.values(),
             color="blue", label="Transactions started")
    tx_completes_avg_tl = tx_completes.get_average_cumulative_timeline()
    plt.plot(tx_completes_avg_tl.keys(), tx_completes_avg_tl.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_transactions_{graph_timestamp}.png")
    plt.show()

    plt.title("Messages Over Time")
    local_messages_avg_tl = local_messages.get_average_cumulative_timeline()
    plt.plot(local_messages_avg_tl.keys(), local_messages_avg_tl.values(),
             color="blue", label="Messages Sent for Local Transactions")
    all_messages_avg_tl = all_messages.get_average_cumulative_timeline()
    plt.plot(all_messages_avg_tl.keys(), all_messages_avg_tl.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_messages_{graph_timestamp}.png")
    plt.show()

if __name__ == "__main__":
    plot(EYEWITNESS_PATH / "LargeLog.json")
