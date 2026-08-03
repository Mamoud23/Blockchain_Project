#include "signal-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SignalModel");
NS_OBJECT_ENSURE_REGISTERED(SignalModel);

TypeId
SignalModel::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SignalModel")
        .SetParent<Object>()
        .SetGroupName("Applications")
        .AddConstructor<SignalModel>();
    return tid;
}

SignalModel::SignalModel()
    : m_sinrThreshold(5.0),
      m_currentlyDegraded(false)
{
    m_intervalRng = CreateObject<ExponentialRandomVariable>();
    m_sinrRng = CreateObject<NormalRandomVariable>();
}

SignalModel::~SignalModel() {}

void
SignalModel::Configure(double meanIntervalSeconds, double meanSinr, double sinrStdDev, double sinrThreshold)
{
    m_intervalRng->SetAttribute("Mean", DoubleValue(meanIntervalSeconds));
    m_sinrRng->SetAttribute("Mean", DoubleValue(meanSinr));
    m_sinrRng->SetAttribute("Variance", DoubleValue(sinrStdDev * sinrStdDev));
    m_sinrThreshold = sinrThreshold;
}

void
SignalModel::SetWallet(Ptr<SpvWallet> wallet)
{
    m_wallet = wallet;
}

void
SignalModel::Start()
{
    ScheduleNextEvent();
}

void
SignalModel::Stop()
{
    if (m_nextEvent.IsPending()) {
        Simulator::Cancel(m_nextEvent);
    }
}

void
SignalModel::ScheduleNextEvent()
{
    double interval = m_intervalRng->GetValue();
    m_nextEvent = Simulator::Schedule(Seconds(interval), &SignalModel::GenerateSignalEvent, this);
}

void
SignalModel::GenerateSignalEvent()
{
    double sinr = m_sinrRng->GetValue();
    NS_LOG_INFO("Evenement signal genere: SINR=" << sinr << " dB a t="
                << Simulator::Now().GetSeconds());

    if (m_wallet) {
        if (sinr < m_sinrThreshold && !m_currentlyDegraded) {
            m_wallet->NotifySignalDegradation(sinr);
            m_currentlyDegraded = true;
        } else if (sinr >= m_sinrThreshold && m_currentlyDegraded) {
            m_wallet->NotifySignalRestored();
            m_currentlyDegraded = false;
        }
    }

    ScheduleNextEvent();
}

} // namespace ns3
