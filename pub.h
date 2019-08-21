#include <list>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
using std::string;





class Sub {
    public:
        string port;
        string ip;
    Sub(string _port, string _ip) {
        port = _port;
        ip = _ip;
    }
};

class Topic {
    public:
        string name;
        list<Sub> subList;
    Topic(string _name) {
        name = _name;
    }
};



class Publisher { /* 구독자 */
    public:
        list<Topic> topicList;
        void addTopic(string, string, string);
        bool isInTopicList(string);
        list<Sub> getSubList(string);
};


void Publisher::addTopic(string name, string port, string ip) {
    list<Topic>::iterator iter = topicList.begin();
    for(iter; iter!=topicList.end(); iter++) {
        if(iter->name.compare(name)==0) {
            iter->subList.push_back(Sub(port,ip));
            return;
        }
    }
    Topic topic(name);
    topic.subList.push_back(Sub(port,ip));
    topicList.push_back(topic);
    return;
}

list<Sub> Publisher::getSubList(string name) {
    list<Topic>::iterator iter = topicList.begin();
    for(iter; iter!=topicList.end(); iter++) {
        if(iter->name.compare(name)==0) {
            return iter->subList;
        }
    }
}

bool Publisher::isInTopicList(string name) {
    list<Topic>::iterator iter = topicList.begin();
    for(iter; iter!=topicList.end(); iter++) {
        if(iter->name.compare(name)==0) {
            return true;
        }
    }
    return false;
}




