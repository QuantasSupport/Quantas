import numpy as np
import matplotlib.pyplot as plt
from functools import reduce
from math import sqrt
def factors(n):
        step = 2 if n%2 else 1
        return set(reduce(list.__add__,
                    ([i, n//i] for i in range(1, int(sqrt(n))+1, step) if n % i == 0)))

# Variables
tests = 100
peers = 1600
tolerances = [0, 3, 6] # number of byzantine shards tolerable
detectRate = [0, 3, 6] # number of rounds to detect and remove a byzantine shard
lineTypes = ['-', '--', ':']
# num_shards = np.array([1, 5, 10, 50, 100, 250, 1000])
# num_shards = np.array(sorted(list(x for x in factors(1200) if x % 6 == 0)))[:-3]
# num_shards = np.array(range(1, 250, 10))
# num_shards = np.array([6, 12, 24, 30, 48, 60, 120, 150, 240])
# num_shards = np.empty(0)
# for x in range(2,101):
#     num_shards = np.append(num_shards, ((x*3) + 1))
num_shards = np.array([25, 40, 64, 100, 160])

shard_sizes = peers / num_shards
limit = np.floor(shard_sizes / 3)
print(num_shards)
print(shard_sizes)
print(limit)

# plt.title("Mean Time To Failure") # no titles
plt.xlabel(f"number of shards")
plt.ylabel("mean time to failure, rounds")

num = 0
# Determine time to failure for different values of F
for F in tolerances:
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
                while byz_peers[shard] > shard_sizes[j]:
                    # pick a differnt shard to corrupt a member of if everyone
                    # in the previously chosen one is already corrupt
                    shard = np.random.randint(num_shards[j])
                byz_peers[shard] = byz_peers[shard] + 1
                round = round + 1
            rounds[j][i] = round

    # Calculate the mean and std
    mean = np.average(rounds,1)
    std = np.std(rounds,1)
    print("F="+str(F))
    print(mean)
    print(std)
    print()
    plt.plot(num_shards[start_index:], mean[start_index:], linestyle=lineTypes[num],
            marker="o", color='black', label="$\it{F}$="+str(F))
    num = num + 1

plt.legend(loc="best")
plt.savefig("./MTTF.png")


for F in tolerances:
    plt.figure()
    # Plot #2 with rollbacks
    # plt.title("Mean Time To Failure Tolerance " + str(F)) # no titles
    plt.xlabel(f"number of shards")
    plt.ylabel("mean time to failure, rounds")
    num = 0
    # Determine time to failure for different detect rates
    for dr in detectRate:
        rounds = np.zeros([len(num_shards), tests])
        # get the index of the first number of shards that makes sense for the
        # cross-shard verification algorithm (because, like, you need at least 4
        # shards to be able to tolerate F=1 failures, for example)
        start_index = next(i for (i, v) in enumerate(num_shards) if v >= 3*F+1)

        for i in range(tests):
            for j in range(start_index, len(num_shards)):
                byz_peers = np.zeros(num_shards[j])
                detection = np.zeros(num_shards[j])
                removal = np.zeros(num_shards[j])
                round = 0
                while np.sum(byz_peers > limit[j]) < F+1 and len(byz_peers) > 3*F + 1:
                    shard = np.random.randint(len(byz_peers))
                    while byz_peers[shard] >= shard_sizes[j] and np.sum(byz_peers[shard] >= shard_sizes[j]) > len(byz_peers):
                        # pick a differnt shard to corrupt a member of if everyone
                        # in the previously chosen one is already corrupt
                        shard = np.random.randint(len(byz_peers))
                    byz_peers[shard] = byz_peers[shard] + 1
                    round = round + 1
                    if (byz_peers[shard] > limit[j]):
                        detection[shard] = 1
                        removal[shard] = dr
                    k = 0
                    while k < len(byz_peers):
                        if detection[k] == 1:
                            if removal[k] == 0:
                                byz_peers = np.delete(byz_peers, k)
                                detection = np.delete(detection, k)
                                removal = np.delete(removal, k)
                                k = k - 1
                            else:
                                removal[k] = removal[k] - 1
                        k = k + 1
                rounds[j][i] = round

        # Calculate the mean and std
        mean = np.average(rounds,1)
        std = np.std(rounds,1)
        print("F="+str(F) + " d=" + str(dr))
        print(mean)
        print(std)
        print()
        plt.plot(num_shards[start_index:], mean[start_index:], linestyle=lineTypes[num],
                marker="o", color='black', label="$\it{d}$="+str(dr))
        num = num + 1

    plt.legend(loc="best")
    plt.savefig("./MTTF_rollbacks_toll" + str(F) + ".png")