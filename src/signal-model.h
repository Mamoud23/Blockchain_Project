#ifndef SIGNAL_MODEL_H
#define SIGNAL_MODEL_H

#include "ns3/object.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"
#include "spv-wallet.h"

namespace ns3 {

// Genere des variations de SINR a intervalles aleatoires et notifie le wallet
// (couple la couche mobile/signal a la couche blockchain, cf. sujet 3.1 "Lien entre les deux couches")
class SignalModel : public Object
{
public:
    static TypeId GetTypeId(void);
    SignalModel();
    virtual ~SignalModel();

    // meanIntervalSeconds : intervalle moyen entre deux evenements de signal (loi exponentielle)
    // meanSinr / sinrStdDev : moyenne et ecart-type du SINR genere (loi normale)
    void Configure(double meanIntervalSeconds, double meanSinr, double sinrStdDev, double sinrThreshold);
    void SetWallet(Ptr<SpvWallet> wallet);

    void Start();
    void Stop();

private:
    void ScheduleNextEvent();
    void GenerateSignalEvent();

    Ptr<SpvWallet> m_wallet;
    Ptr<ExponentialRandomVariable> m_intervalRng;
    Ptr<NormalRandomVariable> m_sinrRng;

    double m_sinrThreshold;
    bool m_currentlyDegraded;
    EventId m_nextEvent;
};

} // namespace ns3

#endif // SIGNAL_MODEL_H
