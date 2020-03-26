//
// HAPWebServer.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPWEBSERVER_HPP_
#define HAPWEBSERVER_HPP_

#include <Arduino.h>


#include "HAPGlobals.hpp"

#include "EventManager.h"

// Inlcudes for setting up the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <ResourceNode.hpp>

#include "HAPWebServerTemplateProcessor.hpp"

#include "HAPAccessorySet.hpp"
#include "HAPLogger.hpp"
#include "HAPConfig.hpp"
#include "HAPPlugins.hpp"

#if HAP_API_USE_JWT
#include "HAPWebServerJWT.hpp"
#endif


// Easier access to the classes of the server
using namespace httpsserver;

class HAPWebServer {
public:	
	HAPWebServer();
	~HAPWebServer();

	static bool begin();

	static inline void setEventManager(EventManager* eventManager){
		_eventManager = eventManager;
	}

	static void handle();

	static void setAccessorySet(HAPAccessorySet* accessorySet);
	static void setConfig(HAPConfig *config);	
	// static void registerPluginNode(const std::string name, const std::string path, const std::string method, const HTTPSCallbackFunction * callback, const std::string tag = "");

	static void registerPluginNode(HAPWebServerPluginNode* pluginNode);

	static String buildNavigation(bool full = true);


private:

	static std::vector<HAPWebServerPluginNode*> _pluginNodes;
	
	static std::vector<std::string> splitString(std::string data, std::string token);
	//SSLCert* _serverCert;

#if HAP_WEBSERVER_USE_SSL	
	static HTTPSServer* _secureServer;
#else
	static HTTPServer* _secureServer;
#endif
	static HAPAccessorySet* _accessorySet;
	static HAPConfig* _config;

	uint16_t _port;



	// ====================================================================================================
	// Websites
	// ====================================================================================================	
	// /
	static void handleRoot(HTTPRequest * req, HTTPResponse * res);
	static void rootKeyProcessor(const String& key, HTTPResponse* res);

	// /config
	static void handleConfigGet(HTTPRequest * req, HTTPResponse * res);	
	static void configGetKeyProcessor(const String& key, HTTPResponse* res);
	static void handleConfigPost(HTTPRequest * req, HTTPResponse * res);	
	
	// /wifi
	static void handleWifiGet(HTTPRequest * req, HTTPResponse * res);	
	static void handleWifiPost(HTTPRequest * req, HTTPResponse * res);	

	// /update
	static void handleUpdateGet(HTTPRequest * req, HTTPResponse * res);
	static void handleUpdatePost(HTTPRequest * req, HTTPResponse * res);
	static void updateKeyProcessor(const String& key, HTTPResponse* res);

	// -> remove
	static void handleLogin(HTTPRequest * req, HTTPResponse * res);	

	// 404
	static void handle404(HTTPRequest * req, HTTPResponse * res);

	// /plugin/*
	static void handlePluginNodes(HTTPRequest * req, HTTPResponse * res);		
	static bool validatePluginNodesGet(std::string s);
	static bool validatePluginNodesPost(std::string s);

	// ====================================================================================================
	// API
	// ====================================================================================================
	// /api/*
	static void handleApi(HTTPRequest * req, HTTPResponse * res);
	
	// validators
	static bool validateApiGet(const std::string s);
	static bool validateApiPost(const std::string s);
	static bool validateApiDelete(const std::string s);

	// GET /api/heap
	static void handleApiHeap(HTTPRequest * req, HTTPResponse * res);


	// GET /api/setup
	static void handleApiSetup(HTTPRequest * req, HTTPResponse * res);	// ? delete ?	


	// GET /api/uptime
	static void handleApiUptime(HTTPRequest *req, HTTPResponse *res);

	// GET /api/reftime
	static void handleApiRefTimeGet(HTTPRequest *req, HTTPResponse *res);
	// POST /api/reftime
	static void handleApiRefTimePost(HTTPRequest *req, HTTPResponse *res);
	
	
	// GET /api/restart
	static void handleApiRestart(HTTPRequest * req, HTTPResponse * res);

	// POST /api/reset
	static void handleApiReset(HTTPRequest *req, HTTPResponse *res);

	// POST /api/config
	static void handleApiConfigPost(HTTPRequest * req, HTTPResponse * res);	
	// GET /api/config
	static void handleApiConfigGet(HTTPRequest *req, HTTPResponse *res);
	
	
	// DELETE /api/pairings
	static void handleApiPairingsDelete(HTTPRequest *req, HTTPResponse *res);


	// ====================================================================================================
	// Middleware
	// ====================================================================================================

	// Declare a middleware function.
	// Parameters:
	// req: Request data, can be used to access URL, HTTP Method, Headers, ...
	// res: Response data, can be used to access HTTP Status, Headers, ...
	// next: This function is used to pass control down the chain. If you have done your work
	//       with the request object, you may decide if you want to process the request.
	//       If you do so, you call the next() function, and the next middleware function (if
	//       there is any) or the actual requestHandler will be called.
	//       If you want to skip the request, you do not call next, and set for example status
	//       code 403 on the response to show that the user is not allowed to access a specific
	//       resource.
	//       For more details, see the definition below.
	static void middlewareLogging(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
	
	static void middlewareBasicAuthentication(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
	static void middlewareBasicAuthorization(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);

#if HAP_WEBSERVER_USE_JWT
	static void middlewareJWTAuthorization(HTTPRequest * req, HTTPResponse * res, std::function<void()> next);
#endif

	static EventManager* _eventManager;

};

#endif /* HAPWEBSERVER_HPP_ */
