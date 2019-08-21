# pub-sub
2019 Network Protocol, Publish and Subscribe Middleware 


Message broker, publisher, subscriber and topic으로 구성된 simple pub-sub middleware 구현
![pubsub](https://user-images.githubusercontent.com/37183417/63412203-33eba180-c432-11e9-895d-890b16587e36.JPG)

## Test Environment
- OS: windows 10
- language: c++
- Compiler: g++(with -lws2_32 option)

## 구현된 기능
- Topic 
  - Topic Type
    - Publish or Subscribe
  - Topic Name
    - Instant name
    - Ex. "Person", "Movie", etc.
  - Topic Message Format은 사전에 알고 있다고 가정

- Message Broker (broker.exe)
  - Publisher와 Subscriber로부터 Topic 수신 및 등록된 Topic 관리
  - Topic owner의 주소(IP, PORT)를 저장
  - Match 관리
  - Match 되는 Topic owner의 정보전달 (Match된 경우 Subscriber에게 Publisher 정보전달)

- Publisher (pub.exe)
  - Publish Topic을 Message Broker에게 전송
  - Matched Subscriber 관리
  - Matched Subscriber에게 Data (text file format) 전송
  - Multi-topic Publish 기능 (Publisher가 여러 개의 다른 Topic 등록 가능, 단 이미 등록된 동일한 이름의 Topic이 Message Broker에 등록되지 않은 경우로 한함)
  
- Subscriber (sub.exe)
  - Subscribe Topic을 Message Broker에게 전송
  - Matched Publisher 관리
  - Matched Publisher로부터 Data (textfile format) 수신
  - Multi-topic Subscribe 기능
  
## Manual
pub.exe sub.exe 실행시 control port number를 받도록 되어있음
broker.exe의 control port number는 36007로 설정

- 구현된 명령어
  - pub.exe
    -	PUB [topic_name] : 연결된 broker에게 해당 Topic 등록을 요청함
    - publish [topic_name] : matched Subscriber에게 해당 Topic 데이터를 publish함
  - sub.exe
    -	SUB [topic_name] : 연결된 broker에게 해당 Topic 등록을 요청함


