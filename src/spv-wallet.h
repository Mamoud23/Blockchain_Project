#ifndef SPV_WALLET_H
#define SPV_WALLET_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include <vector>
#include <map>

namespace ns3 {

enum WalletState {
    S0_CONNECTED = 0,
    S1_MOBILITY,
    S2_WAITING_RECONNECT,
    S3_BOOTSTRAP,
    S4_ECLIPSED
};

struct ConnectionSlot {
    Address peerAddress;
    bool isOccupied = false;
    bool isMalicious = false;
    double connectedSince = 0.0;
};

class SpvWallet : public Application
{
public:
    static TypeId GetTypeId(void);
    SpvWallet();
    virtual ~SpvWallet();

    void SetNumSlots(uint32_t k);
    void SetKnownPeers(const std::vector<Address>& peers);
    void SetMaliciousPeers(const std::vector<Address>& maliciousPeers);

    void NotifySignalDegradation(double sinr);
    void NotifyHandover();
    void NotifySignalRestored();

    void FlushCurrentState();
    double GetTimeInState(WalletState s) const;
    uint32_t GetEclipseCount() const { return m_eclipseCount; }
    double GetAdversarialOccupancyRate() const;

protected:
    virtual void DoDispose(void);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void HandleRead(Ptr<Socket> socket);
    void TryFillSlots();
    void ChangeState(WalletState newState);
    void CheckEclipseCondition();

    Ptr<Socket> m_socket;
    std::vector<ConnectionSlot> m_slots;
    std::vector<Address> m_knownPeers;
    std::vector<Address> m_maliciousPeers;
    uint32_t m_numSlots;
    uint32_t m_nextHonestIdx;
    uint32_t m_nextMaliciousIdx;

    WalletState m_currentState;
    double m_lastStateChangeTime;
    std::map<WalletState, double> m_timeInState;

    uint32_t m_eclipseCount;
    EventId m_bootstrapEvent;
    double m_sinrThreshold;
};

} // namespace ns3

#endif // SPV_WALLET_H
