#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "spv-wallet.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EclipseSimWired");

int
main(int argc, char *argv[])
{
    uint32_t numHonestNodes = 10;
    uint32_t numAttackerNodes = 8;
    uint32_t numSlots = 8;
    double simulationTime = 60.0;

    CommandLine cmd;
    cmd.AddValue("numHonest", "Nombre de noeuds honnetes", numHonestNodes);
    cmd.AddValue("numAttackers", "Nombre de noeuds attaquants", numAttackerNodes);
    cmd.AddValue("simTime", "Duree de la simulation (s)", simulationTime);
    cmd.Parse(argc, argv);

    LogComponentEnable("SpvWallet", LOG_LEVEL_INFO);
    LogComponentEnable("EclipseSimWired", LOG_LEVEL_INFO);

    NodeContainer victimNode;
    victimNode.Create(1);

    NodeContainer honestNodes;
    honestNodes.Create(numHonestNodes);

    NodeContainer attackerNodes;
    attackerNodes.Create(numAttackerNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper internet;
    internet.Install(victimNode);
    internet.Install(honestNodes);
    internet.Install(attackerNodes);

    Ipv4AddressHelper address;
    std::vector<Ipv4InterfaceContainer> honestInterfaces;
    std::vector<Ipv4InterfaceContainer> attackerInterfaces;

    for (uint32_t i = 0; i < numHonestNodes; ++i)
    {
        NetDeviceContainer link = p2p.Install(victimNode.Get(0), honestNodes.Get(i));
        std::ostringstream subnet;
        subnet << "10.1." << i << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        Ipv4InterfaceContainer iface = address.Assign(link);
        honestInterfaces.push_back(iface);
    }

    for (uint32_t i = 0; i < numAttackerNodes; ++i)
    {
        NetDeviceContainer link = p2p.Install(victimNode.Get(0), attackerNodes.Get(i));
        std::ostringstream subnet;
        subnet << "10.2." << i << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");
        Ipv4InterfaceContainer iface = address.Assign(link);
        attackerInterfaces.push_back(iface);
    }

    std::vector<Address> knownPeers;
    std::vector<Address> maliciousPeers;

    for (auto &iface : honestInterfaces)
    {
        knownPeers.push_back(InetSocketAddress(iface.GetAddress(1), 8333));
    }
    for (auto &iface : attackerInterfaces)
    {
        Address addr = InetSocketAddress(iface.GetAddress(1), 8333);
        maliciousPeers.push_back(addr);
    }

    Ptr<SpvWallet> wallet = CreateObject<SpvWallet>();
    wallet->SetNumSlots(numSlots);
    wallet->SetKnownPeers(knownPeers);
    wallet->SetMaliciousPeers(maliciousPeers);
    victimNode.Get(0)->AddApplication(wallet);
    wallet->SetStartTime(Seconds(1.0));
    wallet->SetStopTime(Seconds(simulationTime));

    Simulator::Stop(Seconds(simulationTime + 1.0));
    Simulator::Run();

    wallet->FlushCurrentState();

    NS_LOG_INFO("=== Resultats de la simulation ===");
    NS_LOG_INFO("Nombre d'eclipses detectees: " << wallet->GetEclipseCount());
    NS_LOG_INFO("Taux d'occupation adverse: " << wallet->GetAdversarialOccupancyRate());
    NS_LOG_INFO("Temps en S0 (Connecte): " << wallet->GetTimeInState(S0_CONNECTED));
    NS_LOG_INFO("Temps en S4 (Eclipse): " << wallet->GetTimeInState(S4_ECLIPSED));

    Simulator::Destroy();
    return 0;
}
