#ifndef markPBFT_message_HPP
#define markPBFT_message_HPP

#include <string>
#include "Common/Peer.hpp"
#include <map>
#include <set>
#include <list>

static const std::string idle = "IDLE";
static const std::string preprepare = "PREPREPARE";
static const std::string prepare = "PREPARE";
static const std::string commit = "COMMIT";
static const std::string reply = "REPLY";
static const std::string view_change = "VIEW_CHANGE";
static const std::string request = "REQUEST";

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
	bool _isPrimary;
	std::string _state; // planned delete
	int _messageID;
	int _roundCount;
	int _prepareCount; // planned delete
	int _commitCount; // planned delete
	std::string _highmsgID;
    std::map<std::string,int> _replyCount;
	int _requestPerRound;
	int _roundsToRequest;
	int _remainingRoundstoRequest;
	double _faultTolerance;
	int _shard;
	int _requestQueue;
	int _shardCount;
	int _neighborShard;

	// For view Change

	int _viewCounter;
	int _maxWait;
	bool _voteChange;
	int _waitcommit;



	// Message logs;
	
	std::map<std::string, std::map<std::string, int>> _receivedMsgLog; // map<msgID,map<messagetype, messageCount>>
	std::set<std::string> _preprepareSent;
	std::set<std::string> _prepareSent;
	std::set<std::string> _commitSent;
	std::set<std::string> _replySent;
	std::map<std::string, int> _ledger; // First value is messageID, second is delay to receive
	std::map < std::string, std::map<std::string, std::set<int>>> _msgShardCount; // map<msgID,map<messageType,list<shards>>>

public:
	void setNeighborShard(int a) { _neighborShard = a; }
	int getNeighborShard() { return _neighborShard; }
	void setShardCount(int a) { _shardCount = a; }
	int getShardCount() { return _shardCount; }
	int getRequestCount() { return _requestQueue; }
	void setShard(int shardNum) { _shard=shardNum; }
	int getShard() const { return _shard; };
	bool getVote() { return _voteChange; }
	void resetVote() { _voteChange = false; _viewCounter = 0; }
	void setMaxWait();

	int getMaxWait() { return _maxWait;}
	std::set<std::string>& getRequests() { return _preprepareSent; }

	std::map<std::string, int>& getLedger() { return _ledger; }
	void setRequestPerRound(int a) { if (a > 0) _requestPerRound = a; }
	void setRoundsToRequest(int a);

	markPBFT_peer(std::string id) : Peer<markPBFT_message>(id), _isPrimary(false), _faultTolerance(0.3), _messageID(0), _viewCounter(0), 
		_voteChange(false), _state(idle), _roundCount(0), _prepareCount(0), _commitCount(0), _requestPerRound(1), _maxWait(0), _waitcommit(0),
		_roundsToRequest(1), _remainingRoundstoRequest(1), _requestQueue(0), _neighborShard(0)
	{ 
		//_printNeighborhood = true;
	}

	void setFaultTolerance(double setting) { _faultTolerance = setting; }

	std::ostream& printTo(std::ostream& out) const;

	bool isPrimary() const { return _isPrimary; }
	void setPrimary(bool status) ;
	void makeRequest() ;
	void preformComputation() {}
	void makeRequest(markPBFT_message);

	~markPBFT_peer() {}
};


#endif //MarkPBFT_peer_HPP