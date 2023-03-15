# EyeWitness Protocol

This currently-minimal implementation of EyeWitness creates log files in this directory with filenames that end in Log.json, which the .gitignore is already configured to ignore. Logs store events that happened during program execution. They have three JSON arrays for each test: one called "transactions" which contains the data for transaction initiation events; one called "validations" which logs the corresponding validation events once for each node that commits them; and one called "messages" which logs message sending events. Computing e.g. latency data is just easier to do with a bird's eye view of everything that happened, in Python, than it is to do within Quantas, imo. Although, this does produce files that are quite large. The file plot.py in this folder can read such files and generate graphs.

Currently, multi-shard transactions pool all the nodes in all the neighborhoods involved and run a single instance of PBFT among them, updating when PBFT commits. These transactions are not split up into one between-shard and one within-shard transaction; RapidChain does this, but the only reason it seems to do this is to batch transactions more efficiently, and batching transactions or putting them into blocks together is also not implemented here.

Alternative consensus strategies:

- Use synchronous BFT for stuff like RapidChain does
- Keep nodes from different shards separated during each step in the consensus to prevent shard overlap (not currently implemented) from effectively reducing the number of validators
- Turn between-shard transactions into multiple transactions like RapidChain does

Other documentation can be found in comments in the code.

Issues that should be addressed:

- Each node stores a vector of LocalWallet objects that correspond to the wallets that their neighborhood (shard) stores. These wallets need to be made easier to find for updates with an unordered_map or something. (I don't think we need regular, ordered maps for anything)

- Each wallet stores coins and past coins; it is currently hard to find them for updates. There should be some kind of clever map that makes it easy to find coins to update their histories and potentially move them from "coins" to "past coins" in the wallets they are in. This is especially crucial for non-sender, non-receiver validator nodes because the wallet addresses that hold a "past coin" that will need to be edited history-wise when a transaction concludes are not stored in transactions.

These items roughly correspond to "TODO" comments in the code, I think. Also:

- The Python file that does graphs uses the industry-standard library matplotlib. I can't seem to install that library on the jadamek machines, so I've been running the program there and then copying the log files to my local computer to generate the graphs. This is not ideal but I do not know if it can be fixed by us; it's probably a permissions issue; the problem seems to be that a dependency of matplotlib called Pillow wants the package zlib-dev to be installed by the operating system package manager, which I cannot do.
