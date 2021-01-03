/*
server side
*/
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>

int current_stations=3;
int Multi_Port[]={5000,5001,5002,5003,5004};
int Info_Port[]={4000,4001,4002,4003,4004};
char* Radio[]={"230.198.1.20","230.198.1.21","230.198.1.22","230.198.1.23","230.198.1.24"}; 
char* video_name[]={"movie1.mp4","movie2.mp4", "movie3.mp4","movie4.mp4","movie5.mp4"};
char* duration[]={"timeduration1.txt","timeduration2.txt", "timeduration3.txt","timeduration4.txt","timeduration5.txt"};

#define TRUE 1
#define FALSE 0
#define MAX_CLIENTS 20
#define CLADDR_LEN 100

#define bufSize 4096


int d_flag[5];

/*------------------ STRUCTURES DEFINATIONS ----------------------------*/

typedef struct station_list {
    uint8_t station_number;
    uint8_t station_name_size;
    char station_name[100];
    uint32_t multicast_address;
    uint16_t data_port;
    uint16_t info_port;
    uint32_t bit_rate;
} station_info;

typedef struct plateform_info {
    uint8_t streaming_name_size;
    char streaming_name[100];
    uint8_t total_station;
    station_info station_list[5];

} Plateform_Info;

typedef struct song_info {

    uint8_t type;
    uint8_t song_name_size;
    char song_name[100];
    uint16_t remaining_time_in_sec;
    uint8_t next_song_name_size;
    char next_song_name[100];
} Song_info;

/*--------------------------------------------------------------------*/

// function to clear buffer 
void clearBuf(char* b) 
{ 
	int i; 
	for (i = 0; i < bufSize; i++) 
		b[i] = '\0'; 
} 

void delay(int milliseconds) {
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause) {
        now = clock();
    }
}

struct args {
    char RADIO[17];
    int MULTI_PORT;
    int INFO_PORT;
    int BUF_SIZE;
    char video_name[200];
    char duration_file_name[100];
};


void *threadfunctionInfo(void *input) {
    Song_info songs_playlist;

    char radio[17], video_name[200], duration_file_name[100];
    int info_port, buf_SIZE;

    info_port = ((struct args *)input)->INFO_PORT;
    strcpy(radio, ((struct args *)input)->RADIO);
    buf_SIZE = ((struct args *)input)->BUF_SIZE;
    strcpy(video_name, ((struct args *)input)->video_name);
    strcpy(duration_file_name, ((struct args *)input)->duration_file_name);

    printf("%s\t%d\t%d\t%s\n", radio, info_port, buf_SIZE, duration_file_name);

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;

    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server : socket");
        exit(TRUE);
    }

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(radio);
    servaddr.sin_port = htons(info_port);
    /*------------------------------------------------------------------*/
    /*----------------------    BUFFER DECLARATIONS	  ------------------*/
    int send_status;
    /*------------------------------------------------------------------*/
    int totTime = 60;

    songs_playlist.type = 12;
    songs_playlist.song_name_size = 13;
    strcpy(songs_playlist.song_name, "Let her go");
    songs_playlist.remaining_time_in_sec = totTime;

    songs_playlist.next_song_name_size = 13;
    strcpy(songs_playlist.next_song_name, "Avengers_clip");

    while (songs_playlist.remaining_time_in_sec != 0) {
        printf("Here Error.....");
        if ((send_status = sendto(
                 multi_sockfd, &songs_playlist, sizeof(songs_playlist), 0,
                 (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
            printf("printing\n");
            perror("sender: sendto");
            exit(1);
        }
        printf(
            "Remaining time:--------------------------------------------- %d\n",
            songs_playlist.remaining_time_in_sec);

        //	sleep(1);
        songs_playlist.remaining_time_in_sec--;
    }

    return NULL;
}

void *threadfunctionTCP(void *input) {
    int tcp_server_fd, sock, valread;
    struct sockaddr_in con_addr;
    int opt = 1;
    //socket variable
    int addrlen=sizeof(con_addr);
    con_addr.sin_family = AF_INET;
    con_addr.sin_addr.s_addr = INADDR_ANY;
    con_addr.sin_port = htons(8800);
    
    if ((tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        printf("file discriptor not recieved");
        exit(0);
    }
    
    if (setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        printf("setsockopt:\n");      
        exit(0);
    }
   
    
    char name[100];
    strcpy(name,"Movie Streaming");
    Plateform_Info movie_list;
    movie_list.streaming_name_size = sizeof(name);
    strcpy(movie_list.streaming_name, name);
    movie_list.total_station = current_stations;
    int i;
    
    //loop for available movie stations
    for(i=0; i<current_stations;i++){
            int buf[256];
            snprintf(buf, sizeof(buf),  "Movie %d", i+1);
	    movie_list.station_list[i].station_number = i+1;
	    movie_list.station_list[i].station_name_size = 9;
	    strcpy(movie_list.station_list[i].station_name, buf);
	    movie_list.station_list[i].multicast_address = inet_addr(Radio[i]);
	    movie_list.station_list[i].data_port = Multi_Port[i];
	    movie_list.station_list[i].info_port = Info_Port[i];
	    movie_list.station_list[i].bit_rate = bufSize;
    } 
 
    if (bind(tcp_server_fd, (struct sockaddr *)&con_addr, addrlen) < 0) {
        printf("fail binding\n");
        exit(0);
    }
    if (listen(tcp_server_fd, 3) < 0) {
        printf("\nlistening");

        exit(0);
    }                                      
    while (1) {
        if ((sock = accept(tcp_server_fd, (struct sockaddr *)&con_addr,(socklen_t *)&addrlen)) < 0){

            printf("accept");
            exit(0);
        }
        printf("Site information sending to new connection\n");
        send(sock, &movie_list, sizeof(Plateform_Info), 0);
        
    }
}

// media file sending
void *threadfunctionUploading(void *input){

    char video_name[200], duration_file_name[100];
    int multiport, buf_SIZE=bufSize;
    uint32_t multicast_address = inet_addr("230.192.1.10");
    char buffer[buf_SIZE];
    FILE *mediaFile;
    int i=0;
    char  verify = '1';
    char filename[256];
    int recieve_size;
    multiport = 6010;
    
    struct ifreq ifr;
   
    struct ip_mreq mcastjoin_req;      /* multicast join struct */
    struct sockaddr_in mcast_servaddr; /* multicast sender*/
    socklen_t mcast_servaddr_len;
    

    int sockfd;
    struct sockaddr_in servaddr;
    int addrlen = sizeof(servaddr);

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(multiport);
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "wlo1", sizeof("wlo1") - 1);
    int opt=1;
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server : socket");
        exit(TRUE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
                    sizeof(ifr))) < 0) {
        perror("receiver: setsockopt() error");
        close(sockfd);
        exit(1);
    }

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) <
        0) {
        perror("receiver: bind()");
        exit(1);
    }

    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if ((setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0) {
        perror("mcast join receive: setsockopt()");
        exit(1);
    }	

  while(1){
            clearBuf(buffer);
            snprintf(filename, sizeof(filename),  "clientUpload%d.mp4", i+1);
	    mediaFile = fopen(filename, "w");
	    if (mediaFile == NULL) {
		printf("Error has occurred. Image file could not be opened\n");
		exit(1);
	    }
	    printf("\nReady to listen from the client!\n\n");
	  
	    int flag = 1;
	    while (flag) {  
		    clearBuf(buffer);
		    memset(&mcast_servaddr, 0, sizeof(mcast_servaddr));
                   mcast_servaddr_len = sizeof(mcast_servaddr);
		    if ((recieve_size = recvfrom(sockfd, buffer,buf_SIZE, 0,
                                         (struct sockaddr *)&mcast_servaddr,
                                         &mcast_servaddr_len)) < 0) {
		        perror("receiver: recvfrom()");
		        exit(TRUE);
		    }
		    
		    fwrite(buffer, 1, recieve_size, mediaFile);

		    if (recieve_size < buf_SIZE) {
		        flag = 0;
		    }
	    }
	    
	    fclose(mediaFile);
	    printf("finishhhh--------\n\n\n\n\n------------");
    	    i++;
    }
    close(sockfd);
}

void *threadfunctionUDP(void *input) {

    // usleep(1000);
    char radio[17], video_name[200], duration_file_name[100];
    int multiport, buf_SIZE;

    multiport = ((struct args *)input)->MULTI_PORT;
    strcpy(radio, ((struct args *)input)->RADIO);
    buf_SIZE = ((struct args *)input)->BUF_SIZE;
    strcpy(video_name, ((struct args *)input)->video_name);
    strcpy(duration_file_name, ((struct args *)input)->duration_file_name);

    printf("%s\t%d\t%d\t%s\n", radio, multiport, buf_SIZE, duration_file_name);

    clock_t start, mid, end;
    double execTime;

    FILE *duration_file;
    duration_file = fopen(duration_file_name, "re");

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;

    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server : socket");
        exit(TRUE);
    }

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(radio);
    servaddr.sin_port = htons(multiport);
   
    char buffer[buf_SIZE];
    int send_status;
    
    FILE *mediaFile;
    int filesize, packet_index, read_size, total_sent;
    packet_index = 1;
    total_sent = 0;
   // printf("VIDEO NAME: %s\n\n", video_name);
    mediaFile = fopen(video_name, "re");
    printf("Getting Picture Size\n");

    if (mediaFile == NULL) {
        printf("Error Opening Image File");
    }

    fseek(mediaFile, 0, SEEK_END);
    filesize = ftell(mediaFile);
    fseek(mediaFile, 0, SEEK_SET);
    printf("Total Picture size: %i\n", filesize);

    char string[100] = {0};
    int ret = 0, hour = 0, min = 0, sec = 0;
    fread(string, 1, 100, duration_file);
    sscanf(string, "%d:%d:%d", &hour, &min, &sec);
    int total_time = 3600 * hour + 60 * min + sec;
    int num_pac_sec = (filesize / total_time) / buf_SIZE;

   // printf("---------- %d , %s\n", num_pac_sec, duration_file_name);

    /*-------------------------------------------------------------------*/
    execTime = 0;
    start = clock();
    while (!feof(mediaFile)) {

        read_size = fread(buffer, 1, buf_SIZE, mediaFile);
        total_sent += read_size;
       // printf("Packet Size: = %d\n", read_size);

        if ((send_status = sendto(multi_sockfd, buffer, sizeof(buffer), 0,
                                  (struct sockaddr *)&servaddr,
                                  sizeof(servaddr))) == -1) {

            perror("sender: sendto");
            exit(1);
        }
       // printf("%d : Packet Number: %i\n", multiport, packet_index);
        packet_index++;
        if (packet_index % num_pac_sec == 0) {
            mid = clock();
            execTime = ((double)(mid - start)) / CLOCKS_PER_SEC;
            execTime = (0.9 - execTime);
            usleep((int)(execTime * 1000000));
            // sleep(1);
           // printf("CURRENT SONG NAME : AVENGERS CLIP\n");
           // printf("CURRENT TIME : 00: 02 sec\n");
            //printf("NEXT SONG NAME : NATURALS \n");
            delay(400);
            start = clock();
        }
        delay(5);
        // usleep(13000);
    }
    memset(buffer, 0, sizeof(buffer));
    close(multi_sockfd);
    return NULL;
}

int main() {
     pthread_t movie_id[current_stations];
    pthread_t threadTCP;
    pthread_t upload;
    pthread_create(&threadTCP, NULL, threadfunctionTCP, NULL);
    pthread_create(&upload, NULL, threadfunctionUploading, NULL);
    int i;
    struct args *stations[current_stations];
    for(i=0; i<current_stations;i++){
    	stations[i] = (struct args *)malloc(sizeof(struct args));
    }
    char sys_call[256] ;
    for(i=0; i<current_stations;i++){
    	snprintf(sys_call, sizeof(sys_call),  "ffmpeg -i %s 2>&1 | grep Duration | cut -d ' "
                     "' -f 4 | sed s/,// > %s", video_name[i], duration[i]);
       system(sys_call);
    }
    for(i=0; i<current_stations;i++){
    	strcpy(stations[i]->RADIO, Radio[i]);
        stations[i]->MULTI_PORT = Multi_Port[i];
        stations[i]->INFO_PORT = Info_Port[i];
        stations[i]->BUF_SIZE = bufSize;
        strcpy(stations[i]->video_name, video_name[i]);
        strcpy(stations[i]->duration_file_name, duration[i]);
        pthread_create(&movie_id[i], NULL, threadfunctionUDP, (void *)stations[i]);
    }
    for(i=0; i<current_stations;i++){
       pthread_join(movie_id[i], NULL);
    }
    pthread_join(threadTCP, NULL);
    pthread_join(upload, NULL);

    return 0;
}
