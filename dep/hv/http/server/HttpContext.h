#ifndef HV_HTTP_CONTEXT_H_
#define HV_HTTP_CONTEXT_H_

#include "hexport.h"
#include "HttpMessage.h"
#include "HttpResponseWriter.h"

struct HttpService;

namespace hv {

struct HV_EXPORT HttpContext {
    HttpService*            service;
    HttpRequestPtr          request;
    HttpResponsePtr         response;
    HttpResponseWriterPtr   writer;

    // HttpRequest aliases
    // return request->xxx
    std::string ip() {
        return request->client_addr.ip;
    }

    http_method method() {
        return request->method;
    }

    std::string url() {
        return request->url;
    }

    std::string path() {
        return request->Path();
    }

    std::string host() {
        return request->Host();
    }

    const http_headers& headers() {
        return request->headers;
    }

    std::string header(const char* key, const std::string& defvalue = "") {
        return request->GetHeader(key, defvalue);
    }

    const QueryParams& params() {
        return request->query_params;
    }

    std::string param(const char* key, const std::string& defvalue = "") {
        return request->GetParam(key, defvalue);
    }

    int length() {
        return request->ContentLength();
    }

    http_content_type type() {
        return request->ContentType();
    }

    bool is(http_content_type content_type) {
        return request->content_type == content_type;
    }

    bool is(const char* content_type) {
        return request->content_type == http_content_type_enum(content_type);
    }

    std::string& body() {
        return request->body;
    }

#ifndef WITHOUT_HTTP_CONTENT
    const hv::Json& json() {
        // Content-Type: application/json
        if (request->content_type == APPLICATION_JSON &&
            request->json.empty() &&
            !request->body.empty()) {
            request->ParseBody();
        }
        return request->json;
    }

    const MultiPart& form() {
        // Content-Type: multipart/form-data
        if (request->content_type == MULTIPART_FORM_DATA &&
            request->form.empty() &&
            !request->body.empty()) {
            request->ParseBody();
        }
        return request->form;
    }

    const hv::KeyValue& urlencoded() {
        // Content-Type: application/x-www-form-urlencoded
        if (request->content_type == X_WWW_FORM_URLENCODED &&
            request->kv.empty() &&
            !request->body.empty()) {
            request->ParseBody();
        }
        return request->kv;
    }

    // T=[bool, int, int64_t, float, double]
    template<typename T>
    T get(const char* key, T defvalue = 0) {
        return request->Get(key, defvalue);
    }
    std::string get(const char* key, const std::string& defvalue = "") {
        return request->GetString(key, defvalue);
    }
#endif

    // HttpResponse aliases
    // response->xxx = xxx
    void setStatus(http_status status) {
        response->status_code = status;
    }

    void setContentType(http_content_type type) {
        response->content_type = type;
    }

    void setContentType(const char* type) {
        response->content_type = http_content_type_enum(type);
    }

    void setHeader(const char* key, const std::string& value) {
        response->headers[key] = value;
        if (stricmp(key, "Content-Type") == 0) {
            setContentType(value.c_str());
        }
    }

    void setBody(const std::string& body) {
        response->body = body;
    }

    // response->sendXxx
    int send() {
        writer->End();
        return response->status_code;
    }

    int send(const std::string& str, http_content_type type = APPLICATION_JSON) {
        response->content_type = type;
        response->body = str;
        return send();
    }

    int sendString(const std::string& str) {
        response->String(str);
        return send();
    }

    int sendData(void* data, int len, bool nocopy = true) {
        response->Data(data, len, nocopy);
        return send();
    }

    int sendFile(const char* filepath) {
        response->File(filepath);
        return send();
    }

#ifndef WITHOUT_HTTP_CONTENT
    // T=[bool, int, int64_t, float, double, string]
    template<typename T>
    void set(const char* key, const T& value) {
        response->Set(key, value);
    }

    // @see     HttpMessage::Json
    // @usage   https://github.com/nlohmann/json
    template<typename T>
    int sendJson(const T& t) {
        response->Json(t);
        return send();
    }
#endif

};

} // end namespace hv

typedef std::shared_ptr<hv::HttpContext> HttpContextPtr;

#endif // HV_HTTP_CONTEXT_H_
