#include "spv-wallet.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SpvWallet");
NS_OBJECT_ENSURE_REGISTERED(SpvWallet);

TypeId
SpvWallet::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SpvWallet")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<SpvWallet>();
    return tid;
}

SpvWallet::SpvWallet()
    : m_numSlots(8),
      m_nextHonestIdx(0),
      m_nextMaliciousIdx(0),
      m_currentState(S0_CONNECTED),
      m_lastStateChangeTime(0.0),
      m_eclipseCount(0),
      m_sinrThreshold(5.0)
{
    m_slots.resize(m_numSlots);
}

SpvWallet::~SpvWallet() {}

void
SpvWallet::SetNumSlots(uint32_t k)
{
    m_numSlots = k;
    m_slots.resize(k);
}

void
SpvWallet::SetKnownPeers(const std::vector<Address>& peers)
{
    m_knownPeers = peers;
}

void
SpvWallet::SetMaliciousPeers(const std::vector<Address>& maliciousPeers)
{
    m_maliciousPeers = maliciousPeers;
}

void
SpvWallet::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    m_lastStateChangeTime = Simulator::Now().GetSeconds();
    TryFillSlots();
}

void
SpvWallet::StopApplication(void)
{
    NS_LOG_FUNCTION(this);
    if (m_bootstrapEvent.IsPending()) {
        Simulator::Cancel(m_bootstrapEvent);
    }
}

void
SpvWallet::ChangeState(WalletState newState)
{
    double now = Simulator::Now().GetSeconds();
    m_timeInState[m_currentState] += (now - m_lastStateChangeTime);

    NS_LOG_INFO("Transition d'etat: " << m_currentState << " -> " << newState
                << " a t=" << now);

    m_currentState = newState;
    m_lastStateChangeTime = now;

    if (newState == S4_ECLIPSED) {
        m_eclipseCount++;
    }
    if (newState == S3_BOOTSTRAP) {
        m_bootstrapEvent = Simulator::Schedule(Seconds(1.0), &SpvWallet::TryFillSlots, this);
    }
}

void
SpvWallet::NotifySignalDegradation(double sinr)
{
    if (sinr < m_sinrThreshold && m_currentState == S0_CONNECTED) {
        ChangeState(S2_WAITING_RECONNECT);
    }
}

void
SpvWallet::NotifyHandover()
{
    if (m_currentState == S0_CONNECTED) {
        ChangeState(S1_MOBILITY);
    }
}

void
SpvWallet::NotifySignalRestored()
{
    if (m_currentState == S1_MOBILITY || m_currentState == S2_WAITING_RECONNECT) {
        ChangeState(S3_BOOTSTRAP);
    }
}

void
SpvWallet::TryFillSlots()
{
    // Strategie d'attaque : l'attaquant tente de remplir les slots en priorite
    // (monopolisation), mais uniquement avec le nombre reel de noeuds qu'il controle (r).
    // Une fois les attaquants disponibles epuises, les slots restants sont remplis
    // par des pairs honnetes.
    for (auto& slot : m_slots) {
        if (slot.isOccupied) continue;

        if (m_nextMaliciousIdx < m_maliciousPeers.size()) {
            slot.isOccupied = true;
            slot.peerAddress = m_maliciousPeers[m_nextMaliciousIdx];
            slot.isMalicious = true;
            slot.connectedSince = Simulator::Now().GetSeconds();
            m_nextMaliciousIdx++;
        } else if (m_nextHonestIdx < m_knownPeers.size()) {
            slot.isOccupied = true;
            slot.peerAddress = m_knownPeers[m_nextHonestIdx];
            slot.isMalicious = false;
            slot.connectedSince = Simulator::Now().GetSeconds();
            m_nextHonestIdx++;
        }
        // si aucun pair disponible (ni malveillant ni honnete), le slot reste vide
    }

    CheckEclipseCondition();

    if (m_currentState == S3_BOOTSTRAP) {
        bool allFilled = true;
        for (auto& s : m_slots) if (!s.isOccupied) allFilled = false;
        if (allFilled) {
            ChangeState(S0_CONNECTED);
        }
    }
}

void
SpvWallet::CheckEclipseCondition()
{
    uint32_t maliciousCount = 0;
    uint32_t occupiedCount = 0;
    for (auto& slot : m_slots) {
        if (slot.isOccupied) {
            occupiedCount++;
            if (slot.isMalicious) maliciousCount++;
        }
    }
    if (occupiedCount == m_numSlots && maliciousCount == m_numSlots) {
        ChangeState(S4_ECLIPSED);
    }
}

void
SpvWallet::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from))) {
        NS_LOG_INFO("Paquet recu de " << from);
    }
}

void
SpvWallet::FlushCurrentState()
{
    double now = Simulator::Now().GetSeconds();
    m_timeInState[m_currentState] += (now - m_lastStateChangeTime);
    m_lastStateChangeTime = now;
}

double
SpvWallet::GetTimeInState(WalletState s) const
{
    auto it = m_timeInState.find(s);
    return (it != m_timeInState.end()) ? it->second : 0.0;
}

double
SpvWallet::GetAdversarialOccupancyRate() const
{
    uint32_t maliciousCount = 0;
    for (auto& slot : m_slots) {
        if (slot.isOccupied && slot.isMalicious) maliciousCount++;
    }
    return static_cast<double>(maliciousCount) / m_numSlots;
}

void
SpvWallet::DoDispose(void)
{
    m_socket = 0;
    Application::DoDispose();
}

} // namespace ns3
