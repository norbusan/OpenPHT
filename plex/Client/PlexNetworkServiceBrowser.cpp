#include "PlexNetworkServiceBrowser.h"
#include "PlexApplication.h"
#include "PlexMediaServerClient.h"
#include "Network/NetworkInterface.h"
#include "StringUtils.h"
#include "Application.h"

#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
void CPlexNetworkServiceBrowser::handleServiceArrival(NetworkServicePtr& service)
{
  CPlexServerPtr server = CPlexServerPtr(new CPlexServer(service->getResourceIdentifier(), service->getParam("Name"), true));

  int port = 32400;
  try
  {
    port = boost::lexical_cast<int>(service->getParam("Port"));
  }
  catch (...)
  {
    eprintf("CPlexNetworkServiceBrowser::handleServiceArrival failed to get port?");
  }

  std::string address = service->address().to_string();

  if (address != "127.0.0.1" && NetworkInterface::IsLocalAddress(address))
  {
    CLog::Log(LOGDEBUG,
              "CPlexNetworkServiceBrowser::handleServiceArrival won't add server %s",
              address.c_str());
    return;
  }

  // Secure connections are only available if the GDM Host header is set
  // if the Host contains the servers UUID and if we are signed into plex.tv
  //
  string uri = service->getParam("Host");
  CPlexConnectionPtr conn;
  if (!uri.empty())
  {
    string addr(address);
    StringUtils::Replace(addr, ".", "-");

    CURL u;
    u.SetHostName(addr + "." + uri);
    u.SetProtocol("https");
    u.SetPort(port);

    CLog::Log(LOGDEBUG, "CPlexNetworkServiceBrowser::handleServiceArrival adding SSL connection: %s", u.Get().c_str());
    conn = CPlexConnectionPtr(new CPlexConnection(CPlexConnection::CONNECTION_DISCOVERED, u.GetHostName(), u.GetPort(), u.GetProtocol()));
    server->AddConnection(conn);
  }
  else
  {
    conn = CPlexConnectionPtr(new CPlexConnection(CPlexConnection::CONNECTION_DISCOVERED, address, port));
    server->AddConnection(conn);
  }

  if (conn->TestReachability(server) == CPlexConnection::CONNECTION_STATE_REACHABLE)
    server->SetActiveConnection(conn);

  g_plexApplication.serverManager->UpdateFromDiscovery(server);

  if (!server || server->GetUUID().empty())
    return;

  CSingleLock lk(m_serversSection);
  m_discoveredServers[server->GetUUID()] = server;
  dprintf("CPlexNetworkServiceBrowser::handleServiceArrival %s arrived",
          service->address().to_string().c_str());
  g_plexApplication.timer->RestartTimeout(5000, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CPlexNetworkServiceBrowser::handleServiceDeparture(NetworkServicePtr& service)
{
  CLog::Log(LOGDEBUG,
            "CPlexNetworkServiceBrowser::handleServiceDeparture departing with server %s last seen %f",
            service->getResourceIdentifier().c_str(), service->timeSinceLastSeen());

  CSingleLock lk(m_serversSection);
  /* Remove the server from m_discoveredServers and then tell ServerManager to update it's state */
  if (m_discoveredServers.find(service->getResourceIdentifier()) != m_discoveredServers.end())
    m_discoveredServers.erase(service->getResourceIdentifier());

  PlexServerList list;
  BOOST_FOREACH(PlexServerPair p, m_discoveredServers)
  list.push_back(p.second);

  CLog::Log(LOGDEBUG,
            "CPlexNetworkServiceBrowser::handleServiceDeparture we have %lu servers from GDM",
            list.size());
  g_plexApplication.serverManager->UpdateFromConnectionType(list,
                                                            CPlexConnection::CONNECTION_DISCOVERED);
  g_plexApplication.timer->RestartTimeout(5000, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CPlexNetworkServiceBrowser::handleNetworkChange(const vector<NetworkInterface>& interfaces)
{
  if (g_application.m_bStop) return;

  NetworkServiceBrowser::handleNetworkChange(interfaces);

  bool validInterface = false;
  BOOST_FOREACH(const NetworkInterface& xface, interfaces)
  {
    if (!xface.loopback() && !boost::starts_with(xface.address(), "169.254."))
    {
      validInterface = true;
      break;
    }
  }

  if (validInterface)
  {
    // update all our reachability states
    g_plexApplication.serverManager->UpdateReachability(true);

    // and refresh myPlex
    g_plexApplication.myPlexManager->Refresh();

    // publish our device to plex
    g_plexApplication.mediaServerClient->publishDevice();
  }
  else
  {
    // TODO: Clear all active connections and best server
  }

  g_plexApplication.timer->RestartTimeout(5000, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CPlexNetworkServiceBrowser::OnTimeout()
{
  CSingleLock lk(m_serversSection);
  PlexServerList list;
  BOOST_FOREACH(PlexServerPair p, m_discoveredServers)
  list.push_back(p.second);

  CLog::Log(LOGDEBUG, "CPlexNetworkServiceBrowser::OnTimeout reporting %ld discovered servers",
            list.size());

  g_plexApplication.serverManager->UpdateFromConnectionType(list,
                                                            CPlexConnection::CONNECTION_DISCOVERED);

  // run it every 5th minute even if there are no changes
  g_plexApplication.timer->RestartTimeout(5 * 60 * 1000, this);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CPlexServiceListener::Process()
{
  dprintf("CPlexServiceListener: Initializing.");

  // We start watching for changes in here.
  NetworkInterface::WatchForChanges();

  // Server browser.
  m_pmsBrowser = NetworkServiceBrowserPtr(new CPlexNetworkServiceBrowser(m_ioService, NS_PLEX_MEDIA_SERVER_PORT));

  // start our reporting timer
  g_plexApplication.timer->SetTimeout(5000, (CPlexNetworkServiceBrowser*)m_pmsBrowser.get());

  // Player
  StartAdvertisement();

  // Start the I/O service in its own thread.
  m_ioService.run();
}
