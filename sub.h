#include <list>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
using std::string;



class Topic {
    public:
        string name;
        string port;
        string ip;
    Topic(string _name, string _port, string _ip) {
        name = _name;
        port = _port;
        ip = _ip;
    }
};


class Subscriber { /* 구독자 */
    public:
        list<Topic> topicList;
        void addTopic(Topic);
};

void Subscriber::addTopic(Topic topic) {
    topicList.push_back(topic);
};




