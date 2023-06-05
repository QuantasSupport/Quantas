import numpy as np
import matplotlib.pyplot as plt

# Variables
tests = 100
peers = 1000
# num_shards = np.array([1, 5, 10, 50, 100, 250, 1000])
num_shards = np.array([1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100, 125, 200, 250])
# num_shards = np.array(range(1, 250, 10))

plt.title("Mean Time To Failure")
plt.xlabel(f"Number of Committees for {peers} Peers")
plt.ylabel("Mean Failure Time in Rounds")

# Determine time to failure for different values of F
for F in [0, 3, 6]:
    shard_sizes = peers / num_shards
    limit = np.floor(shard_sizes / 3)
    rounds = np.zeros([len(num_shards), tests])
    # get the index of the first number of shards that makes sense for the
    # cross-shard verification algorithm (because, like, you need at least 4
    # shards to be able to tolerate F=1 failures, for example)
    start_index = next(i for (i, v) in enumerate(num_shards) if v >= 3*F+1)

    for i in range(tests):        
        for j in range(start_index, len(num_shards)):
            byz_peers = np.zeros(num_shards[j])
            round = 0
            while np.sum(byz_peers > limit[j]) < F+1:
                shard = np.random.randint(num_shards[j])
                byz_peers[shard] = byz_peers[shard] + 1
                round = round + 1
            rounds[j][i] = round

    # Calculate the mean and std
    mean = np.average(rounds,1)
    # std = np.std(rounds,1)
    print("F="+str(F))
    print(mean)
    # print(std)
    print()
    plt.plot(num_shards[start_index:], mean[start_index:], linestyle="--",
            marker="o", label="F="+str(F) if F != 0 else "Without Eyewitness")

plt.legend(loc="best")
plt.savefig("./MTTF.png")
