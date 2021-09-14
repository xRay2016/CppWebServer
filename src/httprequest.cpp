/*
 * @Author: your name
 * @Date: 2021-09-09 15:27:29
 * @LastEditTime: 2021-09-14 15:04:51
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cpp_server/src/httprequest.cpp
 */
#include "httprequest.h"
using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML={
    "/index", "/register", "/login",
     "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
    {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::init()
{
    method_=path_=version_=body_="";
    state_=REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::isKeepAlive() const
{
    if(header_.count("Connection")==1)
    {
        return header_.find("Connection")->second=="keep-aliv" &&version_=="1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer& buff)
{
    const char CRLF[]="\r\n";
    if(buff.ReadableBytes()<=0){
        return false;
    }
    while(buff.ReadableBytes() && state_!=FINISH){
        const char* linend=search(buff.Peek(),buff.BeginWriteConst(),CRLF,CRLF+2);
        std::string line(buff.Peek(),linend);
        cout<<line<<endl;
        switch(state_)
        {
        case REQUEST_LINE:
            if(!parseRequestLine_(line)){
                return false;
            }
            parsePath_();
            break;
        case HEADERS:
            parseHeader_(line);
            if(buff.ReadableBytes()<=2){
                state_=FINISH;
            }
            break;
        case BODY:
            parseBody_(line);
            break;
        default:
            break;
        }
        if(linend==buff.BeginWrite()){
            break;
        }
        buff.RetrieveUntil(linend+2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

void HttpRequest::parsePath_()
{
    if(path_=="/"){
        path_="/index.html";
    }
    else{
        for(auto& item:DEFAULT_HTML){
            if(item==path_){
                path_+=".html";
                break;
            }
        }
    }
}

bool HttpRequest::parseRequestLine_(const string& line)
{
    try{
        regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
        smatch subMatch;
        if(regex_match(line,subMatch,pattern)){
            method_ = subMatch[1];
            path_ = subMatch[2];
            version_ = subMatch[3];
            state_ = HEADERS;
            return true;
        }
    }
    catch(const std::regex_error& e){
        std::cout << "regex_error caught: " << e.what() << '\n';
    }
    
    
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::parseHeader_(const string& line)
{
    regex pattern("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line,subMatch,pattern))
    {
        header_[subMatch[1]]=subMatch[2];
    }
    else{
        state_=BODY;
    }
}

void HttpRequest::parseBody_(const string& line)
{
    body_=line;
    parsePost_();
    state_=FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}


int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpRequest::parsePost_()
{
    if(method_=="POST" && header_["Content-Type"]=="application/x-www-form-urlencoded")
    {
        parseFromcoded_();
        if(DEFAULT_HTML_TAG.count(path_))
        {
            int tag=DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d",tag);
            if(tag==0 || tag==1){
                bool isLogin=(tag==1);
                if(UserVerify(post_["username"],post_["password"],isLogin))
                {
                    path_="/welcome.html";
                }
                else{
                    path_="/error.html";
                }
            }
        }
    }
}

void HttpRequest::parseFromcoded_() {
    if(body_.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::UserVerify(const string& name,const string& pwd,bool isLogin)
{
    if(name==""||pwd=="")
    {
        return false;
    }
    LOG_INFO("Verify name:%s pwd:%s",name.c_str(),pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(sql,SqlConnPool::Instance());
    assert(sql);
    

    bool flag=false;
    char order[256]={0};
    MYSQL_FIELD * fields=nullptr;
    MYSQL_RES* res=nullptr;

    
    snprintf(order,256,"SELECT username, password FROM user WHERE username='%s' LIMIT 1",name.c_str());
    LOG_DEBUG("%s",order);

    if(mysql_query(sql,order)){
        mysql_free_result;
        return false;
    }

    res=mysql_store_result(sql);
    unsigned int num_fields=mysql_num_fields(res);
    fields=mysql_fetch_fields(res);
    flag=true;
    
    while(MYSQL_ROW row=mysql_fetch_row(res)){
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);
        if(isLogin){
            if(password==pwd){
                flag=true;
            }
            else{
                flag=false;
                LOG_DEBUG("pwd error!");
            }
        }
        else{
            flag=false;
            LOG_DEBUG("user used");
        }
    }

    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

std::string HttpRequest::path() const{
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::getPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::getPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}


