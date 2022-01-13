#include "ServerApp.h"

int         ServerApp::s_port           = 50000;
std::string ServerApp::s_serverCertFile = "";
std::string ServerApp::s_serverKeyFile  = "";
std::string ServerApp::s_caCertFile     = "";
std::string ServerApp::s_clientCertFile = "";
std::string ServerApp::s_clientKeyFile  = "";