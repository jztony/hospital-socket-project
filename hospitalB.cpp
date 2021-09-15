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

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "mapanalyzer.h"

#define REMOTE_PORT "33697" // scheduler UDP
#define PORT "31697"        // local UDP port

#define MAXBUFLEN 100
#define NAME "hospitalB"
#define CODE "B"

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

std::string UDP_recv(char const *port) {
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
        return NULL;
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
        return NULL;
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
    //printf("%s: packet contains \"%s\"\n", NAME, buf);
    std::string message(buf);
    // std::cout << "test string: " << test << std::endl;
    
    // std::size_t pos = test.find(": ") + 2;
    std::string variable = message.substr(message.find(": ") + 2);
    // std::cout << "variable: [" << variable << "]" << std::endl;
    close(sockfd);

    return variable;
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

int main(int argc, char * argv[])
{  
    // scheduling variables
    int my_location = std::atoi(argv[1]);
    int capacity = std::atoi(argv[2]);
    int occupancy = std::atoi(argv[3]);
    // std::cout << "my_location is: " << my_location << std::endl;
    // std::cout << "capacity is: " << capacity << std::endl;
    // std::cout << "occupancy is: " << occupancy << std::endl;
    
    int client_location = -1;
    double availability = -1;

    std::stringstream sstm;
    sstm << "Capacity: " << NAME << " " << capacity << " " << occupancy;

    // report availability on start
    UDP_send(sstm.str().c_str(), "localhost", REMOTE_PORT);

    // onscreen message
    std::cout << "Hospital " << CODE << " is up and running using UDP on port " << PORT << std::endl;
    std::cout << "Hospital " << CODE << " has total capacity " << capacity;
    std::cout << " and initial occupancy " << occupancy << "." << std::endl;

    // constantly checking for new requests
    while (true) {

        // receive client location from scheduler
        std::string msg = UDP_recv(PORT);

        // decide what type of message it is
        if (msg == CODE) { // update occupancy
            occupancy++;
            // update availability
            availability = (capacity - occupancy) / (double)capacity;
            std::cout << "Hospital " << CODE << " has been assigned to a client, occupation is updated to ";
            std::cout << occupancy << "​, availability is updated to ​" << availability << std::endl;
        } else { // client request forwarded by scheduler

            // initialize variables
            bool location_not_found = false;
            bool score_success = false;
            double dist = 990000;
            double score = -1;

            std::cout << "Hospital " << CODE << " has received input from client at location " << msg << std::endl;
            client_location = stoi(msg);
            // std::cout << "client location is: " << client_location << std::endl;
        
            // use mapanalyzer to find out distance to calculate score if hospital not full
            std::string score_msg;

            // update availability
            availability = (capacity - occupancy) / (double)capacity;
            std::cout << "Hospital " << CODE << " has capacity = " << capacity;
            std::cout << ", occupation= " << occupancy << ", availability = " << availability << std::endl;

            if (availability > 1 || availability < 0) {
                score_msg = "None"; // availablity > 1 or < 0, something wrong!!
            } else {
                if (client_location != my_location) {
                    double dist_limit = 990000;
                    MapAnalyzer m ("map.txt", my_location, client_location, dist_limit);
                    if (m.inMap(client_location)) {
                        dist = m.getTotalDistance();

                        std::cout << "Hospital " << CODE << " has found the shortest path to client, distance = ​" << dist << std::endl;

                        score = 1 / (dist * (1.1 - availability));

                        std::cout << "Hospital " << CODE << " has the score = ​" << score << std::endl;

                        score_msg = std::to_string(score);
                        score_msg += " ";
                        score_msg += std::to_string(dist);
                        score_success = true;
                    } else {
                        std::cout << "Hospital " << CODE << " does not have the location " << client_location;
                        std::cout << " in map" << std::endl;
                        score_msg = "LNF"; // client location not in the map
                        location_not_found = true;
                    }                
                } else {
                    // std::cout << NAME << "client at current hospital" << std::endl;
                    score_msg = "None"; // client at current hospital
                }
            }

            sstm.str(std::string());
            sstm << "Score and distance: " << NAME << " " << score_msg;
            UDP_send(sstm.str().c_str(), "localhost", REMOTE_PORT);
            if (location_not_found) {
                std::cout << "Hospital " << CODE << " has sent \" location not found\" to the Scheduler" << std::endl;
            }
            if (score_success) {
                std::cout << "Hospital " << CODE << " has sent score = " << score << " and distance = " << dist;
                std::cout << " to the Scheduler" << std::endl;
            } else {
                if (!location_not_found) {
                    std::cout << "Hospital " << CODE << " has sent score = " << "None" << " and distance = " << "None";
                    std::cout << " to the Scheduler" << std::endl;
                }
            }
        }


        // receive allocation from scheduler
        // msg = UDP_recv(PORT);
        // if (msg == NAME) {
        //     occupancy++;
        // }
        // std::cout << NAME << " occupancy is: " << occupancy << std::endl;
    }
    

    return 0;
}