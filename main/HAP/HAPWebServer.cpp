//
// HAPWebServer.cpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#include <ArduinoJson.h>


#include "HAPWebServer.hpp"
#include "HAPServer.hpp"
#include "HAPDeviceID.hpp"
#include "HAPHelper.hpp"
#include "HAPPairings.hpp"
#include "HAPWebServerBodyParserMultipart.hpp"
#include "HAPSVG.hpp"

#if HAP_WEBSERVER_USE_SSL
// Certs + Key
// ToDo: KEy not needed -> remove ?
extern const unsigned char server_cert_der_start[] asm("_binary_server_cert_der_start");
extern const unsigned char server_cert_der_end[] asm("_binary_server_cert_der_end");

extern const unsigned char server_privateKey_der_start[] asm("_binary_server_privatekey_der_start");
extern const unsigned char server_privateKey_der_end[] asm("_binary_server_privatekey_der_end");

extern const unsigned char server_publicKey_der_start[] asm("_binary_server_publickey_der_start");
extern const unsigned char server_publicKey_der_end[] asm("_binary_server_publickey_der_end");
#endif

#include "HAPWebServerFiles.hpp"


/*
#include "HAPFlash.hpp"
#include "HAPWebsites.hpp"
 */

#include <mbedtls/version.h>
#include <sodium.h>

// Include certificate data (see note above)
// #include "../cert.h"
// #include "../private_key.h"

// We define two new HTTP-Header names. Those headers will be used internally
// to store the user name and group after authentication. If the client provides
// these headers, they will be ignored to prevent authentication bypass.
#define HEADER_USERNAME "X-USERNAME"
#define HEADER_GROUP "X-GROUP"




// index.html
// extern const unsigned char html_index_start[] asm("_binary_index_html_start");
// extern const unsigned char html_index_end[] asm("_binary_index_html_end");


#if HAP_WEBSERVER_USE_SSL
HTTPSServer* HAPWebServer::_secureServer;
#else
HTTPServer* HAPWebServer::_secureServer;
#endif

HAPAccessorySet* HAPWebServer::_accessorySet;
HAPConfig* HAPWebServer::_config;

std::vector<HAPWebServerPluginNode*> HAPWebServer::_pluginNodes;

HAPWebServer::HAPWebServer() {}

HAPWebServer::~HAPWebServer()
{
    delete[] _secureServer;
}

bool HAPWebServer::begin()
{

    // SPIFFS.begin();



#if HAP_WEBSERVER_USE_SSL
    SSLCert cert = SSLCert(
        (unsigned char *)server_cert_der_start, server_cert_der_end - server_cert_der_start,
        (unsigned char *)server_privateKey_der_start, server_privateKey_der_end - server_privateKey_der_start);
    _secureServer = new HTTPSServer(&cert);
#else
    _secureServer = new HTTPServer();
#endif

    // For every resource available on the server, we need to create a ResourceNode
    // The ResourceNode links URL and HTTP method to a handler function
    
    // ======================================================================================
    // /
    // ====================================================================================== 
    ResourceNode *nodeRoot = new ResourceNode("/", "GET", &HAPWebServer::handleRoot);
    _secureServer->registerNode(nodeRoot);

    // ======================================================================================
    // /config
    // ======================================================================================
    ResourceNode *nodeConfigGet = new ResourceNode("/config", "GET", &HAPWebServer::handleConfigGet);
    _secureServer->registerNode(nodeConfigGet);
    
    ResourceNode *nodeConfigPost = new ResourceNode("/config", "POST", &HAPWebServer::handleConfigPost);
    _secureServer->registerNode(nodeConfigPost);

    // ======================================================================================
    // /wifi
    // ======================================================================================
    ResourceNode *nodeWifiGet = new ResourceNode("/wifi", "GET", &HAPWebServer::handleWifiGet);
    _secureServer->registerNode(nodeWifiGet);

    ResourceNode *nodeWifiPost = new ResourceNode("/wifi", "POST", &HAPWebServer::handleWifiPost);
    _secureServer->registerNode(nodeWifiPost);


    // ======================================================================================
    // /update
    // ======================================================================================
    ResourceNode *nodeUploadGet = new ResourceNode("/update", "GET", &HAPWebServer::handleUpdateGet);
    _secureServer->registerNode(nodeUploadGet);


    // ToDo: Implement update Post for firmware updates!
    ResourceNode *nodeUploadPost = new ResourceNode("/update", "POST", &HAPWebServer::handleUpdatePost);
    _secureServer->registerNode(nodeUploadPost); 

    // ======================================================================================
    // /api/*
    // ======================================================================================
    // /api/* GET
    ResourceNode *nodeApi = new ResourceNode("/api/*", "GET", &HAPWebServer::handleApi);
    nodeApi->addPathParamValidator(0, &HAPWebServer::validateApiGet);
    _secureServer->registerNode(nodeApi);

    // /api/* POST
    ResourceNode *nodeApiPost = new ResourceNode("/api/*", "POST", &HAPWebServer::handleApi);
    nodeApiPost->addPathParamValidator(0, &HAPWebServer::validateApiPost);
    _secureServer->registerNode(nodeApiPost);

    // /api/* DELETE
    ResourceNode *nodeApiDelete = new ResourceNode("/api/*", "DELETE", &HAPWebServer::handleApi);
    nodeApiDelete->addPathParamValidator(0, &HAPWebServer::validateApiDelete);
    _secureServer->registerNode(nodeApiDelete);


    // ======================================================================================
    // /plugin/*
    // ======================================================================================
    // /plugin/* GET
    ResourceNode *nodePlugin = new ResourceNode("/plugin/*", "GET", &HAPWebServer::handlePluginNodes);    
    nodePlugin->addPathParamValidator(0, &HAPWebServer::validatePluginNodesGet);
    _secureServer->registerNode(nodePlugin);

    // /plugin/* POST
    ResourceNode *nodePluginPost = new ResourceNode("/plugin/*", "POST", &HAPWebServer::handlePluginNodes);    
    nodePluginPost->addPathParamValidator(0, &HAPWebServer::validatePluginNodesPost);
    _secureServer->registerNode(nodePluginPost);


    // ======================================================================================
    // 404 Not Found
    // ======================================================================================
    ResourceNode *node404 = new ResourceNode("", "GET", &HAPWebServer::handle404);

    // Add the 404 not found node to the server.
    // The path is ignored for the default node.
    _secureServer->setDefaultNode(node404);


    // ======================================================================================
    // Middleware
    // ======================================================================================
    // Add the middleware. The function will be called globally for every request
    // Note: The functions are called in the order they are added to the server.
    // Also, if you want a middleware to handle only specific requests, you can check
    // the URL within the middleware function.
    _secureServer->addMiddleware(&HAPWebServer::middlewareLogging);
    _secureServer->addMiddleware(&HAPWebServer::middlewareBasicAuthentication);
    _secureServer->addMiddleware(&HAPWebServer::middlewareBasicAuthorization);

#if HAP_WEBSERVER_USE_JWT
    _secureServer->addMiddleware(&HAPWebServer::middlewareJWTAuthorization);
#endif

    _secureServer->start();
    return _secureServer->isRunning();
}

void HAPWebServer::handle()
{
    // This call will let the server do its work
    _secureServer->loop();
}

void HAPWebServer::setConfig(HAPConfig *config)
{
    _config = config;
}

void HAPWebServer::setAccessorySet(HAPAccessorySet *accessorySet)
{
    _accessorySet = accessorySet;
}

// We want to log the following information for every request:
// - Response Status
// - Request Method
// - Request String (URL + Parameters)
void HAPWebServer::middlewareLogging(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{
    // We want to print the response status, so we need to call next() first.
    next();
    // After the call, the status is (hopefully) set by the handler function, so we can
    // access it for logging.
    // Serial.printf("middlewareLogging(): %3d\t%s\t\t%s\n",
    // 		// Status code (like: 200)
    // 		res->getStatusCode(),
    // 		// Method used for the request (like: GET)
    // 		req->getMethod().c_str(),
    // 		// Request string (like /index.html)
    // 		req->getRequestString().c_str());

    LogD("[HTTPS] Request on " + String(req->getRequestString().c_str()) + " - statusCode: " + String(res->getStatusCode()) + " - method: " + String(req->getMethod().c_str()), true);
}


// ===========================================================================================================
// /
// ===========================================================================================================
// For details on the implementation of the hanlder functions, refer to the Static-Page example.
void HAPWebServer::handleRoot(HTTPRequest *req, HTTPResponse *res)
{
    // template processing
#if HAP_WEBSERVER_USE_SPIFFS        
    HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &HAPWebServer::rootKeyProcessor);
#else
    HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &HAPWebServer::rootKeyProcessor);
#endif    
}

void HAPWebServer::rootKeyProcessor(const String& key, HTTPResponse * res){
    if (key == "VAR_TITLE") {
        res->print("Homekit");
    } else if (key == "VAR_NAVIGATION") {
        res->print(HAPWebServer::buildNavigation());
    } else if (key == "VAR_JAVASCRIPT") {
        
        res->print("");
// #if HAP_WEBSERVER_USE_SPIFFS         
//         // minified       
//         res->print("function VanillaQR(r){var e=this;r=\"object\"==typeof r?r:{},e.revision=3,e.imageTypes={bmp:\"image/bmp\",gif:\"image/gif\",jpeg:\"image/jpeg\",jpg:\"image/jpg\",png:\"image/png\",\"svg+xml\":\"image/svg+xml\",tiff:\"image/tiff\",webp:\"image/webp\",\"x-icon\":\"image/x-icon\"},e.toTable=r.toTable,e.domElement=e.toTable?document.createElement(\"div\"):document.createElement(\"canvas\"),e.url=r.url||\"\",e.size=r.size||280,e.qrc=!1,e.colorLight=r.colorLight||\"#fff\",e.colorDark=r.colorDark||\"#000\",e.ecclevel=r.ecclevel||1,e.noBorder=r.noBorder,e.borderSize=r.borderSize||4;var o,a,n,t,i,l,f,c=[],d=[],s=[],v=[],g=[],m=[],u=function(r,e){var o;r>e&&(o=r,r=e,e=o),o=e,o*=e,o+=e,o>>=1,o+=r,v[o]=1},h=function(r,e){var o;for(s[r+n*e]=1,o=-2;o<2;o++)s[r+o+n*(e-2)]=1,s[r-2+n*(e+o+1)]=1,s[r+2+n*(e+o)]=1,s[r+o+1+n*(e+2)]=1;for(o=0;o<2;o++)u(r-1,e+o),u(r+1,e-o),u(r-o,e-1),u(r+o,e+1)},p=function(r){for(;r>=255;)r-=255,r=(r>>8)+(255&r);return r},b=function(r,e,o,a){var n,t,i,l=VanillaQR.gexp,f=VanillaQR.glog;for(n=0;n<a;n++)c[o+n]=0;for(n=0;n<e;n++){if(i=f[c[r+n]^c[o]],255!=i)for(t=1;t<a;t++)c[o+t-1]=c[o+t]^l[p(i+m[a-t])];else for(t=o;t<o+a;t++)c[t]=c[t+1];c[o+a-1]=255==i?0:l[p(i+m[0])]}},R=function(r,e){var o;return r>e&&(o=r,r=e,e=o),o=e,o+=e*e,o>>=1,o+=r,v[o]},Q=function(r){var e,o,a,t;switch(r){case 0:for(o=0;o<n;o++)for(e=0;e<n;e++)e+o&1||R(e,o)||(s[e+o*n]^=1);break;case 1:for(o=0;o<n;o++)for(e=0;e<n;e++)1&o||R(e,o)||(s[e+o*n]^=1);break;case 2:for(o=0;o<n;o++)for(a=0,e=0;e<n;e++,a++)3==a&&(a=0),a||R(e,o)||(s[e+o*n]^=1);break;case 3:for(t=0,o=0;o<n;o++,t++)for(3==t&&(t=0),a=t,e=0;e<n;e++,a++)3==a&&(a=0),a||R(e,o)||(s[e+o*n]^=1);break;case 4:for(o=0;o<n;o++)for(a=0,t=o>>1&1,e=0;e<n;e++,a++)3==a&&(a=0,t=!t),t||R(e,o)||(s[e+o*n]^=1);break;case 5:for(t=0,o=0;o<n;o++,t++)for(3==t&&(t=0),a=0,e=0;e<n;e++,a++)3==a&&(a=0),(e&o&1)+!(!a|!t)||R(e,o)||(s[e+o*n]^=1);break;case 6:for(t=0,o=0;o<n;o++,t++)for(3==t&&(t=0),a=0,e=0;e<n;e++,a++)3==a&&(a=0),(e&o&1)+(a&&a==t)&1||R(e,o)||(s[e+o*n]^=1);break;case 7:for(t=0,o=0;o<n;o++,t++)for(3==t&&(t=0),a=0,e=0;e<n;e++,a++)3==a&&(a=0),(a&&a==t)+(e+o&1)&1||R(e,o)||(s[e+o*n]^=1)}},V=function(r){var e,o=0;for(e=0;e<=r;e++)g[e]>=5&&(o+=VanillaQR.N1+g[e]-5);for(e=3;e<r-1;e+=2)g[e-2]==g[e+2]&&g[e+2]==g[e-1]&&g[e-1]==g[e+1]&&3*g[e-1]==g[e]&&(0==g[e-3]||e+3>r||3*g[e-3]>=4*g[e]||3*g[e+3]>=4*g[e])&&(o+=VanillaQR.N3);return o},k=function(){var r,e,o,a,t,i=0,l=0;for(e=0;e<n-1;e++)for(r=0;r<n-1;r++)(s[r+n*e]&&s[r+1+n*e]&&s[r+n*(e+1)]&&s[r+1+n*(e+1)]||!(s[r+n*e]||s[r+1+n*e]||s[r+n*(e+1)]||s[r+1+n*(e+1)]))&&(i+=VanillaQR.N2);for(e=0;e<n;e++){for(g[0]=0,o=a=r=0;r<n;r++)(t=s[r+n*e])==a?g[o]++:g[++o]=1,a=t,l+=a?1:-1;i+=V(o)}l<0&&(l=-l);var f=l,c=0;for(f+=f<<2,f<<=1;f>n*n;)f-=n*n,c++;for(i+=c*VanillaQR.N4,r=0;r<n;r++){for(g[0]=0,o=a=e=0;e<n;e++)(t=s[r+n*e])==a?g[o]++:g[++o]=1,a=t;i+=V(o)}return i};e.genframe=function(r){var e,g,V,x,C,w,E,N,T=VanillaQR.eccblocks,y=VanillaQR.gexp,z=VanillaQR.glog;x=r.length,a=0;do if(a++,V=4*(o-1)+16*(a-1),t=T[V++],i=T[V++],l=T[V++],f=T[V],V=l*(t+i)+i-3+(a<=9),x<=V)break;while(a<40);for(n=17+4*a,C=l+(l+f)*(t+i)+i,x=0;x<C;x++)d[x]=0;for(c=r.slice(0),x=0;x<n*n;x++)s[x]=0;for(x=0;x<(n*(n+1)+1)/2;x++)v[x]=0;for(x=0;x<3;x++){for(V=0,g=0,1==x&&(V=n-7),2==x&&(g=n-7),s[g+3+n*(V+3)]=1,e=0;e<6;e++)s[g+e+n*V]=1,s[g+n*(V+e+1)]=1,s[g+6+n*(V+e)]=1,s[g+e+1+n*(V+6)]=1;for(e=1;e<5;e++)u(g+e,V+1),u(g+1,V+e+1),u(g+5,V+e),u(g+e+1,V+5);for(e=2;e<4;e++)s[g+e+n*(V+2)]=1,s[g+2+n*(V+e+1)]=1,s[g+4+n*(V+e)]=1,s[g+e+1+n*(V+4)]=1}if(a>1)for(x=VanillaQR.adelta[a],g=n-7;;){for(e=n-7;e>x-3&&(h(e,g),!(e<x));)e-=x;if(g<=x+9)break;g-=x,h(6,g),h(g,6)}for(s[8+n*(n-8)]=1,g=0;g<7;g++)u(7,g),u(n-8,g),u(7,g+n-7);for(e=0;e<8;e++)u(e,7),u(e+n-8,7),u(e,n-8);for(e=0;e<9;e++)u(e,8);for(e=0;e<8;e++)u(e+n-8,8),u(8,e);for(g=0;g<7;g++)u(8,g+n-7);for(e=0;e<n-14;e++)1&e?(u(8+e,6),u(6,8+e)):(s[8+e+6*n]=1,s[6+n*(8+e)]=1);if(a>6)for(x=VanillaQR.vpat[a-7],V=17,e=0;e<6;e++)for(g=0;g<3;g++,V--)1&(V>11?a>>V-12:x>>V)?(s[5-e+n*(2-g+n-11)]=1,s[2-g+n-11+n*(5-e)]=1):(u(5-e,2-g+n-11),u(2-g+n-11,5-e));for(g=0;g<n;g++)for(e=0;e<=g;e++)s[e+n*g]&&u(e,g);for(C=c.length,w=0;w<C;w++)d[w]=c.charCodeAt(w);if(c=d.slice(0),e=l*(t+i)+i,C>=e-2&&(C=e-2,a>9&&C--),w=C,a>9){for(c[w+2]=0,c[w+3]=0;w--;)x=c[w],c[w+3]|=255&x<<4,c[w+2]=x>>4;c[2]|=255&C<<4,c[1]=C>>4,c[0]=64|C>>12}else{for(c[w+1]=0,c[w+2]=0;w--;)x=c[w],c[w+2]|=255&x<<4,c[w+1]=x>>4;c[1]|=255&C<<4,c[0]=64|C>>4}for(w=C+3-(a<10);w<e;)c[w++]=236,c[w++]=17;for(m[0]=1,w=0;w<f;w++){for(m[w+1]=1,E=w;E>0;E--)m[E]=m[E]?m[E-1]^y[p(z[m[E]]+w)]:m[E-1];m[0]=y[p(z[m[0]]+w)]}for(w=0;w<=f;w++)m[w]=z[m[w]];for(V=e,g=0,w=0;w<t;w++)b(g,l,V,f),g+=l,V+=f;for(w=0;w<i;w++)b(g,l+1,V,f),g+=l+1,V+=f;for(g=0,w=0;w<l;w++){for(E=0;E<t;E++)d[g++]=c[w+E*l];for(E=0;E<i;E++)d[g++]=c[t*l+w+E*(l+1)]}for(E=0;E<i;E++)d[g++]=c[t*l+w+E*(l+1)];for(w=0;w<f;w++)for(E=0;E<t+i;E++)d[g++]=c[e+w+E*f];for(c=d,e=g=n-1,V=C=1,N=(l+f)*(t+i)+i,w=0;w<N;w++)for(x=c[w],E=0;E<8;E++,x<<=1){128&x&&(s[e+n*g]=1);do C?e--:(e++,V?0!=g?g--:(e-=2,V=!V,6==e&&(e--,g=9)):g!=n-1?g++:(e-=2,V=!V,6==e&&(e--,g-=8))),C=!C;while(R(e,g))}for(c=s.slice(0),x=0,g=3e4,V=0;V<8&&(Q(V),e=k(),e<g&&(g=e,x=V),7!=x);V++)s=c.slice(0);for(x!=V&&Q(x),g=VanillaQR.fmtword[x+(o-1<<3)],V=0;V<8;V++,g>>=1)1&g&&(s[n-1-V+8*n]=1,V<6?s[8+n*V]=1:s[8+n*(V+1)]=1);for(V=0;V<7;V++,g>>=1)1&g&&(s[8+n*(n-7+V)]=1,V?s[6-V+8*n]=1:s[7+8*n]=1);return s},e.init=function(){o=e.ecclevel;var r=e.genframe(e.url);e.toTable?e.tableWrite(r,n):e.canvasWrite(r,n)},e.init()}VanillaQR.prototype={canvasWrite:function(r,e){var o=this;if(!o.qrc&&(o.qrc=o.getContext(o.domElement),!o.qrc))return o.toTable=!0,o.domElement=document.createElement(\"div\"),void o.tableWrite(r,e);var a=o.size,n=o.qrc;n.lineWidth=1;var t=a;t/=e+10,t=Math.round(t-.5);var i=4;o.noBorder?(n.canvas.width=n.canvas.height=t*e,i=0):n.canvas.width=n.canvas.height=a,n.clearRect(0,0,a,a),n.fillStyle=o.colorLight,n.fillRect(0,0,t*(e+8),t*(e+8)),n.fillStyle=o.colorDark;for(var l=0;l<e;l++)for(var f=0;f<e;f++)r[f*e+l]&&n.fillRect(t*(i+l),t*(i+f),t,t)},tableWrite:function(r,e){var o=this,a=\"border:0;border-collapse:collapse;\",n=Math.round(this.size/e-3.5)+\"px\",t=e+(o.noBorder?0:2*o.borderSize),i=o.borderSize,l=\"width:\"+n+\";height:\"+n+\";\",f=o.colorLight,c=o.colorDark,d=document.createElement(\"table\");d.style.cssText=a;for(var s=document.createElement(\"tr\"),v=document.createElement(\"td\"),g=function(){return v.cloneNode()},m=function(){var r=g();return r.style.cssText=l+\"background:\"+c,r},u=function(){var r=g();return r.style.cssText=l+\"background:\"+f,r},h=function(r){for(var e=r.firstChild,a=0;a<o.borderSize;a++){for(var n=s.cloneNode(),i=0;i<t;i++){var l=u();n.appendChild(l)}r.appendChild(n),r.insertBefore(n.cloneNode(!0),e)}},p=function(r){for(var e=r.firstChild,o=0;o<i;o++)r.insertBefore(u(),e),r.appendChild(u())},b=0;b<e;b++){var R=s.cloneNode();d.appendChild(R);for(var Q=0;Q<e;Q++)if(1===r[b*e+Q]){var V=m();R.appendChild(V)}else{var k=u();R.appendChild(k)}o.noBorder||p(R)}o.noBorder||h(d),o.domElement.innerHTML=\"\",o.domElement.appendChild(d)},getContext:function(r){return r.getContext&&r.getContext(\"2d\")?r.getContext(\"2d\"):(console.log(\"Browser does not have 2d Canvas support\"),!1)},toImage:function(r){if(this.qrc){var e=this.imageTypes[r];if(!e)throw new Error(r+\" is not a valid image type \");var o=new Image;return o.src=this.domElement.toDataURL(e),o}}},VanillaQR.adelta=[0,11,15,19,23,27,31,16,18,20,22,24,26,28,20,22,24,24,26,28,28,22,24,24,26,26,28,28,24,24,26,26,26,28,28,24,26,26,26,28,28],VanillaQR.vpat=[3220,1468,2713,1235,3062,1890,2119,1549,2344,2936,1117,2583,1330,2470,1667,2249,2028,3780,481,4011,142,3098,831,3445,592,2517,1776,2234,1951,2827,1070,2660,1345,3177],VanillaQR.fmtword=[30660,29427,32170,30877,26159,25368,27713,26998,21522,20773,24188,23371,17913,16590,20375,19104,13663,12392,16177,14854,9396,8579,11994,11245,5769,5054,7399,6608,1890,597,3340,2107],VanillaQR.eccblocks=[1,0,19,7,1,0,16,10,1,0,13,13,1,0,9,17,1,0,34,10,1,0,28,16,1,0,22,22,1,0,16,28,1,0,55,15,1,0,44,26,2,0,17,18,2,0,13,22,1,0,80,20,2,0,32,18,2,0,24,26,4,0,9,16,1,0,108,26,2,0,43,24,2,2,15,18,2,2,11,22,2,0,68,18,4,0,27,16,4,0,19,24,4,0,15,28,2,0,78,20,4,0,31,18,2,4,14,18,4,1,13,26,2,0,97,24,2,2,38,22,4,2,18,22,4,2,14,26,2,0,116,30,3,2,36,22,4,4,16,20,4,4,12,24,2,2,68,18,4,1,43,26,6,2,19,24,6,2,15,28,4,0,81,20,1,4,50,30,4,4,22,28,3,8,12,24,2,2,92,24,6,2,36,22,4,6,20,26,7,4,14,28,4,0,107,26,8,1,37,22,8,4,20,24,12,4,11,22,3,1,115,30,4,5,40,24,11,5,16,20,11,5,12,24,5,1,87,22,5,5,41,24,5,7,24,30,11,7,12,24,5,1,98,24,7,3,45,28,15,2,19,24,3,13,15,30,1,5,107,28,10,1,46,28,1,15,22,28,2,17,14,28,5,1,120,30,9,4,43,26,17,1,22,28,2,19,14,28,3,4,113,28,3,11,44,26,17,4,21,26,9,16,13,26,3,5,107,28,3,13,41,26,15,5,24,30,15,10,15,28,4,4,116,28,17,0,42,26,17,6,22,28,19,6,16,30,2,7,111,28,17,0,46,28,7,16,24,30,34,0,13,24,4,5,121,30,4,14,47,28,11,14,24,30,16,14,15,30,6,4,117,30,6,14,45,28,11,16,24,30,30,2,16,30,8,4,106,26,8,13,47,28,7,22,24,30,22,13,15,30,10,2,114,28,19,4,46,28,28,6,22,28,33,4,16,30,8,4,122,30,22,3,45,28,8,26,23,30,12,28,15,30,3,10,117,30,3,23,45,28,4,31,24,30,11,31,15,30,7,7,116,30,21,7,45,28,1,37,23,30,19,26,15,30,5,10,115,30,19,10,47,28,15,25,24,30,23,25,15,30,13,3,115,30,2,29,46,28,42,1,24,30,23,28,15,30,17,0,115,30,10,23,46,28,10,35,24,30,19,35,15,30,17,1,115,30,14,21,46,28,29,19,24,30,11,46,15,30,13,6,115,30,14,23,46,28,44,7,24,30,59,1,16,30,12,7,121,30,12,26,47,28,39,14,24,30,22,41,15,30,6,14,121,30,6,34,47,28,46,10,24,30,2,64,15,30,17,4,122,30,29,14,46,28,49,10,24,30,24,46,15,30,4,18,122,30,13,32,46,28,48,14,24,30,42,32,15,30,20,4,117,30,40,7,47,28,43,22,24,30,10,67,15,30,19,6,118,30,18,31,47,28,34,34,24,30,20,61,15,30],VanillaQR.glog=[255,0,1,25,2,50,26,198,3,223,51,238,27,104,199,75,4,100,224,14,52,141,239,129,28,193,105,248,200,8,76,113,5,138,101,47,225,36,15,33,53,147,142,218,240,18,130,69,29,181,194,125,106,39,249,185,201,154,9,120,77,228,114,166,6,191,139,98,102,221,48,253,226,152,37,179,16,145,34,136,54,208,148,206,143,150,219,189,241,210,19,92,131,56,70,64,30,66,182,163,195,72,126,110,107,58,40,84,250,133,186,61,202,94,155,159,10,21,121,43,78,212,229,172,115,243,167,87,7,112,192,247,140,128,99,13,103,74,222,237,49,197,254,24,227,165,153,119,38,184,180,124,17,68,146,217,35,32,137,46,55,63,209,91,149,188,207,205,144,135,151,178,220,252,190,97,242,86,211,171,20,42,93,158,132,60,57,83,71,109,65,162,31,45,67,216,183,123,164,118,196,23,73,236,127,12,111,246,108,161,59,82,41,157,85,170,251,96,134,177,187,204,62,90,203,89,95,176,156,169,160,81,11,245,22,235,122,117,44,215,79,174,213,233,230,231,173,232,116,214,244,234,168,80,88,175],VanillaQR.gexp=[1,2,4,8,16,32,64,128,29,58,116,232,205,135,19,38,76,152,45,90,180,117,234,201,143,3,6,12,24,48,96,192,157,39,78,156,37,74,148,53,106,212,181,119,238,193,159,35,70,140,5,10,20,40,80,160,93,186,105,210,185,111,222,161,95,190,97,194,153,47,94,188,101,202,137,15,30,60,120,240,253,231,211,187,107,214,177,127,254,225,223,163,91,182,113,226,217,175,67,134,17,34,68,136,13,26,52,104,208,189,103,206,129,31,62,124,248,237,199,147,59,118,236,197,151,51,102,204,133,23,46,92,184,109,218,169,79,158,33,66,132,21,42,84,168,77,154,41,82,164,85,170,73,146,57,114,228,213,183,115,230,209,191,99,198,145,63,126,252,229,215,179,123,246,241,255,227,219,171,75,150,49,98,196,149,55,110,220,165,87,174,65,130,25,50,100,200,141,7,14,28,56,112,224,221,167,83,166,81,162,89,178,121,242,249,239,195,155,43,86,172,69,138,9,18,36,72,144,61,122,244,245,247,243,251,235,203,139,11,22,44,88,176,125,250,233,207,131,27,54,108,216,173,71,142,0],VanillaQR.N1=3,VanillaQR.N2=3,VanillaQR.N3=40,VanillaQR.N4=10;");
//         res->print("\n");
        
//         // minified
//         res->print("function drawQRCode(xhm, pincode) {var qr = new VanillaQR({url: xhm, size: 425, colorLight: \"#ffffff\", colorDark: \"#000000\", toTable: false, ecclevel: 3, noBorder: true, borderSize: 0 }); var theDiv = document.getElementById(\"qr_code\"); theDiv.appendChild(qr.domElement); var pincode = pincode.replace(/-/g, \"\"); var pin1 = pincode.substring(0,4); var pin2 = pincode.substring(4,8); theDiv = document.getElementById(\"pin_1\"); theDiv.innerHTML = pin1; theDiv = document.getElementById(\"pin_2\"); theDiv.innerHTML = pin2; }");
// #else    
//         // minified outsourced    
//         res->print((const char*)html_template_vanilla_qr_start);
// #endif        
//         res->print("\n");
//         res->printf("window.onload=function(){ drawQRCode(\"%s\", \"%s\");}", _accessorySet->xhm(), _accessorySet->pinCode());
//         res->print("\n");


    } else if (key == "VAR_CSS") {

#if HAP_WEBSERVER_USE_SPIFFS          
        res->print("@font-face{font-family:Homekit;src:url(data:font/truetype;charset=utf-8;base64,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) format(\"truetype\");font-weight:400;font-style:normal}.homekit_qr_container{position:relative;text-align:center;color:#000;width:400px}.grossabstand{letter-spacing:.3em;font-size:56px;font-family:Homekit}.centered{position:absolute;top:50%;left:50%;transform:translate(-50%,-50%)}.homekit_qr_text_1{position:absolute;top:25px;left:170px}.homekit_qr_text_2{position:absolute;top:85px;left:170px}.homekit_qr_code{position:absolute;top:180px;left:50px}");
#else        
        res->print((const char*)html_template_qrcode_font_start);
#endif
        res->print("\n");
    } else if (key == "VAR_CONTENT") {
        //res->print("<div class=\"pure-u-1-3 pure-u-md-1 absatz\"><div class=\"homekit_qr_container\"> <img src=\"data:image/png;base64,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\" alt=\"Homekit\" style=\"width:400px;\"><div class=\"homekit_qr_text_1 grossabstand\" id=\"pin_1\"></div><div class=\"homekit_qr_text_2 grossabstand\" id=\"pin_2\"></div><div class=\"homekit_qr_code\" id=\"qr_code\"></div><p>Scan this code with your iPhone to pair this device</p></div></div><div class=\"pure-u-2-3 absatz\"><p> Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.</p><p> Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.</p></div>");        
        res->print("<div class=\"pure-u-1-3 pure-u-md-1 absatz\"><div class=\"homekit_qr_container\">");
        // 
        res->print((const char*)html_template_qrcode_container_start);

        // pin 1st
        res->printf("<div class=\"homekit_qr_text_1 grossabstand\" id=\"pin_1\">%s</div>", "0314");
        
        // pin 2nd
        res->printf("<div class=\"homekit_qr_text_2 grossabstand\" id=\"pin_2\">%s</div>", "5712");
        
        
        
       
	    // Generate QR Code
        res->print("<div class=\"homekit_qr_code\" id=\"qr_code\">");
        QRCode qrCode;
        uint8_t qrcodeData[qrcode_getBufferSize(3)];
        qrcode_initText(&qrCode, qrcodeData, 3, ECC_HIGH, _accessorySet->xhm());

        HAPSVG::drawQRCode(res, &qrCode);

        res->print("</div><p>Scan this code with your iPhone to pair this device</p></div></div>");




        // Main frame

        res->print("</body>");
        res->print("</html>");
    } else {
        res->print("");
    }
}

// ===========================================================================================================
// /wifi
// ===========================================================================================================
void HAPWebServer::handleWifiGet(HTTPRequest * req, HTTPResponse * res){

}

void HAPWebServer::handleWifiPost(HTTPRequest * req, HTTPResponse * res){

}

// void HAPWebServer::handleConfigHTTPFormField(const std::string& fieldName, const std::string& fieldValue) { 

//     HAPConfigValidationResult result;
//     result.valid = true;

//     LogI("FIELD: ", false);
//     LogI(fieldName.c_str(), true);
//     LogI("VALUE: ", false);
//     LogI(fieldValue.c_str(), true);
// }

// ===========================================================================================================
// /config
// ===========================================================================================================
void HAPWebServer::handleConfigPost(HTTPRequest *req, HTTPResponse *res){

    // Checking permissions can not only be done centrally in the middleware function but also in the actual request handler.
    // This would be handy if you provide an API with lists of resources, but access rights are defined object-based.
    // if (req->getHeader(HEADER_GROUP) == "ADMIN")    
    // {
        size_t capacity = HAP_ARDUINOJSON_BUFFER_SIZE / 2;

        // Try to read request into buffer
        size_t idx = 0;

        String boundry;
        String contentType = req->getHeader("Content-Type").c_str();

        size_t idxBoundry = contentType.indexOf("boundry=");
        size_t idxComma = contentType.indexOf(";", idxBoundry + 1);

        if ((idxBoundry > 0) && (idxComma > 0))
        {
            boundry = contentType.substring(idxBoundry + 8, idxComma);
        }
        else if ((idxBoundry > 0))
        {
            boundry = contentType.substring(idxBoundry + 8);
        }


        bool foundBoundry = false;

        bool foundFrom = false;
        bool foundTo = false;

        int from = 0;
        int to = 0;

        // Create buffer to read request
        uint8_t *buffer = new uint8_t[capacity + 1];
        memset(buffer, 0, capacity + 1);

        // while "not everything read" or "buffer is full"
        while (!req->requestComplete() && idx < capacity)
        {
            size_t startIdx = idx;
            idx += req->readBytes(buffer + idx, capacity - idx);

            char *p;
            int k = 1;
            p = strchr((char *)buffer + startIdx, '\n');
            while (p != NULL)
            {

                size_t curIdx = p - (char *)buffer - startIdx + 1;

                // printf ("Startindex at position %d\n", startIdx);
                // printf ("Character i found at position %d\n", curIdx);
                // printf ("Occurrence of character \"i\" : %d \n",k);

                if (strncmp((char *)buffer + startIdx, "--", 2) == 0) {           
                    foundBoundry = true;
                }

                if (strncmp((char *)buffer + startIdx, "\r\n", 2) == 0) {
          
                    if (foundFrom == false) {
                        from = startIdx + curIdx;
                        foundFrom = true;
                        foundTo = false;
                    } else if (foundTo == false) {
                        to = startIdx + curIdx;
                        foundTo = true;
                    }
                }

                // Serial.write(buffer + startIdx, curIdx);
                // Serial.write("\0");

                startIdx += curIdx;

                p = strchr(p + 1, '\n');
                k++;
            }
        }

        // If the request is still not read completely, we cannot process it.
        if (!req->requestComplete()) {
            res->setStatusCode(413);
            res->setStatusText("Request entity too large");
            res->println("413 Request entity too large");
            // Clean up
            delete[] buffer;
            return;
        }

        if (to == 0) {
            to = idx;
        }

#if HAP_DEBUG
        Serial.write(buffer + from, to - from);
        Serial.println("");
#endif

        HAPConfigValidationResult result = _config->parse(buffer + from, to - from, false);
        if (result.valid == false) {
            res->setStatusCode(400);
            res->setStatusText("Bad Request");
            res->setHeader("Content-Type", "application/json");
            res->println("{ \"Bad Request\": \"" + result.reason + "\"}");
            // Clean up
            delete[] buffer;
            return;
        } else {
            
            bool error = _config->save();
            if (error == false)
            {
                LogE("ERROR: Could not save configuration!", true);

                res->setStatusCode(500);
                res->setStatusText("Internal Server Error");
                // res->println("Passed but could not save :(");
            } else {
                LogD("Config saved successfully!", true);

                // res->setStatusCode(201);
                // res->setStatusText("Created");

                // Call updatecallback
                _config->update();

                // template processing                    
                

#if HAP_WEBSERVER_USE_SPIFFS        
                HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &HAPWebServer::configGetKeyProcessor, 201, "Created", "text/html");
#else
                HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &HAPWebServer::configGetKeyProcessor, 201, "Created", "text/html");
#endif                

            }            
        }

        delete[] buffer;
    // } else {
    //     res->setStatusCode(401);
    //     res->setStatusText("Unauthorized");
    //     res->setHeader("Content-Type", "text/html");
    //     res->println("<p><strong>403 Unauthorized</strong> You have no power here!</p>");
    // }

}



void HAPWebServer::handleConfigGet(HTTPRequest *req, HTTPResponse *res){
    // Checking permissions can not only be done centrally in the middleware function but also in the actual request handler.
    // This would be handy if you provide an API with lists of resources, but access rights are defined object-based.
    // if (req->getHeader(HEADER_GROUP) == "ADMIN")
    // {

        req->discardRequestBody();    

        // template processing        
#if HAP_WEBSERVER_USE_SPIFFS        
        HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &HAPWebServer::configGetKeyProcessor);
#else
        HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &HAPWebServer::configGetKeyProcessor);
#endif

    // } else {
    //     res->setStatusCode(401);
    //     res->setStatusText("Unauthorized");
    //     res->setHeader("Content-Type", "text/html");
    //     res->println("<p><strong>403 Unauthorized</strong> You have no power here!</p>");
    // }
}


void HAPWebServer::configGetKeyProcessor(const String& key, HTTPResponse* res){

#if HAP_WEBSERVER_TEMPLATE_PROCESSING_CHUNKED
    if (key == "VAR_TITLE") {
        // res->print("Configuration");
        HAPWebServerTemplateProcessor::sendChunk(res, "Configuration");
    } else if (key == "VAR_NAVIGATION") {
        // res->print(HAPWebServer::buildNavigation());  
        HAPWebServerTemplateProcessor::sendChunk(res, buildNavigation());  
    } else if (key == "VAR_CONTENT") {
        String result = "";

        result += "<div class=\"pure-u-1 pure-u-md-1\">";

        result += "<p>Please specify the config file you want to upload</p>";
    	result += "<form action=\"/config\" method=\"post\" enctype=\"multipart/form-data\">";
        result += "<input type=\"file\" name=\"configfile\">";
        result += "<input class=\"pure-button\" type=\"submit\" value=\"Upload\">";
        result += "</form>";

        result += "<br>";

        result += "<form action=\"/config\" method=\"post\" enctype=\"multipart/form-data\">";
        result += "<textarea class=\"jsonviewer\" name=\"config\" cols=\"50\" rows=\"20\">";
        HAPWebServerTemplateProcessor::sendChunk(res, result);

        res->printf("%x%s", _config->measureLength() , "\r\n");
        _config->prettyPrintTo(*res);

        result += "</textarea>";
        result += "<button type=\"submit\" class=\"pure-button pure-button-primary\">Save</button>";
        result += "</form>";
        result += "</div>";
        HAPWebServerTemplateProcessor::sendChunk(res, result);
#else

    if (key == "VAR_TITLE") {
        res->print("Configuration");
    } else if (key == "VAR_NAVIGATION") {
        res->print(HAPWebServer::buildNavigation());    
    } else if (key == "VAR_CONTENT") {
        //_config->printToHTTPResponse(res);   

        res->print("<div class=\"pure-u-1 pure-u-md-1\">");

        res->print("<p>Please specify the config file you want to upload</p>");
    	res->print("<form action=\"/config\" method=\"post\" enctype=\"multipart/form-data\">");
        res->print("<input type=\"file\" name=\"configfile\">");
        res->print("<input class=\"pure-button\" type=\"submit\" value=\"Upload\">");
        res->print("</form>");

        res->print("<br>");

        res->print("<form action=\"/config\" method=\"post\" enctype=\"multipart/form-data\">");
        res->print("<textarea class=\"jsonviewer\" name=\"config\" cols=\"50\" rows=\"20\">");
        _config->prettyPrintTo(*res);
        res->print("</textarea>");
        res->print("<button type=\"submit\" class=\"pure-button pure-button-primary\">Save</button>");
        res->print("</form>");

        res->print("</div>");
        res->print("");
        
    } else {
        res->print("");
    }
#endif    
}



// ===========================================================================================================
// /update
// ===========================================================================================================
void HAPWebServer::handleUpdatePost(HTTPRequest * req, HTTPResponse * res){

}

void HAPWebServer::handleUpdateGet(HTTPRequest *req, HTTPResponse *res)
{    
    req->discardRequestBody();

    // template processing
#if HAP_WEBSERVER_USE_SPIFFS        
    HAPWebServerTemplateProcessor::processAndSend(res, "/index.html", &HAPWebServer::updateKeyProcessor);
#else
    HAPWebServerTemplateProcessor::processAndSendEmbedded(res, html_template_index_start, html_template_index_end, &HAPWebServer::updateKeyProcessor);
#endif
}

void HAPWebServer::updateKeyProcessor(const String& key, HTTPResponse* res){
    if (key == "VAR_TITLE") {
        res->print("Update");
    } else if (key == "VAR_NAVIGATION") {
        res->print(HAPWebServer::buildNavigation());    
    } else if (key == "VAR_CONTENT") {
        res->print("<div class=\"pure-u-1 pure-u-md-1\">");
    	res->print("<p>Please specify the firmware you want to upload</p>");
    	res->print("<form action=\"/update\" method=\"post\" enctype=\"multipart/form-data\">");
        res->print("<input type=\"file\" name=\"firmare\">");
        res->print("<input class=\"pure-button\" type=\"submit\" value=\"Upload\">");
        res->print("</form>");
        res->print("</div>");
    } else {
        res->print("");
    }
}

// ===========================================================================================================
// 404 Not Found
// ===========================================================================================================
void HAPWebServer::handle404(HTTPRequest *req, HTTPResponse *res)
{

    //if(loadFromFlash( req, res )) return;

    req->discardRequestBody();
    res->setStatusCode(404);
    res->setStatusText("Not Found");
    res->setHeader("Content-Type", "text/html");
    res->println("<!DOCTYPE html>");
    res->println("<html>");
    res->println("<head><title>Not Found</title></head>");
    res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
    res->println("</html>");
}

void HAPWebServer::handleLogin(HTTPRequest *req, HTTPResponse *res)
{
    req->discardRequestBody();

    //req->getHeader(HEADER_USERNAME);
    // The "admin area" will only be shown if the correct group has been assigned in the authenticationMiddleware
    if (req->getHeader(HEADER_GROUP) == "ADMIN")
    {
        LogD("[HTTPS] Access granted", true);

        res->setStatusCode(200);
        res->setStatusText("OK");

#if HAP_WEBSERVER_USE_JWT
        res->setHeader("Content-Type", "application/json");

        res->print("{\"access_token\":\"");
        //Serial.println("---->>> JWT:");
        //Serial.println("started: " + String(millis()));

        char *jwt = HAPWebServerJWT::createGCPJWT("api", "https://esp32.local", (unsigned char *)server_privateKey_der_start, server_privateKey_der_end - server_privateKey_der_start);
        res->print(jwt);
        //Serial.println("ended:   " + String(millis()));
        LogV("Creaetd JWT access token: ", false);
        LogV(jwt, true);

#endif
        res->println("\"}");

#if HAP_WEBSERVER_USE_JWT
        free(jwt);
#endif
    }
}


// ===========================================================================================================
// /api
// ===========================================================================================================
void HAPWebServer::handleApi(HTTPRequest *req, HTTPResponse *res)
{

    // Get access to the parameters
    ResourceParameters *params = req->getParams();
    
    if ( (strcmp(params->getPathParameter(0).c_str(), "heap") == 0) && (strcmp(req->getMethod().c_str(), "GET") == 0) )                
    {
        req->discardRequestBody();
        handleApiHeap(req, res);
        return;
    }

    else if ( (strcmp(params->getPathParameter(0).c_str(), "restart") == 0) && (strcmp(req->getMethod().c_str(), "POST") == 0) )                
    {
        req->discardRequestBody();
        handleApiRestart(req, res);
        return;
    }

    else if ( (strcmp(params->getPathParameter(0).c_str(), "reset") == 0) && (strcmp(req->getMethod().c_str(), "POST") == 0) )                
    {
        req->discardRequestBody();
        handleApiReset(req, res);
        return;
    }

    else if ( (strcmp(params->getPathParameter(0).c_str(), "setup") == 0) && (strcmp(req->getMethod().c_str(), "GET") == 0) )                
    {
        req->discardRequestBody();
        handleApiSetup(req, res);
        return;
    }

    else if ( (strcmp(params->getPathParameter(0).c_str(), "pairings") == 0) && (strcmp(req->getMethod().c_str(), "DELETE") == 0) )
    {
        req->discardRequestBody();
        handleApiPairingsDelete(req, res);
        return;
    }        
    
    else if ( (strcmp(params->getPathParameter(0).c_str(), "uptime") == 0) && (strcmp(req->getMethod().c_str(), "GET") == 0) )        
    {
        req->discardRequestBody();
        handleApiUptime(req, res);
        return;
    }
    else if ( (strcmp(params->getPathParameter(0).c_str(), "config") == 0) && (strcmp(req->getMethod().c_str(), "POST") == 0) )
    {
        // req->discardRequestBody();
        handleApiConfigPost(req, res);
        return;
    }
    else if ( (strcmp(params->getPathParameter(0).c_str(), "config") == 0) && (strcmp(req->getMethod().c_str(), "GET") == 0) )
    {
        req->discardRequestBody();
        handleApiConfigGet(req, res);
        return;
    }
    else if ( (strcmp(params->getPathParameter(0).c_str(), "reftime") == 0) && (strcmp(req->getMethod().c_str(), "GET") == 0) )
    {
        req->discardRequestBody();
        handleApiRefTimeGet(req, res);
        return;
    }
    else if ( (strcmp(params->getPathParameter(0).c_str(), "reftime") == 0) && (strcmp(req->getMethod().c_str(), "POST") == 0) )
    {
        //req->discardRequestBody();
        handleApiRefTimePost(req, res);
        return;
    } 
    
    handle404(req, res);
    
}

bool HAPWebServer::validateApiGet(const std::string s)
{
    if (strcmp(s.c_str(), "heap") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "uptime") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "config") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "reftime") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "setup") == 0)
    {
        return true;
    }

    return false;
}

bool HAPWebServer::validateApiPost(const std::string s)
{
    if (strcmp(s.c_str(), "config") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "reset") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "reftime") == 0)
    {
        return true;
    }
    else if (strcmp(s.c_str(), "restart") == 0)
    {
        return true;
    }
    
    return false;
}

bool HAPWebServer::validateApiDelete(const std::string s)
{

   
    if (strcmp(s.c_str(), "pairings") == 0)
    {
        return true;
    }
   
    return false;
}

// ===========================================================================================================
// /api/config
// ===========================================================================================================
void HAPWebServer::handleApiConfigGet(HTTPRequest *req, HTTPResponse *res)
{
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    // JsonObject cfg = _config->config();
    String configStr;
    serializeJsonPretty(_config->config(), configStr);
    res->println(configStr.c_str());    
}

void HAPWebServer::handleApiConfigPost(HTTPRequest *req, HTTPResponse *res)
{    
    const size_t capacity = HAP_ARDUINOJSON_BUFFER_SIZE;
    DynamicJsonDocument doc(capacity);

    // Create buffer to read request
    uint8_t * buffer = new uint8_t[capacity + 1];
    memset(buffer, 0, capacity+1);

    // Try to read request into buffer
    size_t idx = 0;
    // while "not everything read" or "buffer is full"
    while (!req->requestComplete() && idx < capacity) {
        idx += req->readBytes(buffer + idx, capacity-idx);
    }

    // If the request is still not read completely, we cannot process it.
    if (!req->requestComplete()) {
        res->setStatusCode(413);
        res->setStatusText("Request entity too large");
        // res->println("413 Request entity too large");
        
        // Clean up
        delete[] buffer;
        return;
    }

    HAPConfigValidationResult result = _config->parse(buffer, capacity, false);
    if (result.valid == false)
    {
        res->setStatusCode(400);
        res->setStatusText("Bad Request");
        res->setHeader("Content-Type", "application/json");
        res->println("{ \"Bad Request\": \"" + result.reason + "\"}");
        // Clean up
        delete[] buffer;
        return;
    } else {                

        bool error = _config->save();
        if (error == false)
        {
            LogE("ERROR: Could not save configuration!", true);

            res->setStatusCode(500);
            res->setStatusText("Internal Server Error");
            
        } else {
            LogI("Config saved successfully!", true);

            res->setStatusCode(201);
            res->setStatusText("Created");

            // Call updatecallback
            _config->update();                       
        }
    }

    delete[] buffer;
    return;
}


// ===========================================================================================================
// /api/heap
// ===========================================================================================================
void HAPWebServer::handleApiHeap(HTTPRequest *req, HTTPResponse *res)
{
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->printf("{ \"heap\": %d }\n", ESP.getFreeHeap());
}

// ===========================================================================================================
// /api/uptime
// ===========================================================================================================
void HAPWebServer::handleApiUptime(HTTPRequest *req, HTTPResponse *res)
{
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->printf("{ \"uptime\": %lu }\n", millis()/1000);
}


// ===========================================================================================================
// /api/reftime
// ===========================================================================================================
void HAPWebServer::handleApiRefTimeGet(HTTPRequest *req, HTTPResponse *res)
{
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->printf("{ \"reftime\": %zu }\n", _config->getRefTime());
}

void HAPWebServer::handleApiRefTimePost(HTTPRequest *req, HTTPResponse *res)
{
    const size_t capacity = JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonDocument doc(capacity);

    // Create buffer to read request
    char * buffer = new char[capacity + 1];
    memset(buffer, 0, capacity+1);

    // Try to read request into buffer
    size_t idx = 0;
    // while "not everything read" or "buffer is full"
    while (!req->requestComplete() && idx < capacity) {
        idx += req->readChars(buffer + idx, capacity-idx);
    }

    // If the request is still not read completely, we cannot process it.
    if (!req->requestComplete()) {
        res->setStatusCode(413);
        res->setStatusText("Request entity too large");
        res->println("413 Request entity too large");
        // Clean up
        delete[] buffer;
        return;
    }

    // Parse the object
    DeserializationError error = deserializeJson(doc, buffer, capacity + 1);
    if (error){        
        res->setStatusCode(400);
        res->setStatusText("Bad Request");
        res->println("400 Bad Request");
        delete[] buffer;
        return;
    }

    // Validate and set
    if (doc.containsKey("reftime") && doc["reftime"].is<uint32_t>()) {        
        _config->setRefTime(doc["reftime"].as<uint32_t>(), true);

        res->setStatusCode(201);
        res->setStatusText("Created");

    } else {
        res->setStatusCode(400);
        res->setStatusText("Bad Request");
        // res->println("400 Bad Request");      
    }
        

    // Clean up, we don't need the buffer any longer
    delete[] buffer;
}




// ===========================================================================================================
// /api/pairings
// ADMIN only
// ===========================================================================================================
void HAPWebServer::handleApiPairingsDelete(HTTPRequest *req, HTTPResponse *res)
{
    HAPPairings::resetEEPROM();

    req->discardRequestBody();
    res->setStatusCode(200);
    res->setStatusText("OK");
    // res->setHeader("Content-Type", "text/html");
    // res->println("<p>All parings deleted!</p>");
    res->print("");

#if HAP_DEBUG
    res->setHeader("Content-Type", "application/json");
    res->println("{ \"pairings\": \"deleted\"}");
#endif        

}

// ===========================================================================================================
// /api/restart
// ADMIN only
// ===========================================================================================================
void HAPWebServer::handleApiRestart(HTTPRequest *req, HTTPResponse *res)
{       
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->println("{ \"restart\": \"in 3 sec\" }");
    res->print("");

    delay(3000);
    ESP.restart();
}

// ===========================================================================================================
// /api/reset
// ADMIN only
// ===========================================================================================================
void HAPWebServer::handleApiReset(HTTPRequest *req, HTTPResponse *res)
{   
    LogD("Reset!", true);
    _config->begin();
    _config->save();

    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->println("{ \"restart\": \"in 3 sec\" }");
    res->print("");
    
    delay(3000);
    ESP.restart();
}

// ===========================================================================================================
// /api/setup
// ADMIN only
// ===========================================================================================================
void HAPWebServer::handleApiSetup(HTTPRequest *req, HTTPResponse *res)
{
    // Remove "-" from pincode
    // --< do in webpage
    // String pinCode = _accessorySet->pinCode();
    // pinCode.replace("-", "");

    res->setStatusCode(200);
    res->setStatusText("OK");
    res->setHeader("Content-Type", "application/json");
    res->printf("{ \"pin\": \"%s\", \"xhm\": \"%s\" }\n", _accessorySet->pinCode(), _accessorySet->xhm());
}




/**
 * The following middleware function is one of two functions dealing with access control. The
 * middlewareAuthentication() will interpret the HTTP Basic Auth header, check usernames and password,
 * and if they are valid, set the X-USERNAME and X-GROUP header.
 *
 * If they are invalid, the X-USERNAME and X-GROUP header will be unset. This is important because
 * otherwise the client may manipulate those internal headers.
 *
 * Having that done, further middleware functions and the request handler functions will be able to just
 * use req->getHeader("X-USERNAME") to find out if the user is logged in correctly.
 *
 * Furthermore, if the user supplies credentials and they are invalid, he will receive an 401 response
 * without any other functions being called.
 */
void HAPWebServer::middlewareBasicAuthentication(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{

    // Unset both headers to discard any value from the client
    // This prevents authentication bypass by a client that just sets X-USERNAME
    req->setHeader(HEADER_USERNAME, "");
    req->setHeader(HEADER_GROUP, "");

    // Get login information from request
    // If you use HTTP Basic Auth, you can retrieve the values from the request.
    // The return values will be empty strings if the user did not provide any data,
    // or if the format of the Authorization header is invalid (eg. no Basic Method
    // for Authorization, or an invalid Base64 token)
    std::string reqUsername = req->getBasicAuthUser();
    std::string reqPassword = req->getBasicAuthPassword();

    // If the user entered login information, we will check it
    if (reqUsername.length() > 0 && reqPassword.length() > 0)
    {

        std::string group = "";
        // reqUsername to lowerCase
        transform(reqUsername.begin(), reqUsername.end(), reqUsername.begin(), ::tolower);


        bool authValid = false;

        for (JsonVariant admin : _config->config()["webserver"]["admins"].as<JsonArray>()) {
            if ( ( strcmp(reqUsername.c_str(), admin["username"].as<const char*>() ) == 0)  && ( strcmp(reqPassword.c_str(), admin["password"] ) == 0) ){
                LogD("Set GROUP Header to ADMIN", true);
                group = "ADMIN";
                authValid = true;
                break;
            }
        }

        for (JsonVariant api : _config->config()["webserver"]["apis"].as<JsonArray>()) {
            if ( ( strcmp(reqUsername.c_str(), api["username"].as<const char*>() ) == 0)  && ( strcmp(reqPassword.c_str(), api["password"] ) == 0) ){
                LogD("Set GROUP Header to API", true);
                group = "API";
                authValid = true;
                break;
            }
        }


        // _Very_ simple hardcoded user database to check credentials and assign the group
        // bool authValid = true;

        // // if (reqUsername == "admin" && reqPassword == "secret")        
        // if ( ( strcmp(reqUsername.c_str(), _config->config()["webserver"]["adminName"] ) == 0)  && ( strcmp(reqPassword.c_str(), _config->config()["webserver"]["adminPassword"] ) == 0) )
        // {
        //     group = "ADMIN";
        // }
        // // else if (reqUsername == "user" && reqPassword == "test")
        // else if ( ( strcmp(reqUsername.c_str(), _config->config()["webserver"]["userName"] ) == 0)  && ( strcmp(reqPassword.c_str(), _config->config()["webserver"]["userPassword"] ) == 0) )
        // {
        //     group = "USER";
        // } else {
        //     authValid = false;
        // }

        // If authentication was successful
        if (authValid)
        {
            // set custom headers and delegate control
            req->setHeader(HEADER_USERNAME, reqUsername);
            req->setHeader(HEADER_GROUP, group);

            // The user tried to authenticate and was successful
            // -> We proceed with this request.
            next();
        }
        else
        {
            // Display error page
            res->setStatusCode(401);
            res->setStatusText("Unauthorized");
            res->setHeader("Content-Type", "text/plain");

            // This should trigger the browser user/password dialog, and it will tell
            // the client how it can authenticate
            res->setHeader("WWW-Authenticate", "Basic realm=\"Homekit privileged area\"");

#if HAP_DEBUG
            // Small error text on the response document. In a real-world scenario, you
            // shouldn't display the login information on this page, of course ;-)
            res->println("401. Unauthorized (try admin/secret or api/test)");
#endif
            // NO CALL TO next() here, as the authentication failed.
            // -> The code above did handle the request already.
        }
    }
    else
    {
        // No attempt to authenticate
        // -> Let the request pass through by calling next()
        next();
    }
}

/**
 * This function plays together with the middlewareAuthentication(). While the first function checks the
 * username/password combination and stores it in the request, this function makes use of this information
 * to allow or deny access.
 *
 * This example only prevents unauthorized access to every ResourceNode stored under an /internal/... path.
 */
void HAPWebServer::middlewareBasicAuthorization(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{    
    std::string group = req->getHeader(HEADER_GROUP);

    // admins have access to everything
    if ( group == "ADMIN" ) {
        next();
    } 

    // restrict access for admin api to admins: /api/config, /api/setup, /api/pairings, /api/reset, /api/restart 
    else if ( group != "ADMIN" && (req->getRequestString()  == "/api/config" 
                            || req->getRequestString() == "/api/setup"
                            || req->getRequestString() == "/api/pairings" 
                            || req->getRequestString() == "/api/reset" 
                            || req->getRequestString() == "/api/restart" ) ) {
        // Same as the deny-part in middlewareAuthentication()
        res->setStatusCode(401);
        res->setStatusText("Unauthorized");
        res->setHeader("Content-Type", "text/plain");
        res->setHeader("WWW-Authenticate", "Basic realm=\"Homekit privileged area\"");

#if HAP_DEBUG        
        res->println("401. Unauthorized (try admin/secret or api/test)");
#endif        
    }

    // apis or admins have access to /api
    else if ( (group == "API" || group == "ADMIN") && req->getRequestString().substr(0, 4) == "/api" ) {
        next();
    }
        
    else {

        // Same as the deny-part in middlewareAuthentication()
        res->setStatusCode(401);
        res->setStatusText("Unauthorized");
        res->setHeader("Content-Type", "text/plain");
        res->setHeader("WWW-Authenticate", "Basic realm=\"Homekit privileged area\"");
        
#if HAP_DEBUG        
        res->println("401. Unauthorized (try admin/secret or api/test)");
#endif
    }

#if 0

    // Get the username (if any)
    std::string username = req->getHeader(HEADER_USERNAME);

    // ToDo: Implement all paths correct
    // Check that only logged-in users may get to the internal area (All URLs starting with /internal)
    // Only a simple example, more complicated configuration is up to you.
    if ((username == "" && req->getRequestString().substr(0, 4) == "/api") || (username == "" && req->getRequestString().substr(0, 7) == "/config"))
    {
        // Same as the deny-part in middlewareAuthentication()
        res->setStatusCode(401);
        res->setStatusText("Unauthorized");
        res->setHeader("Content-Type", "text/plain");
        res->setHeader("WWW-Authenticate", "Basic realm=\"Homekit privileged area\"");

#if HAP_DEBUG        
        res->println("401. Unauthorized (try admin/secret or user/test)");
#endif
        // No call denies access to protected handler function.
    }
    else
    {
        // Everything else will be allowed, so we call next()
        next();
    }

#endif


}

#if HAP_WEBSERVER_USE_JWT
/**
 * This function plays together with the middlewareAuthentication(). While the first function checks the
 * username/password combination and stores it in the request, this function makes use of this information
 * to allow or deny access.
 *
 * This example only prevents unauthorized access to every ResourceNode stored under an /internal/... path.
 */
void HAPWebServer::middlewareJWTAuthorization(HTTPRequest *req, HTTPResponse *res, std::function<void()> next)
{

    if (req->getRequestString().substr(0, 4) == "/api")
    {

        // Get the username (if any)
        std::string token = req->getHeader("Authorization");

        // Check that only logged-in users may get to the internal area (All URLs starting with /internal)
        // Only a simple example, more complicated configuration is up to you.
        if (HAPWebServerJWT::verifyToken(token.c_str(), "api", "https://esp32.local", server_publicKey_der_start, server_publicKey_der_end - server_publicKey_der_start) == false)
        {
            // Same as the deny-part in middlewareAuthentication()
            res->setStatusCode(401);
            res->setStatusText("Unauthorized");
            res->setHeader("Content-Type", "text/plain");
            res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32 privileged area\"");
            res->println("401. Unauthorized (try admin/secret or user/test)");

            // No call denies access to protected handler function.
        }
        else
        {
            // Everything else will be allowed, so we call next()
            next();
        }
    }
    else
    {
        // Everything else will be allowed, so we call next()
        next();
    }
}
#endif

std::vector<std::string> HAPWebServer::splitString(std::string data, std::string token)
{
    std::vector<std::string> output;
    size_t pos = std::string::npos; // size_t to avoid improbable overflow
    do
    {
        pos = data.find(token);
        output.push_back(data.substr(0, pos));
        if (std::string::npos != pos)
            data = data.substr(pos + token.size());
    } while (std::string::npos != pos);
    return output;
}


String HAPWebServer::buildNavigation(bool full){

    String result = "";
    result += "";
    result += "<nav class=\"nav\">";
    result += "<ul class=\"menu\">";
    result += "<li class=\"menu-item\"><a href=\"/\">Homekit</a></li>";
    if (full) {
        result += "<li class=\"menu-item\"><a href=\"/wifi\">WiFi</a></li>";
        result += "<li class=\"menu-item\"><a href=\"/config\">Configuration</a></li>";
        result += "<li class=\"menu-item\"><a href=\"/update\">Update</a></li>";
        
        bool infoAdded = false;
        if (_pluginNodes.size() > 0) {

            for (auto &node : _pluginNodes) {              
                if (node->method == "GET") {
                    if (infoAdded == false){
                        result += "<li class=\"menu-item has-children\"><a href=\"\">Plugins<span class=\"dropdown-icon\"></span></a>";
                        result += "<ul class=\"sub-menu\">";
                        infoAdded = true;
                    }         

                    result += "<li class=\"menu-item\"><a href=\"/plugin/" + String(node->path.c_str()) + "\">" + String(node->name.c_str()) + "</a></li>";
                }            
            }
            
            if (infoAdded == true) {
                result += "</ul>";
                result += "</li>";
            }

        }
    }
    result += "</ul>";
    result += "</nav>";

    return result;
}



// ===========================================================================================================
// /plugin/*
// ===========================================================================================================
void HAPWebServer::registerPluginNode(HAPWebServerPluginNode* pluginNode){
    LogD("Adding node to webserver for path: /plugin/" + String(pluginNode->path.c_str()), true);
    _pluginNodes.push_back(std::move(pluginNode));
}

bool HAPWebServer::validatePluginNodesGet(std::string s){
    for (auto &node : _pluginNodes) {         
        if ((node->path == s) && (node->method == "GET")) {
            return true;
        }
    } 
    return false;
}

bool HAPWebServer::validatePluginNodesPost(std::string s){
    for (auto &node : _pluginNodes) {         
        if ((node->path == s) && (node->method == "POST")) {
            return true;
        }
    } 
    return false;
}

void HAPWebServer::handlePluginNodes(HTTPRequest * req, HTTPResponse * res){
   
    // Checking permissions can not only be done centrally in the middleware function but also in the actual request handler.
    // This would be handy if you provide an API with lists of resources, but access rights are defined object-based.
    // Get access to the parameters
    ResourceParameters *params = req->getParams();
    for (auto node : _pluginNodes) { 
        if ( (params->getPathParameter(0) == node->path) && (req->getMethod() == node->method ) )                
        {
            node->callback(req, res);
            return;
        }
    }

}