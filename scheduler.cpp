#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>

// local (scheduler) ports
#define TCP_PORT "34697"
#define UDP_PORT "33697"

// hospital ports
#define HA_PORT "30697"
#define HB_PORT "31697"
#define HC_PORT "32697"

#define BACKLOG 10
#define MAXBUFLEN 100
#define NAME "scheduler"
#define HOS_NUM 3

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int UDP_recv(char const *port) {
    int sockfd;
    int rv;
    int numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        printf(NAME);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf(NAME);
            perror(": socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf(NAME);
            perror(": bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "%s: failed to bind socket\n", NAME);
        return 2;
    }

    freeaddrinfo(servinfo);

    //printf("%s: waiting to recvfrom...\n", NAME);

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

    // printf("%s: got packet from %s\n", NAME,
    //     inet_ntop(their_addr.ss_family,
    //         get_in_addr((struct sockaddr *)&their_addr),
    //         s, sizeof s));
    // printf("%s: packet is %d bytes long\n", NAME, numbytes);
    buf[numbytes] = '\0';
    printf("%s: packet contains \"%s\"\n", NAME, buf);

    close(sockfd);

    return 0;
}

std::vector<std::string> UDP_recv_multi(char const *port, int amount, bool init) {
    
    if (amount == 0) {
        return {};
    }

    int sockfd;
    int rv;
    int numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    std::vector<std::string> results;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        printf(NAME);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return {};
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf(NAME);
            perror(": socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf(NAME);
            perror(": bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "%s: failed to bind socket\n", NAME);
        return {};
    }

    freeaddrinfo(servinfo);

    int recv_cnt = 0; // counter for the 3 expected messages
    while (recv_cnt < amount) {
        //printf("%s: waiting to recvfrom...\n", NAME);

        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);

        // printf("%s: got packet from %s\n", NAME,
        //     inet_ntop(their_addr.ss_family,
        //         get_in_addr((struct sockaddr *)&their_addr),
        //         s, sizeof s));
        // printf("%s: packet is %d bytes long\n", NAME, numbytes);
        buf[numbytes] = '\0';
        // printf("%s: packet contains \"%s\"\n", NAME, buf);

        std::string message (buf);
        if (init) { // print on screen message for initialize phase

            std::vector<std::string> temp;
            std::istringstream iss(message.substr(message.find(": ") + 2));
            copy(std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                back_inserter(temp));

            temp.at(0) = temp.at(0).substr(temp.at(0).find("hospital") + 8);
            // std::cout << "CODE extracted is: " << temp.at(0) << std::endl;

            std::cout << "The Scheduler has received information from Hospital " << temp.at(0);
            std::cout << ": total capacity is " << temp.at(1) << " and initial occupancy is ";
            std::cout << temp.at(2) << std::endl;
        } else { // print on screen message for receive phase
            std::vector<std::string> temp;
            std::istringstream iss(message.substr(message.find(": ") + 2));
            copy(std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                back_inserter(temp));

            temp.at(0) = temp.at(0).substr(temp.at(0).find("hospital") + 8);
            // std::cout << "CODE extracted is: " << temp.at(0) << std::endl;

            std::string score;
            std::string distance;
            if (temp.at(1) != "None" && temp.at(1)!= "LNF") {
                score = temp.at(1);
                distance = temp.at(2);
            } else {
                score = "None";
                distance = "None";
            }

            std::cout << "The Scheduler has received map information from Hospital " << temp.at(0);
            std::cout << ", the score = " << score << " and the distance = ";
            std::cout << distance << std::endl;
        }

        results.push_back(message);
        recv_cnt++; // increment counter
    }

    // printf("%s: Done receiving %d UDP packets.\n", NAME, recv_cnt);

    close(sockfd);

    return results;
}

int UDP_send(char const *msg, char const *addr, char const *port) {
    int sockfd;
    int rv;
    int numbytes;
    struct addrinfo hints, *servinfo, *p;
    // struct sockaddr_storage their_addr;
    // char buf[MAXBUFLEN];
    // socklen_t addr_len;
    // char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf(NAME);
            perror(": socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "%s: failed to bind socket\n", NAME);
        return 2;
    }

    freeaddrinfo(servinfo);

    // char const *msg = "Assigned patient to Hospital A!"; // use 'const' to avoid warning
    int len;
    len = strlen(msg);

    if ((numbytes = sendto(sockfd, msg, len, 0,
              p->ai_addr, p->ai_addrlen)) == -1) {
        printf(NAME);
        perror(": sendto");
        exit(1);
    }

    // printf("%s: UDP packet sent to %s.\n", NAME, port);

    close(sockfd);

    return 0;
}

void printVector(std::vector<int> v) {
    for (std::vector<int>::iterator i = v.begin(); i != v.end(); ++i) {
        std::cout << *i << ' ';
    }
    std::cout << std::endl;
}

void printVector(std::vector<std::string> v) {
    for (std::vector<std::string>::iterator i = v.begin(); i != v.end(); ++i) {
        std::cout << *i << ' ';
    }
    std::cout << std::endl;
}

void printVector(std::vector<double> v) {
    for (std::vector<double>::iterator i = v.begin(); i != v.end(); ++i) {
        std::cout << *i << ' ';
    }
    std::cout << std::endl;
}

// set capacity with received information in msg
void setHospitalInfo(std::vector<std::string> const msg, 
                    std::vector<int>* capacity, 
                    std::vector<int>* occupancy) {

    for (int i = 0; i < msg.size(); i++) {

        std::vector<std::string> temp;
        std::istringstream iss(msg.at(i).substr(msg.at(i).find(": ") + 2));
        copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            back_inserter(temp));

        // for testing purpose
        // std::cout << "temp is now: " << std::endl;
        // printVector(temp);

        if (temp.at(0) == "hospitalA") {
            capacity->at(0) = std::stoi(temp.at(1));
            occupancy->at(0) = std::stoi(temp.at(2));
        }
        else if (temp.at(0) == "hospitalB") {
            capacity->at(1) = std::stoi(temp.at(1));
            occupancy->at(1) = std::stoi(temp.at(2));
        }
        else if (temp.at(0) == "hospitalC") {
            capacity->at(2) = std::stoi(temp.at(1));
            occupancy->at(2) = std::stoi(temp.at(2));
        }
        
    }
}

std::string setScoreDistance(std::vector<std::string> const msg, std::vector<double>* scores, std::vector<double>* distances) {

    // std::vector<std::string> status;
    // status.push_back(std::string());
    // status.push_back(std::string());
    // status.push_back(std::string());

    int none_cnt = 0;

    for (int i = 0; i < msg.size(); i++) {
        std::vector<std::string> temp;
        std::istringstream iss(msg.at(i).substr(msg.at(i).find(": ") + 2));
        copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            back_inserter(temp));

        if (temp.at(1) == "None") {
            none_cnt++;
        } else if (temp.at(1) == "LNF") {
            return "LNF";
        } else {
            if (temp.at(0) == "hospitalA") {
                scores->at(0) = std::stod(temp.at(1));
                distances->at(0) = std::stod(temp.at(2));
            }
            else if (temp.at(0) == "hospitalB") {
                scores->at(1) = std::stod(temp.at(1));
                distances->at(1) = std::stod(temp.at(2));
            }
            else if (temp.at(0) == "hospitalC") {
                scores->at(2) = std::stod(temp.at(1));
                distances->at(2) = std::stod(temp.at(2));
            }

        }

    }

    if (none_cnt == HOS_NUM) {
        return "None";
    } else {
        return "Success";
    }
    
}

// hospitalA: n = 1, HospitalB: n = 2 ...
// cause of the vector index out of bound error
void logOccupancy(int n, std::vector<int>* occupancy) {
    if (n >= 0) {
        occupancy->at(n)++;
    }
}

std::string selectHospital(std::vector<double>* const scores, std::vector<double>* const distances, std::vector<int>* occupancy) {
    int index = -1; 
    double max = -1;
    for (int i = 0; i < HOS_NUM; i++) {
        if (scores->at(i) > max) {
            index = i;
            max = scores->at(i);
        }
    }

    // if tie, select shortest distance
    if (index == -1) {
        double min = 990000;
        for (int i = 0; i < HOS_NUM; i++) {
            if (distances->at(i) < min) {
                index = i;
                min = distances->at(i);
            }
        }
    }

    logOccupancy(index, occupancy);

    if (index == 0) {
        return "A";
    } else if (index == 1) {
        return "B";
    } else if (index == 2) {
        return "C";
    } else {
        return "None";
    }
    
}

int main()
{

    std::cout << "The Scheduler is up and running." << std::endl;
    // std::cout << "Hello World!" << std::endl;
    std::vector<std::string> msg = UDP_recv_multi(UDP_PORT, HOS_NUM, true); // recv capacity reports
    // std::cout << "init is now: " << std::endl;


  // scheduling related variables
  std::vector<int>* capacity = new std::vector<int>();
  std::vector<int>* occupancy = new std::vector<int>();
  std::vector<double>* scores = new std::vector<double>();
  std::vector<double>* distances = new std::vector<double>();
  capacity->push_back(-1);
  capacity->push_back(-1);
  capacity->push_back(-1);
  occupancy->push_back(-1);
  occupancy->push_back(-1);
  occupancy->push_back(-1);
  scores->push_back(-1);
  scores->push_back(-1);
  scores->push_back(-1);
  distances->push_back(990000);
  distances->push_back(990000);
  distances->push_back(990000);
  setHospitalInfo(msg, capacity, occupancy);
//   *capacity = {0, 0, 0};
//   *occupancy = {0, 0, 0};

  int backlog = 10;
  int numbytes;

  int status; // getaddrinfo status
  struct addrinfo hints;
  struct addrinfo *servinfo;
  int sockfd; // socket
  int new_fd;
  char s[INET6_ADDRSTRLEN];

  // var for accept
  struct sockaddr_storage their_addr;
  socklen_t addr_size;

  // var for recv
  int buf_len = 128;
  char buf[buf_len];
//   socklen_t addr_len;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // ipv4
  hints.ai_socktype = SOCK_STREAM;  // TCP
  hints.ai_flags = AI_PASSIVE;      // my IP

  // handle possible err
  if ((status = getaddrinfo(NULL, TCP_PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

  bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);

  freeaddrinfo(servinfo); // free the linked-list

  listen(sockfd, backlog);

  //printf("server: waiting for connections...\n");

  addr_size = sizeof their_addr;



  // init. with hospitals
  // boot-up
//   std::cout << "bootup " << std::endl;
  
    

// for test
    // std::cout << "capacity is now: " << std::endl;
    // printVector(*capacity);
    // std::cout << "occupancy is now: " << std::endl;
    // printVector(*occupancy);


  // notice: 
  // new_fd is child process
  // sockfd is parent process

  while (true) {  // main accept() loop

    // clear scores and distances
    for (int i = 0; i < scores->size(); i++) {
        scores->at(i) = -1;
        distances->at(i) = 990001;
    }

    // now accept an incoming connection:
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size); // child process

    if (new_fd == -1) {
        perror("accept");
        continue;
    }

    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);
    // printf("server: got connection from %s\n", s);

    numbytes = recv(new_fd, &buf, sizeof(buf) - 1, 0);
    if (numbytes < 0) { // recv from new_fd (child process)
      perror("recv err");
      continue;
    }

    buf[numbytes] = '\0'; // null terminate the message end in the buf IMPORTANT! otherwise will have garbage
    // std::cout << "from client: " << buf << std::endl;
    // test: send back recv'd info (echo)



    /* START later stage UDP comm. w/ hospitals */

    // read in client location from message
    std::string message(buf);
    int client_location = std::stod(message.substr(message.find(": ") + 2));

    std::cout << "The Scheduler has received client at location " << client_location;
    std::cout << " from the client using TCP over port " << TCP_PORT << std::endl;

    std::stringstream sstm;
    sstm << "client at location: " << client_location;

    // send client location to hospitals that are not full yet

    int response_num = 0; // record how many responses to receive

    // only send when not full
    if (capacity->at(0) > occupancy->at(0)) {
        UDP_send(sstm.str().c_str(), "localhost", HA_PORT);
        response_num++;
        std::cout << "The Scheduler has sent client location to Hospital A using UDP over port ";
        std::cout << UDP_PORT << std::endl;
    }

    if (capacity->at(1) > occupancy->at(1)) {
        UDP_send(sstm.str().c_str(), "localhost", HB_PORT);
        response_num++;
        std::cout << "The Scheduler has sent client location to Hospital B using UDP over port ";
        std::cout << UDP_PORT << std::endl;
    }

    if (capacity->at(2) > occupancy->at(2)) {
        UDP_send(sstm.str().c_str(), "localhost", HC_PORT);
        response_num++;
        std::cout << "The Scheduler has sent client location to Hospital C using UDP over port ";
        std::cout << UDP_PORT << std::endl;
    }
    
    // UDP_send(sstm.str().c_str(), "localhost", HB_PORT);

    std::string allocation = "None";
    std::string client_output = "None";

    if (response_num > 0) {
        msg = UDP_recv_multi(UDP_PORT, response_num, false); // recv hospital scores
        client_output = setScoreDistance(msg, scores, distances);
        allocation = selectHospital(scores, distances, occupancy);
        // std::cout << "occupancy are: " << std::endl;
        // printVector(*occupancy);
    }    

    // std::cout << "scores are: " << std::endl;
    // printVector(*scores);
    
    // std::cout << "distances are: " << std::endl;
    // printVector(*distances);

    std::cout << "The Scheduler has assigned Hospital " << allocation << "​ to the client" << std::endl;

    sstm.str(std::string());

    sstm << "client assigned to: " << allocation;
    //std::cout << sstm.str() << std::endl;

    if (client_output == "LNF") {
        allocation = "LNF";
    }

    // send client allocation to selected hospital ONLY
    if (allocation == "A") {
        UDP_send(sstm.str().c_str(), "localhost", HA_PORT);
        std::cout << "The Scheduler has sent the result to Hospital ​A​ using UDP over port ​" << UDP_PORT << std::endl;
    } else if (allocation == "B") {
        UDP_send(sstm.str().c_str(), "localhost", HB_PORT);
        std::cout << "The Scheduler has sent the result to Hospital ​B using UDP over port ​" << UDP_PORT << std::endl;
    } else if (allocation == "C") {
        UDP_send(sstm.str().c_str(), "localhost", HC_PORT);
        std::cout << "The Scheduler has sent the result to Hospital ​C​ using UDP over port ​" << UDP_PORT << std::endl;
    } else {
        // HOSPITALS ALL FULL OR ILLEGAL CLIENT LOCATION OR DISTANCE TIE
    }
    
    
    // UDP_send(sstm.str().c_str(), "localhost", HB_PORT);
    // UDP_send(sstm.str().c_str(), "localhost", HC_PORT);

    // UDP_recv(UDP_PORT);

    /* END UDP comm. w/ hospitals */


    /* now use TCP to notify patient about the assignment result */
    sstm.str(std::string());
    sstm << "You are assigned to: " << allocation;
    const std::string tmp = sstm.str();
    char const *client_msg = tmp.c_str();
    int msg_len = strlen(client_msg);
    // printf("strlen is %d, should be 25", msg_len);
    // (char)(client_msg + msg_len) = '\0';

    if (send(new_fd, client_msg, msg_len, 0) == -1) {
      perror("send");
    }

    std::cout << "The Scheduler has sent the result to client using TCP over port " << TCP_PORT << std::endl;
  }

  std::cout << "Client disconnected" << std::endl;

  return 0;

}