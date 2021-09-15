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
#include <string>

#define REMOTE_PORT "34697" // scheduler TCP port

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// std::string read_buf(char buf[]) {
//   std::string result;
//   int buf_size = sizeof(buf)/sizeof(buf[0]);
//   printf("buf size is %d\n", buf_size);
//   for (int i = 0; i < buf_size; i++) {
//     if (buf[i] == NULL) {
//       if (buf[i+1] == NULL) {
//         break;
//       } else {
//         result += buf[i];
//       }
//     } else {
//       result += buf[i];
//     }
//   }
//   return result;
// }

int main(int argc, char * argv[])
{
  // std::cout << "Hello World!" << std::endl;

  int status;
  int sockfd;
  struct addrinfo hints;
  struct addrinfo *res, *p;
  char s[INET6_ADDRSTRLEN];
  // will point to the results
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_INET;
  // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  // get ready to connect

  // var for recv
  int buf_len = 128;
  char buf[buf_len];

  if ((status = getaddrinfo("localhost", REMOTE_PORT, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  // sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  // connect(sockfd, res->ai_addr, res->ai_addrlen);

  // loop through all the results and connect to the first we can
  for(p = res; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1) {
          perror("client: socket");
          continue;
      }

      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
          // close(sockfd);
          perror("client: connect");
          continue;
      }

      break;
  }

  // handle connection failure
  if (p == NULL) {
    std::cout << "client: failed to connect" << std::endl;
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  // show connected server
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
  s, sizeof s);
  // std::cout << "client: connecting to " << s << std::endl;

  freeaddrinfo(res); // free the linked-list

  std::cout << "The client is up and running" << std::endl;

  // send info and request
  int my_location = atoi(argv[1]);
  std::stringstream sstm;
  sstm << "Client at: " << my_location;
  const std::string tmp = sstm.str();
  // char const *client_msg = tmp.c_str();
  // char *client_msg = "Client at: ";
  
  char const *msg = tmp.c_str(); // use 'const' to avoid warning

  // OR do this:
  // char msg[] = "Client at: ";
  // strcat(msg, argv[1]);

  // test: pure text
  // char msg[4];
  // msg[0] = '1';
  // msg[1] = '2';
  // msg[2] = '3';
  // msg[3] = '\0';
  // printf("[%s]\n", msg);

  int len, bytes_sent;

  len = strlen(msg);
  // std::cout << "length of msg is: " << len << std::endl;
  // msg[len - 1] = '\0'; // no need to null terminate here
  bytes_sent = send(sockfd, msg, len, 0);
  if (bytes_sent == -1) {
    perror("send");
  }

  std::cout << "The client has sent query to Scheduler using TCP: client location " << my_location << std::endl;

  // wait for response from scheduler

  while (true) {
    int numbytes = recv(sockfd, &buf, sizeof(buf) - 1, 0);
    if (numbytes < 0) {
      perror("recv");
      continue;
    } else {
      buf[numbytes] = '\0'; // important, otherwise will have some garble
      // printf("numbytes is %d\n", numbytes);
      // try read_buf
      std::string message(buf);
      std::string allocation = message.substr(message.find(": ") + 2);
      std::string allocation_msg;
      if (allocation == "LNF") {
        allocation_msg = "None";
      } else {
        allocation_msg = allocation;
      }
      
      std::cout << "The client has received results from the Scheduler: assigned to Hospital ​" << allocation_msg << std::endl;
      if (allocation == "LNF") {
        std::cout << "Location " << my_location << " not found" << std::endl;
      } else if (allocation == "None") {
        std::cout << "Score = None, No assignment" << std::endl;
      }
      // close(sockfd);
      break; // terminate after receiving response
    }
  }

  close(sockfd);
  return 0;

}