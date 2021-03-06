/*
 * POWNode.h
 *
 *  Created on: Nov 14, 2018
 *      Author: jburke
 */

#ifndef POWNODE_H_
#define POWNODE_H_

#include <map>
#include <omnetpp.h>
#include "P2PNode.h"
#include "messages/pow_message_m.h"
#include "messages/scheduler_message_m.h"
#include "MessageGenerator.h"
#include "pow_node_data.h"
#include "addr_manager.h"
#include "blockchain/blockchain.h"
#include "blockchain/tx.h"
#include <functional>
#include <memory>
#include <queue>

using namespace omnetpp;

struct POWNodeState {
    bool syncStarted;
    int numSyncs;
    int bestPeerHeight;
    std::vector<Transaction> verifiedTransactions;
    std::vector<Transaction> unverifiedTransactions;
    std::vector<BlockHeader> blocksToAnnounce;
    // maps a hash of a transaction to a map of (int,int), where each key is the index of the transaction output and each value is the amount left
    // of the corresponding transaction output
    std::map<int64_t, std::map<int,int>> outputsSpent;

    POWNodeState() : syncStarted(false), numSyncs(0), bestPeerHeight(-1) {
    }
};

/*! Node in a proof of work based blockchain network (i.e., Bitcoin, Litecoin, etc).
 *
 */
// TODO: set some nodes to only accept incoming connections, instead of attempting to connect to all known nodes at beginning
class POWNode: public P2PNode {
public:
    POWNode();
    virtual ~POWNode();
protected:
    /*! Initialize the node.  Occurs during the set up stage of the simulation, before any messages are sent.
     * Runs the following steps:
     * 1.  Establish connections with nodes in known node list (if no known nodes, just connect to a list of default nodes).
     * 2.  Broadcast our known nodes list (excluding default nodes)
     */
    virtual void initialize() override;

    /*! Process an incoming message.  Message gets added to incomingMessages queue, which gets processed at a user-specified
     * time interval.
     * \param msg Message to handle.  Will be cast to a POWMessage.
     */
    virtual void handleMessage(cMessage *msg) override;

    virtual void refreshDisplay() const override;

    /*! Check if the node is online (would be handled by TCP timeouts in real network).  Connections will not be established with an offline node.
     * \returns True if the node is online and can be connected to, false otherwise.
     */
    bool isOnline() const;

    /*! Map the node index to the outgoing gate to be used to send messages over.
     * \param nodeIndex The intended destination index for messages
     * \param gate The outgoing gate to send messages over.
     * \param inboundValue True if the node marked by nodeIndex is initiating the connection
     */
    void addNodeToGateMapping(int nodeIndex, cGate *gate, bool inboundValue);
private:
    /*! Internal initialzation steps, done before anything network related.
     * These involve loading data files, connecting message handlers, etc.
     */
    void internalInitialize();

    void initBlockchain();

    /*! Read addresses from data/<index>.dat
     *
     */
    void readAddresses();

    /*! One time initial scheduling of self messages (message handler, dump addresses, etc)
     *
     */
    void scheduleSelfMessages();

    /*! Read NED parameters that will not change during the simulation.
     *
     */
    void readConstantParameters();

    /*! Create connections to currently known list of networks (step 1 of initialization).  Each connection will create a new gate in the source and destination nodes if needed.
     * In the future, the max number of connections may be a parameter in the network simulation (as the actual Bitcoin client has such a parameter).
     */
    void initConnections();

    /*! Connect message handlers using the name to member function map.
     *
     */
    void setupMessageHandlers();

    /*! Initiate the appropriate "thread" for the given self message.
     * \param msg Message indicating what thread to start.
     */
    void handleSelfMessage(POWMessage *msg);

    /*! Send given message to all currently known peers that the given predicate is true for.  Copies the message for each one.
     * \param broadcast Message to broadcast.
     * \param predicate Optional predicate used to filter peers.  Defaults to a predicate that returns true for all peers.
     */
    void broadcastMessage(POWMessage *broadcast, std::function<bool(int)> predicate = [](int peer) { return true; });

    /*! Handle an incoming node version message.  If the node's version is less than our minimum acceptable
     * protocol version, send a reject message in response, otherwise, send a verack message in response.
     * \param msg Message to handle.
     */
    void handleNodeVersionMessage(POWMessage *msg);

    void handleScheduledMessage(SchedulerMessage *msg);

    /*! Handle an incoming verack message.  Set state of connection to accept incoming blockchain messages.
     * \param msg Message to handle.
     */
    void handleVerackMessage(POWMessage *msg);

    /*! Handle an incoming reject message.  Disconnect from the sender if the data indicates so.
     * \param msg Message to handle.  Contains data that indicates the ban's reason and whether it should result in a disconnect.
     */
    void handleRejectMessage(POWMessage *msg);

    /*! Handle an incoming get addr message.  Push our known addresses into the corresponding node's
     * addresses queue.
     * \param msg Message to handle.  Does not contain any data.
     */
    void handleGetAddrMessage(POWMessage *msg);

    /*! Handle an incoming address advertisement.
     * \param msg Message to handle.  Contains a set of addresses
     */
    void handleAddrMessage(POWMessage *msg);

    void handleGetHeadersMessage(POWMessage *msg);

    void handleHeadersMessage(POWMessage *msg);

    void handleTxMessage(POWMessage *msg);

    void handleGetBlocksMessage(POWMessage *msg);

    void handleBlocksMessage(POWMessage *msg);

    /*! Handler for half of address polling interface.  Handles receiving addresses from a peer.
     * \param msg Message to handle.  Contains a set of addresses that we asked for.
     */
    void handleAddrsMessage(POWMessage *msg);

    /*! Print information upon receiving a message.
     * \param msg Message to log.
     */
    void logReceivedMessage(POWMessage *msg) const;

    /*! Convenience method for sending a message to the specified node.
     * \param msg Message to send
     * \param nodeIndex Index of node to send to.
     */
    void sendToNode(POWMessage *msg, int nodeIndex);

    /*! "Thread" that checks peers' incoming and outgoing message queues.
     * Calls processIncomingMessages and sendOutgoingMessages for each peer.
     * \param msg Message that initiated the check.  Not used (only here to work with the handler map)
     */
    void messageHandler(POWMessage *msg);

    void handleNewBlock(SchedulerMessage *msg);

    void handleNewTx(SchedulerMessage *msg);

    /*! Self-scheduled message for nodes marked as miners.
     * Steps:
     * Collect broadcasted transactions and ensure they follow our policy
     * Verify the transactions to be included in the block
     * Select block tip of the longest path in the blockchain, and insert the hash of its block header into the new block
     * Solve proof of work problem
     */
    void mineHandler(POWMessage *msg);

    /*! Dump addresses.  Called at a specified interval.
     * \param msg Message that initiated the address dump.
     */
    void dumpAddresses(POWMessage *msg);

    /*! "Thread" that advertises address to the peer specified by the message.
     * \param msg Message carrying index of peer to advertise addresses to.
     */
    void advertiseAddresses(POWMessage *msg);

    /*! Process a message from a queue.
     * Calls the message's appropriate handler, if one exists.
     */
    void processMessage(POWMessage *msg);

    /*! Ask currently connected peers for their known addresses
     *
     */
    void pollAddresses(POWMessage *msg);

    /*! Process the given peer's incoming messages.
     * \param peerIndex Index of peer to process messages of.
     * \returns True if there is still more processing to be done for the peer, false otherwise.
     */
    bool processIncomingMessages(int peerIndex);

    /*! Process data to be sent to the specified peer.  Data can include addresses, inventory, block headers, blocks, etc.
     * \param peerIndex Index of peer to send data to.
     */
    void sendOutgoingMessages(int peerIndex);

    void sendBroadcasts();

    void startBlockSync(int peerTo);

    /*! Disconnect from the specified node.
     * \param nodeIndex Index of node to disconnect
     */
    void disconnectNode(int nodeIndex);

    /*! Part of node loop run post initialization.
     * Sends any necessary data to peers.
     */
    void sendMessages();

    /*! Checks if the given message is being received at an appropriate time.
     * \param msg Message to check.
     * \returns True if the message is in scope, false otherwise.
     */
    bool checkMessageInScope(POWMessage *msg);

    /*! Create a connection with the specified node.
     * \param otherIndex index of the peer.  Used to map the gates in the connection.
     * \param other node representing the peer.  Used to make sure data stored and the connection are symmetric.
     */
    void connectTo(int otherIndex, POWNode *other);

    /*! Add connections to addresses received from an ADDRS message.
     * \param newAddresses Vector containing addresses of new peers.
     */
    void dynamicConnect(const std::vector<int> &newAddresses);

    /*! Schedule an address advertisement to the given peer.  The advertisement is poisson-distributed according to lambda = threadScheduleInterval.
     * \param peerIndex index of peer to send ad to.
     */
    void scheduleAddrAd(int peerIndex);

    /*! Send the address to a random subselection of peers.
     * \param address Address to relay.
     */
    void relayAddress(int address);

    void updateOutputsSpent();

    POWNode *getPeerNodeByPath(int address) {
        std::string nodePath = "node[" + std::to_string(address) + "]";
        return check_and_cast<POWNode*>(getModuleByPath(nodePath.c_str()));
    }

    std::map<std::string, std::function<void(POWNode &, POWMessage *)> > messageHandlers;
    // these are kept separate because self messages are processed immediately
    std::map<std::string, std::function<void(POWNode &, POWMessage *)> > selfMessageHandlers;
    std::map<std::string, std::function<void(POWNode &, SchedulerMessage *)> > simulationScheduleHandlers;

    std::unique_ptr<Blockchain> blockchain;

    std::map<int, cGate*> nodeIndexToGateMap;
    int versionNumber;
    int minAcceptedVersionNumber;
    int maxMessageProcess;
    int addrSendInterval;
    int maxAddrAd;
    int numAddrRelay;
    int addrRelayVecSize;
    int dumpAddressesInterval;
    int blocksPerFile;
    bool isMiner;
    int blockSyncRecency;
    double randomAddressFraction;
    int coinbaseOutput;
    bool newNetwork;
    int stopAddrPollingTime;
    std::vector<int> defaultNodes;
    std::unique_ptr<MessageGenerator> messageGen;
    // maintain data known about each peer
    std::map<int, std::unique_ptr<POWNodeData> > peers;
    std::unique_ptr<AddrManager> addrMan;
    std::queue<int> peersProcess; // make sure nodes are processed fairly
    int threadScheduleInterval;
    int currentMessagesProcessed;  // counter for number of messages that have been processed in one loop
    std::string addressesFile;
    std::string blocksDir;
    std::string dataDir;
    POWNodeState state;
    int chainHeight;
    int coins;
};

Define_Module(POWNode)

#endif /* POWNODE_H_ */
