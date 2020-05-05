/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Winlin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SRS_APP_JANUS_HPP
#define SRS_APP_JANUS_HPP

#include <srs_core.hpp>

#include <srs_http_stack.hpp>

#include <map>
#include <string>
#include <vector>

class SrsRtcServer;
class SrsJsonObject;
class SrsJanusServer;
class SrsJanusSession;
class SrsJanusCall;
class SrsRequest;
class SrsSdp;

struct SrsJanusMessage
{
    // The janus action field.
    std::string janus;

    // The common request header, except polling.
    std::string transaction;
    std::string client_tid;
    std::string rpcid;
    std::string source_module;

    // For janus event.
    // {"janus": "event", "session_id": xxx, "sender": xxx, "transaction": "xxx", "plugindata":
    //      {"plugin": "janus.plugin.videoroom", "data": {"videoroom": "xxx", "id": xxx, "private_id": xxx}}
    // }
    uint64_t session_id;
    uint64_t sender;
    std::string plugin;
    std::string videoroom;
    // For video-room, joined.
    uint64_t feed_id;
    uint32_t private_id;
    // For video-room, configured.
    std::string jsep_type;
    std::string jsep_sdp;

    SrsJanusMessage() {
        session_id = sender = feed_id = 0;
        private_id = 0;
    }
};

class SrsGoApiRtcJanus : public ISrsHttpHandler
{
private:
    SrsJanusServer* janus;
public:
    SrsGoApiRtcJanus(SrsJanusServer* j);
    virtual ~SrsGoApiRtcJanus();
public:
    virtual srs_error_t serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r);
private:
    srs_error_t do_serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, SrsJsonObject* res);
};

class SrsJanusServer
{
private:
    SrsRtcServer* rtc;
    std::map<uint64_t, SrsJanusSession*> sessions;
public:
    SrsJanusServer(SrsRtcServer* r);
    virtual ~SrsJanusServer();
public:
    srs_error_t listen_api();
public:
    srs_error_t create(SrsJsonObject* req, SrsJanusMessage* msg, SrsJsonObject* res);
    SrsJanusSession* fetch(uint64_t sid);
    SrsRtcServer* server();
};

class SrsJanusSession
{
public:
    std::string appid;
    std::string channel;
    std::string userid;
    std::string sessionid;
private:
    SrsJanusServer* janus;
    std::map<uint64_t, SrsJanusCall*> calls;
    std::vector<SrsJanusMessage*> msgs;
public:
    uint64_t id;
    int cid;
public:
    SrsJanusSession(SrsJanusServer* j);
    virtual ~SrsJanusSession();
public:
    srs_error_t polling(SrsJsonObject* req, SrsJsonObject* res);
    void enqueue(SrsJanusMessage* msg);
public:
    srs_error_t attach(SrsJsonObject* req, SrsJanusMessage* msg, SrsJsonObject* res);
    SrsJanusCall* fetch(uint64_t sid);
    SrsRtcServer* server();
};

class SrsJanusCall
{
private:
    SrsRtcServer* server_;
    SrsJanusSession* session;
public:
    std::string callid;
    uint64_t id;
public:
    SrsJanusCall(SrsJanusSession* s);
    virtual ~SrsJanusCall();
public:
    srs_error_t message(SrsJsonObject* req, SrsJanusMessage* msg);
    srs_error_t trickle(SrsJsonObject* req, SrsJanusMessage* msg);
private:
    srs_error_t on_join_message(SrsJsonObject* req, SrsJanusMessage* msg);
    srs_error_t on_configure_message(SrsJsonObject* req, SrsJsonObject* body, SrsJanusMessage* msg);
    srs_error_t exchange_sdp(SrsRequest* req, const SrsSdp& remote_sdp, SrsSdp& local_sdp);
};

#endif