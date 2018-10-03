#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
//static const char *host_hdr_format = "Host: %s\r\n";
//static const char *requestlint_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

void doit(int fd);
void parse_url(char *url, char *host, char *filename, int *port);
void clienterror(int fd, char *cause, char *errnum, 
         char *shortmsg, char *longmsg);
void send_header(char *host, char *filename, int port, rio_t *rp, int fd);
void *thread(void *vargp);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    pthread_t tid;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        //doit(connfd);                                             //line:netp:tiny:doit
        //Close(connfd);                                            //line:netp:tiny:close
        Pthread_create(&tid, NULL, thread, (void *) &connfd);
    }
}
/* $end tinymain */

void *thread(void *vargp) {
    int connfd = *(int *) vargp;

    Pthread_detach(pthread_self());
    
    doit(connfd);
    Close(connfd);
}

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int clientfd, port;
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], host[MAXLINE], p[MAXLINE];
    rio_t rio, rioGot;
    size_t n;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    //printf("%s", buf);

    sscanf(buf, "%s %s %s\r\n", method, url, version);

    if (strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return;
    }

    //if (strcasecmp(version, "HTTP/1.0") || strcasecmp(method, "HTTP/1.1")) {
    //    clienterror(fd, version, "505", "Not Supported",
    //                "Proxy does not support this version");
    //    return;
    //}

    parse_url(url, host, filename, &port);

    sprintf(p, "%d", port);
    if ((clientfd = Open_clientfd(host, p)) == -1) {
        clienterror(fd, method, "404", "Bad request",
                    "Host name is invalid");
        return;
    }

    Rio_readinitb(&rioGot, clientfd);

    send_header(host, filename, port, &rio, clientfd);

    while ((n = Rio_readlineb(&rioGot, buf, MAXLINE)) != 0) {
        printf("Proxy server received %ld bytes and sent back\n", n);
        Rio_writen(fd, buf, n);
    }

    Close(clientfd);
}
/* $end doit */

/*
 * parse_url - parse URL into filename
 */
/* $begin parse_url */
void parse_url(char *url, char *host, char *filename, int *port) 
{
    char *hostPos, *filePos, *portPos;

    hostPos = strstr(url, "//");

    hostPos = (hostPos == NULL) ? url : (hostPos + 2);

    portPos = strstr(hostPos, ":");

    filePos = strstr(hostPos, "/");

    if (filePos == NULL)
        *filename = '/';
    else {
        sscanf(filePos, "%s", filename);
        *filePos = '\0';
    }

    if (portPos == NULL)
        *port = 80;
    else {
        *portPos = '\0';
        sscanf(portPos + 1, "%d", port);
    }

    sscanf(hostPos, "%s", host);
}
/* $end parse_url */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
         char *shortmsg, char *longmsg) 
{
   char body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body);

    ///* Print the HTTP response */
    //sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    //Rio_writen(fd, buf, strlen(buf));
    //sprintf(buf, "Content-type: text/html\r\n");
    //Rio_writen(fd, buf, strlen(buf));
    //sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    //Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */

void send_header(char *host, char *filename, int port, rio_t *rp, int fd) {
    char buf[MAXLINE], message[MAXLINE], hostbuf[MAXLINE];
    sprintf(hostbuf, "Host: %s\r\n", host);

    while (Rio_readlineb(rp, buf, MAXLINE) > 0) {
        if (!strcmp(buf, endof_hdr))
            break;

        if (!strncasecmp(buf, "Host:", 5)) {
            sprintf(hostbuf, "%s", buf);
        }
        //sprintf(message, "%s%s", message, buf);
    }

    sprintf(message, "GET %s HTTP/1.0\r\n", filename);
    sprintf(message, "%s%s", message, hostbuf);
    sprintf(message, "%s%s", message, user_agent_hdr);
    sprintf(message, "%s%s%s", message, conn_hdr, prox_hdr);
    sprintf(message, "%s%s", message, endof_hdr);
    Rio_writen(fd, message, strlen(message));
}
