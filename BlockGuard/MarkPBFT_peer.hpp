#ifndef markPBFT_message_HPP
#define markPBFT_message_HPP

#include <string>
#include "Common/Peer.hpp"
#include <map>
#include <set>
#include <list>

static const std::string idle           = "IDLE";
static const std::string preprepare     = "PREPREPARE";
static const std::string prepare        = "PREPARE";
static const std::string commit         = "COMMIT";
static const std::string reply          = "REPLY";
static const std::string view_change    = "VIEW_CHANGE";
static const std::string request        = "REQUEST";

struct markPBFT_message {
	std::string         client_id;
	// this is the peer that created the message
	std::string         creator_id;
	int				    requestGoal;
	int					creator_shard;
	int                 view;
	std::string         type;
	char                operation;
	std::pair<int, int> operands;
	int                 result;
	int                 round; // used instead of timestamp
	std::string         highestID;
	//////////////////////////////////////////
	// phases info
	std::string         phase;
	int                 sequenceNumber;

	bool operator==(const markPBFT_message& rhs);
};

class markPBFT_peer : public Peer<markPBFT_message> {
private:
	markPBFT_peer() {}

	bool                                                            _isPrimary;
	std::string                                                     _state; // planned delete
	int                                                             _messageID;
    int                                                             _roundCount;
	int                                                             _prepareCount; // planned delete
	int                                                             _commitCount; // planned delete
	std::string                                                     _highmsgID;
    std::map<std::string,int>                                       _replyCount;
	int                                                             _requestPerRound;
	int                                                             _roundsToRequest;
	int                                                             _remainingRoundstoRequest;
	double                                                          _faultTolerance;
	int                                                             _shard;
	int                                                             _requestQueue;
	int                                                             _shardCount;
	int                                                             _neighborShard;

	// For view Change
	int                                                             _viewCounter;
	int                                                             _maxWait;
	bool                                                            _voteChange;
	int                                                             _waitcommit;

	// Message logs;
	std::map<std::string, std::map<std::string, int>>               _receivedMsgLog; // map<msgID,map<messagetype, messageCount>>
	std::set<std::string>                                           _preprepareSent;
	std::set<std::string>                                           _prepareSent;
	std::set<std::string>                                           _commitSent;
	std::set<std::string>                                           _replySent;
	std::map<std::string, int>                                      _ledger; // First value is messageID, second is delay to receive
	std::map < std::string, std::map<std::string, std::set<int>>>   _msgShardCount; // map<msgID,map<messageType,list<shards>>>

public:
    markPBFT_peer(std::string id) : Peer<markPBFT_message>(id), _isPrimary(false), _faultTolerance(0.3), _messageID(0), _viewCounter(0),
                                    _voteChange(false), _state(idle), _roundCount(0), _prepareCount(0), _commitCount(0), _requestPerRound(1), _maxWait(0), _waitcommit(0),
                                    _roundsToRequest(1), _remainingRoundstoRequest(1), _requestQueue(0), _neighborShard(0)
    {
        //_printNeighborhood = true;
    }
                                    ~markPBFT_peer      ()                          {}

    // getters
    int                             getShardCount       () const                    { return _shardCount; }
    int                             getRequestCount     () const                    { return _requestQueue; }
    int                             getNeighborShard    () const                    { return _neighborShard; }
    int                             getShard            () const                    { return _shard; };
    bool                            getVote             () const                    { return _voteChange; }
    int                             getMaxWait          () const                    { return _maxWait;}
    bool                            isPrimary           () const                    { return _isPrimary; }

    // setters
    void                            setNeighborShard    (int a)                     { _neighborShard = a; }
    void                            setShardCount       (int a)                     { _shardCount = a; }
    void                            setShard            (int shardNum)              { _shard=shardNum; }
    void                            setMaxWait          ();
    void                            setRequestPerRound  (int a)                     { if (a > 0) _requestPerRound = a; }
    void                            setRoundsToRequest  (int a);
    void                            setFaultTolerance   (double setting)            { _faultTolerance = setting; }
    void                            setPrimary          (bool status);
    std::set<std::string>&          requests            ()                          { return _preprepareSent; }
    std::map<std::string, int>&     ledger              ()                          { return _ledger; }

    // mutators
	void                            resetVote           ()                          { _voteChange = false; _viewCounter = 0; }

    // base class functions
    void                            makeRequest         ();
    void                            preformComputation  ()                          {}
    void                            makeRequest         (markPBFT_message);

    // loging
    std::ostream&                   printTo             (std::ostream& out) const;
};


#endif //MarkPBFT_peer_HPP