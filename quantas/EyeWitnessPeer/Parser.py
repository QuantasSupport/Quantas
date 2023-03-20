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
GRAPH_TIMESTAMPS = round(time())

def plotTOT(tx_starts: EventTimelineMuxer, honest_tx_starts: EventTimelineMuxer, tx_completes: EventTimelineMuxer):
    plt.title("Transactions Over Time")
    tx_starts_avg_tl = tx_starts.get_average_cumulative_timeline()
    htx_starts_avg_tl = honest_tx_starts.get_average_cumulative_timeline()
    plt.plot(tx_starts_avg_tl.keys(), tx_starts_avg_tl.values(),
             color="orange", label="Transactions started")
    plt.plot(htx_starts_avg_tl.keys(), htx_starts_avg_tl.values(),
             color="green", label="Honest transactions started")
    tx_completes_avg_tl = tx_completes.get_average_cumulative_timeline()
    plt.plot(tx_completes_avg_tl.keys(), tx_completes_avg_tl.values(),
             color="blue", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_transactions_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

def plotMOT(local_messages: EventTimelineMuxer, all_messages: EventTimelineMuxer):
    plt.title("Messages Over Time")
    local_messages_avg_tl = local_messages.get_average_cumulative_timeline()
    plt.plot(local_messages_avg_tl.keys(), local_messages_avg_tl.values(),
             color="blue", label="Messages Sent for Local Transactions")
    all_messages_avg_tl = all_messages.get_average_cumulative_timeline()
    plt.plot(all_messages_avg_tl.keys(), all_messages_avg_tl.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_messages_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

def plotCOT(corrupt_wallets: EventTimelineMuxer):
    plt.title("Corrupt Wallets Over Time")
    corrupt_wallets_avg_tl = corrupt_wallets.get_average_cumulative_timeline()
    plt.plot(corrupt_wallets_avg_tl.keys(), corrupt_wallets_avg_tl.values(),
             color="blue", label="no validation")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_wallets_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

def parser(logfile: str):

    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    num_rounds = log["tests"][0]["roundInfo"]["rounds"] + 1

    # graph 1:
    tx_starts = EventTimelineMuxer(0, num_rounds)
    honest_tx_starts = EventTimelineMuxer(0, num_rounds)
    tx_completes = EventTimelineMuxer(0, num_rounds)

    # graph 2:
    local_messages = EventTimelineMuxer(0, num_rounds)
    all_messages = EventTimelineMuxer(0, num_rounds)

    # graph 3:
    corrupt_wallets = EventTimelineMuxer(0, num_rounds)

    proposals = {}
    """maps sequence numbers of initiated transactions to the transaction and the number of validation
    events that are needed before it can be marked as completed"""

    tolerance_fraction = 2/3
    """fraction of validators that need to confirm a transaction for it to be marked
    as completed"""

    
    
    for test_index, test in enumerate(log["tests"]):
        transactions = test["transactions"]
        validations = test["validations"]
        messages = test["messages"]

        max_seq_num = -1
        for transaction in transactions:
            if (transaction["seqNum"] > max_seq_num):
                max_seq_num = transaction["seqNum"]
            
            tx_starts.add_event(transaction["round"], test_index)
            if transaction["honest"]:
                honest_tx_starts.add_event(transaction["round"], test_index)
            proposals[transaction["seqNum"]] = (
                {
                    "validators_still_needed": tolerance_fraction*transaction["validatorCount"],
                    **transaction                
                }
            )
        
        for validation in validations:
            if proposals[validation["seqNum"]]["validators_still_needed"] > 0:
                proposals[validation["seqNum"]]["validators_still_needed"] -= 1
                if proposals[validation["seqNum"]]["validators_still_needed"] <= 0:
                    tx_completes.add_event(validation["round"], test_index)
                    proposals[validation["seqNum"]]["roundConfirmed"] = validation["round"]
        
        for corruption in test["corruptWallets"]:
            corrupt_wallets.add_event(test["roundInfo"]["byzantineRound"], test_index)
        
        """
        Graphing the number of corrupt wallets:
        A wallet is defined to be corrupt if any of the following are true:
            a) it is stored by a neighborhood that is initially set to be Byzantine.
            b) it receives a coin sent by a Byzantine neighborhood which was
               previously sent by that neighborhood to someone else, and thus we
               have a confirmed proposal that moved it to someone other than the
               last known receiver
            c) it receives a coin that was sent by a Byzantine neighborhood in
               the above-described manner at some point in that coin's history
        """

        coins = {}
        """maps coin id to current owner (wallet address)"""
        wallets = {}
        """maps wallet ids to corruptedness"""

        for i in range(max_seq_num):
            prop = proposals[i]
            if prop["validators_still_needed"] <= 0:
                if (prop["coin"] in coins):
                    if (prop["sender"] == coins[prop["coin"]]):
                        coins[prop["coin"]] = prop["receiver"]
                    else:
                        coins[prop["coin"]] = -1

                        if prop["sender"] not in wallets:
                            wallets[prop["sender"]] = False

                        if prop["receiver"] not in wallets:
                            wallets[prop["receiver"]] = False

                        if not wallets[prop["sender"]]:
                            corrupt_wallets.add_event(prop["roundConfirmed"], test_index)
                            wallets[prop["sender"]] = True
                        if not wallets[prop["receiver"]]:
                            corrupt_wallets.add_event(prop["roundConfirmed"], test_index)
                            wallets[prop["receiver"]] = True

                else:
                    coins[prop["coin"]] = prop["receiver"]
                    wallets[prop["sender"]] = not prop["honest"]
                    wallets[prop["receiver"]] = not prop["honest"]

        for message in messages:
            all_messages.add_event(message["round"], test_index)
            if message["transactionType"] == "local":
                local_messages.add_event(message["round"], test_index)

    plotTOT(tx_starts, honest_tx_starts, tx_completes)
    plotMOT(local_messages, all_messages)
    plotCOT(corrupt_wallets)

    

if __name__ == "__main__":
    parser(EYEWITNESS_PATH / "LargerLog.json")
