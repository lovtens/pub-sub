#include <list>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
using std::string;



class Owner {
    private:
        string ip;
        string port;
    public:
        string getip(void);
        string getport(void);
        void setMatched(void);
        bool matched;
    Owner(string _port, string _ip) {
        ip = _ip;
        port = _port;
        matched = false;
    }
};

class Match {
    public:
        string name;
        string pub_ip;
        string pub_port;
        list<Owner> subList;

    Match(string _name, string _pub_ip, string _pub_port) {
        name = _name;
        pub_ip = _pub_ip;
        pub_port = _pub_port;
    }
};

class Topic {
    private:
        string type; //publish or subscribe
        string name;
    public:
        string gettype(void);
        string getname(void);
        list<Owner> ownerList;
        void addOwner(Owner);
        bool checkOwnerList(Owner);
        
    Topic(string _type, string _name) {
        type = _type;
        name = _name;
    }
};




class MessageBroker {
    private:
        list<Topic> pubTopicList; //원소로 class가 가능한가
        list<Topic> subTopicList;
    public:
        map<string,Match> matchList;
        bool isInTopicList(string,string);
        bool registerTopic(string,string,string,string);
        void testFunction(void);
        void updateMatchList(void);
};

string Topic::gettype(void) {
    return type;
};
string Topic::getname(void) {
    return name;
};

void Topic::addOwner(Owner owner) {
    if(checkOwnerList(owner)) {
        ownerList.push_back(owner);
    } 
    return;
};
bool Topic::checkOwnerList(Owner owner) {
    list<Owner>::iterator iter = ownerList.begin();
    for(iter=ownerList.begin(); iter!=ownerList.end(); iter++) {
        if(!iter->getport().compare(owner.getport()) and !iter->getip().compare(owner.getip())) {
            return false; /* 있음 */
        } 
    } 
    return true; /* 없음 */
};

string Owner::getip(void) {
    return ip;
};
string Owner::getport(void) {
    return port;
};
void Owner::setMatched(void) {
    matched = true;
}

/* 수정필요 */
bool MessageBroker::isInTopicList(string type, string name) {
    if(!type.compare("pub")) {
        /* 리스트 반복자 */
        list<Topic>::iterator iter = pubTopicList.begin();
        for(iter = pubTopicList.begin(); iter!=pubTopicList.end(); iter++) {
            if(!iter->getname().compare(name) and !iter->gettype().compare(type)) {
                return true;
            }
        } 
    } else if(!type.compare("sub")) {
        list<Topic>::iterator iter = pubTopicList.begin();
        for(iter = subTopicList.begin(); iter!=subTopicList.end(); iter++) {
            if(!iter->getname().compare(name) and !iter->gettype().compare(type)) {
                return true;
            }
        }
    }
    return false; /* 리스트에 없으면 */
}

bool MessageBroker::registerTopic(string type, string name, string port, string ip) {
    if(!type.compare("pub")) {
        /* 리스트 반복자 */
        list<Topic>::iterator iter = pubTopicList.begin();
        for(iter = pubTopicList.begin(); iter!=pubTopicList.end(); iter++) {
            if(iter->getname().compare(name) == 0) { /* 있음 */
                return false;
            }
        }
        /* 없음 */
        Topic topic(type,name);
        Owner owner(port,ip);
        topic.addOwner(owner);
        pubTopicList.push_back(topic);

    } else if(!type.compare("sub")) {
        list<Topic>::iterator iter = subTopicList.begin();
        for(iter = subTopicList.begin(); iter!=subTopicList.end(); iter++) {
            if(iter->getname().compare(name) == 0) { /* 있음 */
                list<Owner> l = iter->ownerList;
                list<Owner>::iterator iter2 = l.begin();
                for(iter2; iter2!=l.end(); iter2++) {
                    if(iter2->getip().compare(ip)==0 and iter2->getport().compare(port)==0) { /* 있음 */
                        return false;
                    }
                }
                Owner owner(port,ip);
                iter->addOwner(owner);
                return true;
            }
        }
        /* 없음 */
        Topic topic(type,name);
        Owner owner(port,ip);
        topic.addOwner(owner);
        subTopicList.push_back(topic);
    }
    return true;
};


void MessageBroker::testFunction(void) {
    list<Topic>::iterator iter = pubTopicList.begin();
    for(iter = pubTopicList.begin(); iter!=pubTopicList.end(); iter++) {
        string name = iter->getname();
        string type = iter->gettype();
        list<Owner> tmp = iter->ownerList;
        list<Owner>::iterator iter2 = tmp.begin();
        for(iter2 = tmp.begin(); iter2!=tmp.end(); iter2++) {
            string port = iter2->getport();
            string ip = iter2->getip();
            cout<<type<<" "<<name<<" "<<port<<" "<<ip<<endl;
        }
    } 
};


void MessageBroker::updateMatchList(void) {
    list<Topic>::iterator iter = pubTopicList.begin();

    for(iter = pubTopicList.begin(); iter!=pubTopicList.end(); iter++) {
        string name = iter->getname();
        string pub_ip = iter->ownerList.begin()->getip();
        string pub_port = iter->ownerList.begin()->getport();
        list<Topic>::iterator iter2 = subTopicList.begin();
        for(iter2 = subTopicList.begin(); iter2!=subTopicList.end(); iter2++) {
            if(iter2->getname().compare(name) == 0) {
                if(matchList.find(name) == matchList.end()) { /* Match 없으면 만들어줌*/
                    Match match(name,pub_ip,pub_port);
                    matchList.insert(pair<string,Match>(name,match));
                }
                map<string,Match>::iterator now = matchList.find(name);
                list<Owner>::iterator iter3 = iter2->ownerList.begin();
                for(iter3 = iter2->ownerList.begin(); iter3 != iter2->ownerList.end(); iter3++) { /* sub의 owner들 중에서 unmatched만*/
                    // cout<<"in update name: "<<name<<"matched: "<<iter3->matched<<endl;
                    if(iter3->matched) {
                        continue;
                    } else {
                        iter3->setMatched();
                        now->second.subList.push_back(Owner(iter3->getport(),iter3->getip()));
                    }
                }
            }
        }
    }
}
