import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict
from time import time

EYEWITNESS_PATH = Path(__file__).parent
GRAPH_TIMESTAMPS = round(time())

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

def plotTOT(tx_starts: EventTimelineMuxer, tx_completes: EventTimelineMuxer):
    plt.title("Transactions Over Time")
    tx_starts_avg_tl = tx_starts.get_average_cumulative_timeline()
    plt.plot(tx_starts_avg_tl.keys(), tx_starts_avg_tl.values(),
             color="blue", label="Transactions started")
    tx_completes_avg_tl = tx_completes.get_average_cumulative_timeline()
    plt.plot(tx_completes_avg_tl.keys(), tx_completes_avg_tl.values(),
             color="green", label="Transactions finished")
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

# hard coded parameter 10 i.e. 1-9 fault tolerant shards
def plotFT():
    NUM_FT = 10
    local_messages = {}
    all_messages = {}
    tx_starts = {}
    tx_completes = {}
    for i in range(1,NUM_FT):
        output = parser(EYEWITNESS_PATH / f"LogFT{i}.json")

        local_messages_avg_tl = output["local_messages"].get_average_cumulative_timeline()
        local_messages[i] = local_messages_avg_tl[output["local_messages"].max - 1]

        all_messages_avg_tl = output["all_messages"].get_average_cumulative_timeline()
        all_messages[i] = all_messages_avg_tl[output["all_messages"].max - 1]

        tx_starts_avg_tl = output["tx_starts"].get_average_cumulative_timeline()
        tx_starts[i] = tx_starts_avg_tl[output["all_messages"].max - 1]

        tx_completes_avg_tl = output["tx_completes"].get_average_cumulative_timeline()
        tx_completes[i] = tx_completes_avg_tl[output["all_messages"].max - 1]


    plt.title("Messages per # fault tolerant committees")
    plt.plot(local_messages.keys(), local_messages.values(),
             color="blue", label="Messages Sent for Local Transactions")
    plt.plot(all_messages.keys(), all_messages.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="center right")
    plt.savefig(EYEWITNESS_PATH / f"graph_FTmessages_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

    plt.title("Transactions per # fault tolerant committees")
    
    plt.plot(tx_starts.keys(), tx_starts.values(),
             color="blue", label="Transactions started")
    plt.plot(tx_completes.keys(), tx_completes.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.savefig(EYEWITNESS_PATH / f"graph_FTtransactions_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

def parser(logfile: str):

    with open(logfile) as logfileobj:
        log = json.load(logfileobj)
    
    # todo: include how many rounds are being used in log and then retrieve it
    # below instead of having a hard-coded constant
    NUM_ROUNDS = log["tests"][0]["roundInfo"]["rounds"] + 1

    # graph 1:
    tx_starts = EventTimelineMuxer(0, NUM_ROUNDS)
    tx_completes = EventTimelineMuxer(0, NUM_ROUNDS)

    # graph 2:
    local_messages = EventTimelineMuxer(0, NUM_ROUNDS)
    all_messages = EventTimelineMuxer(0, NUM_ROUNDS)

    # graph 3:
    corrupt_wallets = EventTimelineMuxer(0, NUM_ROUNDS)

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
            proposals[transaction["seqNum"]] = (
                {"validators_still_needed": tolerance_fraction*transaction["validatorCount"],
                "coin": transaction["coin"],
                "receiver": transaction["receiver"],
                "sender": transaction["sender"]
                }
            )
        
        for validation in validations:
            if proposals[validation["seqNum"]]["validators_still_needed"] > 0:
                proposals[validation["seqNum"]]["validators_still_needed"] -= 1
                if proposals[validation["seqNum"]]["validators_still_needed"] <= 0:
                    tx_completes.add_event(validation["round"], test_index)
                    proposals[validation["seqNum"]]["roundConfirmed"] = validation["round"]
        
        if "corruptWallets" in test:
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
                if proposals[i]["validators_still_needed"] <= 0:
                    if (proposals[i]["coin"] in coins):
                        if (proposals[i]["sender"] == coins[proposals[i]["coin"]]):
                            coins[proposals[i]["coin"]] = proposals[i]["receiver"]
                        else:
                            coins[proposals[i]["coin"]] = -1

                            if proposals[i]["sender"] not in wallets:
                                wallets[proposals[i]["sender"]] = False

                            if proposals[i]["receiver"] not in wallets:
                                wallets[proposals[i]["receiver"]] = False

                            if not wallets[proposals[i]["sender"]]:
                                corrupt_wallets.add_event(proposals[i]["roundConfirmed"], test_index)
                                wallets[proposals[i]["sender"]] = True
                            if not wallets[proposals[i]["receiver"]]:
                                corrupt_wallets.add_event(proposals[i]["roundConfirmed"], test_index)
                                wallets[proposals[i]["receiver"]] = True

                    else:
                        coins[proposals[i]["coin"]] = proposals[i]["receiver"]
                        wallets[proposals[i]["sender"]] = False
                        wallets[proposals[i]["receiver"]] = False


        

        for message in messages:
            all_messages.add_event(message["round"], test_index)
            if message["transactionType"] == "local":
                local_messages.add_event(message["round"], test_index)
    output = {"tx_starts": tx_starts, "tx_completes": tx_completes, "local_messages": local_messages, "all_messages": all_messages, "corrupt_wallets": corrupt_wallets}
    return output

    

if __name__ == "__main__":
    plotFT()
    # output = parser(EYEWITNESS_PATH / "LargerLog.json")
    # plotTOT(output.tx_starts, output.tx_completes)
    # plotMOT(output.local_messages, output.all_messages)
    # plotCOT(output.corrupt_wallets)
