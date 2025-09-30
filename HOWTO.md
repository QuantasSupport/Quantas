# QUANTAS Tutorial

This guide walks through the workflow for adding a new abstract-simulator algorithm to QUANTAS. We use the Alternating Bit Protocol (ABP) – a minimal reliable data-transfer scheme – as the concrete example. By the end you will know how to:

- Declare a peer type that derives from `quantas::Peer`.
- Register that type with `PeerRegistry` so the simulator can instantiate it from JSON.
- Implement round-by-round behaviour, parameter initialisation, and metric logging.
- Configure, build, and run an experiment that uses your new peer.
- Reuse the common runtime utilities (`RoundManager`, `LogWriter`, etc.) that most algorithms depend on.

## Alternating Bit Protocol at a Glance

ABP coordinates a sender and receiver over an unreliable channel by tagging every message with a single bit (`0` or `1`). The receiver acknowledges each data packet using the same bit. The sender resends the last packet if the acknowledgement does not arrive within a timeout. When QUANTAS runs ABP, we typically simulate:

- Two peers connected by reliable or lossy channels (configured in JSON).
- The sender submitting transactions and waiting for acknowledgements.
- Throughput and message-utility statistics collected at the end of the run.

The reference implementation lives in `quantas/AltBitPeer/AltBitPeer.cpp`, but we reconstruct it below to illustrate the moving pieces.

## Step 1 – Create a Peer Subclass

Every algorithm extends `quantas::Peer` (defined in `quantas/Common/Peer.hpp`). Your header declares the state the peer needs, along with overrides for lifecycle hooks:

```cpp
// quantas/AltBitPeer/AltBitPeer.hpp (excerpt)
class AltBitPeer : public Peer {
public:
    AltBitPeer(NetworkInterface*);
    AltBitPeer(const AltBitPeer& rhs);
    ~AltBitPeer() override;

    void performComputation() override;           // core per-round logic
    void endOfRound(std::vector<Peer*>& peers) override;
    void initParameters(const std::vector<Peer*>& peers,
                        json parameters) override;

    void sendMessage(interfaceId peer, json message);
    void submitTrans(int txId);

    int currentTransaction = 1;                   // next transaction to submit
    int requestsSatisfied = 0;                    // completed transfers
    int messagesSent = 0;                         // network usage counter
    int ns = 1;                                   // next sequence bit (1-based for readability)
    int timeoutRounds = 4;                        // resend interval
    int lastSendRound = 0;                        // round when last message left
};
```

Key points:

- Store any per-peer state as member variables.
- Expose helper functions for actions you invoke repeatedly (e.g., `sendMessage`).
- Override `initParameters` to read experiment-wide values from JSON.

## Step 2 – Register the Peer Type

`PeerRegistry` is QUANTAS’ factory for creating peers from strings in the JSON `topology.initialPeerType`. Register your type in the corresponding `.cpp` file:

```cpp
// Register for abstract simulations (NetworkInterfaceAbstract)
static bool registerAltBit = []() {
    PeerRegistry::registerPeerType(
        "AltBitPeer",
        [](interfaceId pubId) {
            return new AltBitPeer(new NetworkInterfaceAbstract(pubId));
        });
    return true;
}();
```

If you also produce a concrete-network variant, register a second symbol with `NetworkInterfaceConcrete`. The lambda executes at static initialisation time, so make sure the translation unit is compiled whenever you want the peer available (the JSON `algorithms` array drives this).

## Step 3 – Implement the Round Logic

`performComputation()` is executed once per round for each peer. A typical structure is:

1. Drain all newly arrived packets with `while (!inStreamEmpty()) { Packet pkt = popInStream(); ... }`.
2. Update local state based on the payload (`pkt.getMessage()` returns a JSON object).
3. Use the network interface helpers (`unicastTo`, `broadcast`, `broadcastBut`, `randomMulticast`) to emit messages.
4. Optionally trigger retries or timeouts when no packets arrived.

For alternating bit, the sender submits new transactions, waits for an acknowledgement with the matching message number, and resends when the timeout fires:

```cpp
void AltBitPeer::performComputation() {
    while (!inStreamEmpty()) {
        Packet packet = popInStream();
        json msg = packet.getMessage();

        if (msg["action"] == "ack" && msg["messageNum"] == ns) {
            lastSendRound = RoundManager::currentRound();
            requestsSatisfied++;
            ns++;
            submitTrans(currentTransaction);
        } else if (msg["action"] == "data") {
            // Mirror sender behaviour on the receiver side.
            lastSendRound = RoundManager::currentRound();
            ns = msg["messageNum"];
            json ack = msg;
            ack["action"] = "ack";
            sendMessage(packet.sourceId(), ack);
        }
    }

    if (messagesSent == 0 && publicId() == 0) {
            submitTrans(currentTransaction);           // boot strap sender
    } else if (lastSendRound + timeoutRounds < RoundManager::currentRound()) {
        json retry;
        retry["action"] = (publicId() == 0) ? "data" : "ack";
        retry["messageNum"] = ns;
        retry["roundSubmitted"] = RoundManager::currentRound();
        interfaceId partner = (publicId() == 0) ? 1 : 0;
        lastSendRound = RoundManager::currentRound();
        sendMessage(partner, retry);
    }
}
```

Support helpers encapsulate repetitive work:

```cpp
void AltBitPeer::sendMessage(interfaceId dest, json message) {
    ++messagesSent;
    unicastTo(message, dest);
}

void AltBitPeer::submitTrans(int txId) {
    json message;
    message["action"] = "data";
    message["messageNum"] = ns;
    message["roundSubmitted"] = RoundManager::currentRound();
    lastSendRound = RoundManager::currentRound();
    sendMessage(1, message);                       // peer 0 is the sender in this scenario
    currentTransaction++;
}
```

### Receiving Messages

- `Packet::sourceId()` returns the neighbour’s public ID.
- `Packet::getMessage()` exposes the JSON payload you stored when sending.
- Messages are automatically delayed/dropped/duplicated according to the experiment’s `distribution` parameters.

### Sending Messages

- Use `unicastTo(payload, targetId)` to send to a specific neighbour.
- `broadcast`, `broadcastBut`, and `multicast` are available for wider dissemination.
- QUANTAS transparently handles channel queuing and delivery semantics based on `distribution`.

## Step 4 – Initialise Parameters and Log Metrics

`initParameters` runs once after the peers are created. Forward experiment-level configuration to each peer, and compute any derived state:

```cpp
void AltBitPeer::initParameters(const std::vector<Peer*>& peers, json parameters) {
    const int timeout = parameters.value("timeOutRate", RoundManager::lastRound());
    for (auto* peer : reinterpret_cast<const std::vector<AltBitPeer*>&>(peers)) {
        peer->timeoutRounds = timeout;
    }
}
```

At the end of a round, you can aggregate statistics across all peers and push them into `LogWriter`. Values are emitted when the experiment finishes.

```cpp
void AltBitPeer::endOfRound(std::vector<Peer*>& peers) {
    if (RoundManager::currentRound() >= RoundManager::lastRound()) {
        double throughput = 0.0;
        double networkCost = 0.0;
        for (const auto* peer : reinterpret_cast<const std::vector<AltBitPeer*>&>(peers)) {
            throughput += peer->requestsSatisfied;
            networkCost += peer->messagesSent;
        }
        const double utility = (networkCost > 0.0) ? (throughput / networkCost) * 100.0 : 0.0;
        LogWriter::pushValue("utility", utility);
        LogWriter::pushValue("messages", networkCost);
        LogWriter::pushValue("throughput", throughput);
    }
}
```

## Step 5 – Describe the Experiment in JSON

Experiment files live under `quantas/<Algorithm>/`. A minimal configuration for ABP looks like `quantas/AltBitPeer/AltBitUtility.json`:

```json
{
  "algorithms": [
    "AltBitPeer/AltBitPeer.cpp"
  ],
  "experiments": [
    {
      "logFile": "AltBitStats.txt",
      "threadCount": 1,
      "distribution": {
        "type": "UNIFORM",
        "minDelay": 1,
        "maxDelay": 3,
        "dropProbability": 0.0,
        "duplicateProbability": 0.0,
        "reorderProbability": 0.0,
        "maxMsgsRec": 1
      },
      "topology": {
        "type": "chain",
        "initialPeers": 2,
        "initialPeerType": "AltBitPeer"
      },
      "parameters": {
        "timeOutRate": 4
      },
      "tests": 1,
      "rounds": 200
    }
  ]
}
```

Notes:

- `algorithms` lists the translation units that register peer types (the build system compiles exactly these).
- `distribution` applies channel behaviour globally; rate-limiting (`maxMsgsRec`) now lives here, not under `topology`.
- `topology.initialPeerType` must align with the string you registered in Step 2.
- `parameters` is passed to `initParameters`; nest additional structures as needed.

Update the root `makefile` (or override at invocation) to point to your JSON file:

```sh
make run INPUTFILE=quantas/AltBitPeer/AltBitUtility.json
```

## Step 6 – Build and Run

```sh
make release                     # or make debug for asserts and instrumentation
make run                          # executes every experiment listed in the JSON
```

Metrics appear either on stdout (`"logFile": "cout"`) or in the named log file. For the sample above, expect final entries similar to:

```json
{
  "RunTime": 0.012345,
  "tests": [
    {
      "throughput": 180.0,
      "messages": 240.0,
      "utility": 75.0
    }
  ]
}
```

## Runtime Utilities Cheat Sheet

Most QUANTAS algorithms reuse a collection of helper classes. Understanding them keeps your code concise and consistent with the rest of the repository.

- **`RoundManager` (`quantas/Common/RoundManager.hpp`)**
  - `RoundManager::currentRound()` – current synchronous round (1-based).
  - `RoundManager::lastRound()` – last round configured for the experiment.
  - `RoundManager::incrementRound()` – advanced internally by the simulator at the end of each round.
  - Use it for timeouts, phased logic, and statistics keyed by round numbers.

- **`LogWriter` (`quantas/Common/LogWriter.hpp`)**
  - `LogWriter::setLogFile(file)` – set per-experiment destination (`"cout"` or filename).
  - `LogWriter::pushValue(key, value)` – queue a metric for the current round/test.
  - `LogWriter::print()` – emit accumulated metrics (called automatically when a test ends).

- **`NetworkInterface` & messaging helpers**
  - The abstract simulator wires peers together using `NetworkInterfaceAbstract` and `Channel` objects.
  - `unicastTo(msg, neighbourId)` – send directly to a specific neighbour.
  - `broadcast(msg)` / `multicast(msg, targets)` / `broadcastBut(msg, excluded)` – higher-level fan-out options.
  - Channels respect the configured `distribution` (delay, drop, duplicate, reorder, queue size).

- **`Packet` (`quantas/Common/Packet.hpp`)**
  - Encapsulates a single in-flight message.
  - `Packet::getMessage()` – returns the JSON payload.
  - `Packet::sourceId()` / `Packet::targetId()` – neighbour identifiers for bookkeeping.

- **`PeerRegistry` (`quantas/Common/Peer.hpp`)**
  - Registers string identifiers to factory lambdas.
  - Ensures your peer type can be instantiated when parsing `topology.initialPeerType`.

- **`FaultManager` & Byzantine support (`quantas/Common/ByzantinePeer.hpp`, `quantas/Common/Faults.hpp`)**
  - Extend `ByzantinePeer` instead of `Peer` when you want hookable fault injection.
  - Attach `Fault` subclasses (e.g., `EquivocateFault`, `ParasiteFault`) for adversarial scenarios.

- **Thread pool (`quantas/Common/BS_thread_pool.hpp`)**
  - The simulator runs receive/computation phases in parallel according to the experiment’s `threadCount`.
  - Your peer code should remain thread-safe with respect to its own state; QUANTAS calls `performComputation()` serially per peer.

## Where to Go Next

- Browse `quantas/ExamplePeer/` for a lightweight template with multiple behaviours in one executable.
- Inspect other algorithm folders (e.g., `KademliaPeer`, `PBFTPeer`, `RaftPeer`) to learn how larger protocols organise their state machines, logging, and fault injection.
- Combine the concepts above to add your own protocol: derive from `Peer`, register the type, implement the per-round logic, and describe the experiment in JSON.

Happy simulating!
