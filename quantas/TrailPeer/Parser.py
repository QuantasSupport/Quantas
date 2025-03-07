import json
from pathlib import Path
import matplotlib.pyplot as plt
from collections import defaultdict
from time import time

TRAIL_PATH = Path(__file__).parent
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

    def __init__(self, lowest_timestamp: int, highest_timestamp: int, normalization_factor: int=1):
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
        self.normalization_factor = normalization_factor
        self.events_by_round_cache = None

    def add_event(self, time: int, timeline_id: int=0, num_times: int=1) -> None:
        """
        time: integer timestamp between lowest and highest timestamps
        timeline_id: id of the timeline the event belongs to
        """
        assert self.min <= time < self.max
        self.events_by_round_cache = None
        self.timelines[timeline_id][time] += num_times
        if num_times == 0:
            pass  # this is weird though right

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

    def get_average_cumulative_timeline(self, normalize: bool=False) -> Timeline:
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
            average_timeline[i] = 0 if sum == 0 else (
                sum /
                len(self.timelines.keys()) /
                (self.normalization_factor if normalize else 1)
            )
        return average_timeline


TRAIL_PATH = Path(__file__).parent
GRAPH_TIMESTAMPS = round(time())

def plotTOT(tx_starts: EventTimelineMuxer, honest_tx_starts: EventTimelineMuxer, tx_completes: EventTimelineMuxer,
            outfile: str, inset_centered_on: int=-1):

    tx_starts_avg_tl = tx_starts.get_average_cumulative_timeline()
    htx_starts_avg_tl = honest_tx_starts.get_average_cumulative_timeline()
    tx_completes_avg_tl = tx_completes.get_average_cumulative_timeline()
    
    def plot(axes):
        axes.plot(tx_starts_avg_tl.keys(), tx_starts_avg_tl.values(),
                color="black", label="transactions started")
        axes.plot(htx_starts_avg_tl.keys(), htx_starts_avg_tl.values(),
                color="black", linestyle="dashed", label="honest transactions started")
        axes.plot(tx_completes_avg_tl.keys(), tx_completes_avg_tl.values(),
                color="black", linestyle="dotted", label="transactions confirmed")
    plot(plt)

    plt.xlabel("rounds")
    plt.ylabel("transactions")

    if inset_centered_on != -1:
        axin = plt.gca().inset_axes([0.55, 0.02, 0.43, 0.43])
        plot(axin)
        axin.set_xlim(inset_centered_on-35, inset_centered_on+35)
        axin.set_ylim(tx_starts_avg_tl[inset_centered_on-35], 
                      tx_starts_avg_tl[inset_centered_on+35])
        plt.gca().indicate_inset_zoom(axin)

        # Hide X and Y axes label marks
        axin.xaxis.set_tick_params(labelbottom=False)
        axin.yaxis.set_tick_params(labelleft=False)

        # Hide X and Y axes tick marks
        axin.set_xticks([])
        axin.set_yticks([])

    plt.legend(loc="upper left")
    plt.savefig(TRAIL_PATH / outfile)
    plt.show()
    plt.clf()

def plotMOT(local_messages: EventTimelineMuxer, all_messages: EventTimelineMuxer, outfile: str, normalize: bool = False):
    plt.title("Messages Over Time")
    local_messages_avg_tl = local_messages.get_average_cumulative_timeline(normalize)
    plt.plot(local_messages_avg_tl.keys(), local_messages_avg_tl.values(),
             color="blue", label="Messages Sent for Local Transactions")
    all_messages_avg_tl = all_messages.get_average_cumulative_timeline(normalize)
    plt.plot(all_messages_avg_tl.keys(), all_messages_avg_tl.values(),
             color="green", label="Messages Sent for All Transactions")
    if normalize:
        plt.ylabel("Messages normalized by number of peers")
    else:
        plt.ylabel("Total # of Messages")
    plt.legend(loc="upper left")
    plt.tight_layout()
    plt.savefig(TRAIL_PATH / outfile)
    plt.show()
    plt.clf()

def plotCOT(
        bft: EventTimelineMuxer, 
        noBFT: EventTimelineMuxer, 
        recovery: EventTimelineMuxer, 
        outfile: str):
    make_percents = lambda x: x*100
    no_bft_avg_tl = noBFT.get_average_cumulative_timeline(True)
    plt.plot(no_bft_avg_tl.keys(), list(map(make_percents, no_bft_avg_tl.values())),
            color="black", label="without $TRAIL$")
    
    bft_avg_tl = bft.get_average_cumulative_timeline(True)
    plt.plot(bft_avg_tl.keys(), list(map(make_percents, bft_avg_tl.values())),
             color="black", linestyle="dashed", label="with $TRAIL$")
    
    rec_avg_tl = recovery.get_average_cumulative_timeline(True)
    plt.plot(rec_avg_tl.keys(), list(map(make_percents, rec_avg_tl.values())), 
            color="black", linestyle="dotted", label="with $TRAIL$ and failed shard recovery")

    plt.ylabel("compromised wallets, %")
    plt.xlabel("rounds")
    plt.legend(loc="upper left")
    plt.savefig(TRAIL_PATH / outfile)
    plt.show()
    plt.clf()

def plot_coins_lost(lost_coins: EventTimelineMuxer, outfile: str):
    plt.title("Lost Coins Over Time")
    lost_coins_avg_tl = lost_coins.get_average_cumulative_timeline()
    plt.plot(lost_coins_avg_tl.keys(), lost_coins_avg_tl.values(),
             color="blue")
    if False:  # normalize later
        plt.ylabel("Fraction of Total Wallets")
    else:
        plt.ylabel("Total # of Coins")
    plt.savefig(TRAIL_PATH / outfile)
    plt.show()
    plt.clf()


def plot_throughput_vs_committees():
    line_styles = [
        {"color": "black"},
        {"color": "black", "linestyle": "dashed"},
        {"color": "black", "linestyle": "dotted"}
    ]
    for F in range(0, 3):
        log_path = TRAIL_PATH / "varyingCommsLogs" / f"F{F}"
        log_files = list(log_path.glob("*.json"))
        xs = [int(x.name[0:x.name.index("C")]) for x in log_files]
        used_xs = []
        ys = []

        for i, log in enumerate(log_files):
            try:
                print("reading", log)
                ys.append(max(parser(log)["tx_completes"].get_average_cumulative_timeline().values()))
                used_xs.append(xs[i])
            except json.decoder.JSONDecodeError:
                # this error is Probably for an empty file while the program is running
                print("could not read", log)

        # sort both arrays based on x values
        used_xs, ys = zip(*sorted(zip(used_xs, ys), key=lambda t: t[0]))
        neighborhood_size = 13
        rounds = 200

        plt.plot(
            [x*neighborhood_size for x in used_xs],
            [y/rounds for y in ys],
            **line_styles[F], marker='o', label=f"$F={F}$"
        )

    plt.xlabel("number of peers in the network")
    plt.ylabel("transaction rate per round")
    plt.legend(loc="best")
    plt.savefig(TRAIL_PATH / "throughput_scaling.png")
    plt.clf()


def plot_throughput_vs_committee_size():
    log_path = TRAIL_PATH / "varyingCommSizesLogs"
    log_files = list(log_path.glob("*.json"))
    xs = [int(x.name[0:x.name.index("P")]) for x in log_files]
    used_xs = []
    ys = []

    for i, log in enumerate(log_files):
        try:
            ys.append(max(parser(log)["tx_completes"].get_average_cumulative_timeline().values()))
            used_xs.append(xs[i])
            print("read", log)
        except json.decoder.JSONDecodeError:
            # this error is Probably for an empty file while the program is running
            print("could not read", log)

    # sort both arrays based on x values
    used_xs, ys = zip(*sorted(zip(used_xs, ys), key=lambda t: t[0]))
    
    plt.title("Throughput vs Committee Size")
    plt.xlabel("Committee Size With 2000 Peers")
    plt.ylabel("Transactions Processed in 200 Rounds")
    plt.plot(used_xs, ys, linestyle='--', marker='o')
    plt.savefig(TRAIL_PATH / "throughput_comm_size.png")
    plt.clf()


def plotFT():
    NUM_FT = 5
    NUM_SHARDS = 29
    NUM_PEERS = 203
    NUM_ROUNDS = 100
    local_messages = {}
    all_messages = {}
    tx_starts = {}
    tx_completes = {}
    for i in range(1,NUM_FT+1):
        output = parser(TRAIL_PATH / f"FTLog{i}.json")

        local_messages_avg_tl = output["local_messages"].get_average_cumulative_timeline()
        local_messages[i] = local_messages_avg_tl[output["local_messages"].max - 1] / NUM_PEERS / NUM_ROUNDS

        all_messages_avg_tl = output["all_messages"].get_average_cumulative_timeline()
        all_messages[i] = all_messages_avg_tl[output["all_messages"].max - 1] / NUM_PEERS / NUM_ROUNDS

        tx_starts_avg_tl = output["tx_starts"].get_average_cumulative_timeline()
        tx_starts[i] = tx_starts_avg_tl[output["all_messages"].max - 1] / NUM_SHARDS

        tx_completes_avg_tl = output["tx_completes"].get_average_cumulative_timeline()
        tx_completes[i] = tx_completes_avg_tl[output["all_messages"].max - 1] / NUM_SHARDS


    plt.title("Messages wrt F")
    plt.plot(local_messages.keys(), local_messages.values(),
             color="blue", label="Messages Sent for Local Transactions")
    plt.plot(all_messages.keys(), all_messages.values(),
             color="green", label="Messages Sent for All Transactions")
    plt.legend(loc="center right")
    plt.savefig(TRAIL_PATH / f"graph_FTmessages_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

    plt.title("Transactions wrt F")

    plt.plot(tx_starts.keys(), tx_starts.values(),
             color="blue", label="Transactions started")
    plt.plot(tx_completes.keys(), tx_completes.values(),
             color="green", label="Transactions finished")
    plt.legend(loc="upper left")
    plt.savefig(TRAIL_PATH / f"graph_FTtransactions_{GRAPH_TIMESTAMPS}.png")
    plt.show()
    plt.clf()

def plot_malicious_effects(outfile: str, normalize: bool=False):
    MALICIOUS_RANGE = range(1, 6)
    corrupt_at_end: dict[int, int] = {}
    corrupt_at_end_bft: dict[int, int] = {}
    for i in MALICIOUS_RANGE:
        output = parser(TRAIL_PATH / F"ML{i}Log.json")
        avg_corrupt = output["corrupt_wallets"].get_average_cumulative_timeline(normalize)
        corrupt_at_end[i] = avg_corrupt[output["corrupt_wallets"].max-1]
        bft_output = parser(TRAIL_PATH / F"ML{i}ValidatedLog.json")
        avg_corrupt_bft = bft_output["corrupt_wallets"].get_average_cumulative_timeline(normalize)
        corrupt_at_end_bft[i] = avg_corrupt_bft[bft_output["corrupt_wallets"].max-1]

    plt.title("Corrupt Wallets Due to # Malicious Neighborhoods")
    fig, ax = plt.subplots()
    bar_width = 0.35
    ax.bar(
        corrupt_at_end.keys(),
        corrupt_at_end.values(),
        bar_width,
        label="without supercommittee validation",
        color="blue"
    )
    ax.bar(
        [x+bar_width for x in corrupt_at_end_bft.keys()],
        corrupt_at_end_bft.values(),
        bar_width,
        label="with supercommittee validation",
        color="green"
    )
    plt.legend(loc="upper left")
    plt.xlabel("Malicious Neighborhoods")
    ax.set_xticklabels(corrupt_at_end.keys())
    plt.xticks([x+bar_width/2 for x in corrupt_at_end.keys()])
    if normalize:
        plt.ylabel("Fraction of Corrupt Wallets at End")
    else:
        plt.ylabel("# of Corrupt Wallets at End")
    plt.savefig(TRAIL_PATH / outfile)
    plt.show()
    plt.clf()

def parser(logfile: str) -> dict[str, EventTimelineMuxer]:

    with open(logfile) as logfileobj:
        log = json.load(logfileobj)

    num_rounds = log["tests"][0]["roundInfo"]["roundCount"]
    num_peers = log["tests"][0]["peerInfo"]["peerCount"]
    num_wallets = log["tests"][0]["walletInfo"]["walletCount"]

    tx_starts = EventTimelineMuxer(0, num_rounds)
    honest_tx_starts = EventTimelineMuxer(0, num_rounds)
    tx_completes = EventTimelineMuxer(0, num_rounds)

    local_messages = EventTimelineMuxer(0, num_rounds, num_peers)
    all_messages = EventTimelineMuxer(0, num_rounds, num_peers)

    corrupt_wallets = EventTimelineMuxer(0, num_rounds, num_wallets)

    coins_lost = EventTimelineMuxer(0, num_rounds)

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
        rollback_coin_ids = set()
        rollbacks_for_wallets: defaultdict[int, int] = defaultdict(lambda: 0)

        max_seq_num = -1
        for transaction in transactions:
            if (transaction["seqNum"] > max_seq_num):
                max_seq_num = transaction["seqNum"]

            if "rollback" in transaction and transaction["rollback"]:
                rollback_coin_ids.add(transaction["coin"])
                rollbacks_for_wallets[transaction["sender"]] += 1

            tx_starts.add_event(transaction["round"], test_index)
            if transaction["honest"]:
                honest_tx_starts.add_event(transaction["round"], test_index)
            proposals[transaction["seqNum"]] = (
                {
                    "validatorsStillNeeded": tolerance_fraction*transaction["validatorsNeeded"],
                    **transaction
                }
            )

        lost_coins = set()
        if "lostCoins" in test:
            for c in test["lostCoins"]:
                lost_coins.add(c)
                coins_lost.add_event(test["roundInfo"]["byzantineRound"], test_index)

        for validation in validations:
            proposal = proposals[validation["seqNum"]]
            if proposal["validatorsStillNeeded"] > 0:
                proposal["validatorsStillNeeded"] -= 1
                if proposal["validatorsStillNeeded"] <= 0:
                    tx_completes.add_event(validation["round"], test_index)
                    proposal["roundConfirmed"] = validation["round"]
                    if proposal["rollback"]:
                        coins_lost.add_event(validation["round"], test_index, -1)
                        rollbacks_for_wallets[proposal["sender"]] -= 1
                        if rollbacks_for_wallets[proposal["sender"]] == 0:
                            corrupt_wallets.add_event(validation["round"], test_index, -1)

        if "corruptWallets" in test:
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

            corrupt_ids = set()
            double_spent_coins = set()
        
            byzantine_round = test["roundInfo"]["byzantineRound"]

            for wallet_id in test["corruptWallets"]:
                corrupt_wallets.add_event(byzantine_round, test_index)
                corrupt_ids.add(wallet_id)

            for i in range(max_seq_num):
                # for every transaction proposal, in sequential (chronological) order:
                prop = proposals[i]
                if prop["validatorsStillNeeded"] <= 0:  # if the proposal was confirmed
                    if not prop["honest"] or prop["coin"] in double_spent_coins:
                        double_spent_coins.add(prop["coin"])
                        if prop["receiver"] not in corrupt_ids:
                            corrupt_ids.add(prop["receiver"])
                            corrupt_wallets.add_event(prop["roundConfirmed"], test_index)

        for message in messages:
            all_messages.add_event(message["round"], test_index, message["batchSize"])
            if message["transactionType"] == "local":
                local_messages.add_event(message["round"], test_index, message["batchSize"])

    output = {}
    output["tx_starts"] = tx_starts
    output["honest_tx_starts"] = honest_tx_starts
    output["tx_completes"] = tx_completes
    output["local_messages"] = local_messages
    output["all_messages"] = all_messages
    output["corrupt_wallets"] = corrupt_wallets
    output["coins_lost"] = coins_lost
    return output

def make_timing_diagrams(logfile) -> EventTimelineMuxer:
    output = parser(logfile)
    plotTOT(output["tx_starts"], output["honest_tx_starts"],
            output["tx_completes"],
            f"log {Path(logfile).stem}_txs_{GRAPH_TIMESTAMPS}.png",
            110 if "Rollback" in str(logfile) else 140
    )
    # plotMOT(output["local_messages"], output["all_messages"], f"log {Path(logfile).stem}_msgs_normalized_{GRAPH_TIMESTAMPS}.png", normalize)
    return output

if __name__ == "__main__":
    bft = make_timing_diagrams(TRAIL_PATH / "LargerLog.json")
    noBFT = make_timing_diagrams(TRAIL_PATH / "LargerNoBFTLog.json")
    recovery = make_timing_diagrams(TRAIL_PATH / "RollbackLog.json")
    plotCOT(
        bft["corrupt_wallets"], noBFT["corrupt_wallets"], recovery["corrupt_wallets"],
        f"log wlt_normalized_{GRAPH_TIMESTAMPS}.png"
    )

    # plotFT()

    # plot_malicious_effects(f"graph_maliciousness_{GRAPH_TIMESTAMPS}.png")
    # plot_malicious_effects(f"normalized_graph_maliciousness_{GRAPH_TIMESTAMPS}.png", True)

    plot_throughput_vs_committees()
    # plot_throughput_vs_committee_size()
