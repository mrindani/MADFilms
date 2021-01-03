/*
client side
*/

//to make available the in_port_t and in_addr_t type and in_addr structure
#include <arpa/inet.h> 
//to display the error message
#include <errno.h> 
//for graphics 
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
//for checking the socket local interfaces (number value and the name of the interface)
#include <net/if.h>
//for defining the in_addr and the sockaddr_in structure for storing internet protocol family
#include <netinet/in.h>
//for calling all the  htread related functions(multithreading)
#include <pthread.h>
//calling the main libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
//ioctl = IO control (data and driver operations)
#include <sys/ioctl.h>
//for defining the sa_family_t structure used in sockaddr_in
#include <sys/socket.h>
//defining the timeval structure
#include <sys/time.h>
//defining the file block counts and size types
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
//for vlc player features
#include <vlc/vlc.h>

//for defining the port to be used 
#define TCP_PORT 8080
//for defining the protocol value
#define TCP_IP "0.0.0.0"

//the maximum clients that can use the system 
#define MAX_CLIENTS 20
//the client address length
#define CLADDR_LEN 100

//defining the buffer size for data transfer
/*#define BUF_SIZE1 256
#define BUF_SIZE2 512
#define BUF_SIZE3 1024
#define BUF_SIZE4 2048
#define BUF_SIZE6 10240*/
#define BUF_SIZE5 4096

//defining the interfaces that provide multicasting facility
#define IF_NAME1 "wlo1"
#define IF_NAME2 "eno1"

//variable for GUI of the window
#define BORDER_WIDTH 6

/*IMPORTANT DO NOT REMOVE file:// in the path, it needs to be a file url*/
#define input_video "file:///home/miracle/CN_Lab/Project/MADFilms/Client/output.mp4"

/////////////////////defining the structures//////////////////////////////

//strucure for fetching the station details from the server
typedef struct station_list {

    uint8_t station_number;
    uint8_t station_name_size;
    char station_name[100];
    uint32_t multicast_address;
    uint16_t data_port;
    uint16_t info_port;
    uint32_t bit_rate;

} station_info;

//strucure for fetching the site details from the server
typedef struct plateform_info {
    uint8_t streaming_name_size;
    char streaming_name[100];
    uint8_t total_station;
    station_info station_list[5];

} Plateform_Info;

//structure for sending the address ,port and buffer size details together
struct args {

    uint32_t multicast_address;
    uint16_t data_port;
    uint32_t info_port;
    uint32_t BUF_SIZE;
};

//struct vlc {};

///////////declaring the functions for gtk and video streaming////////////
void destroy(GtkWidget *widget, gpointer data);
void player_widget_on_realize(GtkWidget *widget, gpointer data);
void open_media(const char *uri);
void play(void);
void pause_player(void);
void on_playpause(GtkWidget *widget, gpointer data);
void on_stop(GtkWidget *widget, gpointer data);
static void crete_new_wind (GtkWidget *widget, gpointer window);
static void increase_speed();
static void decrease_speed();
void download_video(GtkWidget* widget, gpointer data);

///////////////////declaring all the global variables/////////////////////
//for maintaining the status of movie
int flag;           //display the movie
int pause_flag;     //pause or play the movie
int curr_radio_channel; //for maintaining the movie station number
int uploadFlag=0;

//for vlc display
libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;

//for movie streaming button display
GtkWidget *playpause_button;
GtkWidget *button,*inc_button,*dec_button;
GtkWidget *download_button;

//for fetching the movie station details
Plateform_Info movie_list;

////////////////////defining the functions/////////////////////////////
void clearBuf(char* b) 
{ 
	int i; 
	for (i = 0; i < BUF_SIZE5; i++) 
		b[i] = '\0'; 
} 

void delay(int milliseconds) {
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

void destroy(GtkWidget *widget, gpointer data) { gtk_main_quit(); }

void player_widget_on_realize(GtkWidget *widget, gpointer data) {
    libvlc_media_player_set_xwindow(
        media_player, GDK_WINDOW_XID(gtk_widget_get_window(widget)));
}

void *on_open() {
    usleep(500000);
    open_media(input_video);
}

void open_media(const char *uri) {

    libvlc_media_t *media;
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);
    play();
    libvlc_media_release(media);
}

void on_stop(GtkWidget *widget, gpointer data) {
    pause_player();
    libvlc_media_player_stop(media_player);
}

void play(void) {
     if(pause_flag==0)
    	libvlc_media_player_play(media_player);
    else{
    	libvlc_media_player_set_pause(media_player,0);
    }
    pause_flag=0;
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
}

void pause_player(void) {
    libvlc_media_player_set_pause(media_player,1);
    pause_flag=1;
    
   
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-play");
}

void crete_new_wind (GtkWidget *widget,gpointer window){
    
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Choose a file", GTK_WINDOW(window),      GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_widget_show_all(dialog);
//  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),"/");
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    
    printf("helloo\n-------");
    int sockfd, PORT_NO; 
    FILE *mediaFile;
    int filesize, packet_index, read_size, total_sent;   
    int send_status;
    char video_name[]="v1.mp4";
    PORT_NO = 6010;
    uint32_t BUF_SIZE = BUF_SIZE5;
    
    struct sockaddr_in addr_con; 
    struct hostent *server;
    int addrlen = sizeof(addr_con); 
    addr_con.sin_family = AF_INET; 
    addr_con.sin_port = htons(PORT_NO); 

    addr_con.sin_addr.s_addr = inet_addr("230.192.1.10");
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
					
    if (sockfd < 0) {
		printf("\nfile descriptor not received!!\n"); 
		return;
   }else
		printf("\nfile descriptor %d received\n", sockfd); 

    char buffer[BUF_SIZE];
   
  
    packet_index = 1;
    total_sent = 0;
  
    printf("VIDEO NAME: %s\n\n", video_name);
    mediaFile = fopen(video_name, "re");
    printf("Getting Picture Size\n");

    if (mediaFile == NULL) {
        printf("Error Opening Image File");
    }

    fseek(mediaFile, 0, SEEK_END);
    filesize = ftell(mediaFile);
    fseek(mediaFile, 0, SEEK_SET);
    
    printf("Total Picture size: %i\n", filesize);
    clearBuf(buffer);
 
    while (!feof(mediaFile)) {
        clearBuf(buffer);
        read_size = fread(buffer, 1, BUF_SIZE, mediaFile);
        total_sent += read_size;
        printf("Packet Size: = %d\n", read_size);

        if ((send_status = sendto(sockfd, buffer,BUF_SIZE, 0,
                                  (struct sockaddr *)&addr_con,
                                  addrlen)) == -1) {

            perror("sender: sendto");
            exit(1);
        }
        printf("%d : Packet Number: %i\n", PORT_NO, packet_index);
        packet_index++;

        delay(5);
    }

   // close(sockfd);
    fclose(mediaFile);
    
    if(resp == GTK_RESPONSE_OK)
        g_print("%s\n", gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    else
        g_print("You pressed Cancel\n");
    gtk_widget_destroy(dialog);
}

void download_video(GtkWidget* widget, gpointer data) {
	FILE *output;
	
	output = fopen("output.mp4", "r");
	
	if(output) {
		printf("File exists...\n");
		system("cp output.mp4 /home/miracle/CN_Lab/Project/MADFilms/Client/download.mp4");
		printf("Your video is downloaded successfully...");	
	}
	else
		printf("video file you want to download does not exist...");
		
	fclose(output);
}

void increase_speed(){
  libvlc_media_player_set_rate(media_player,libvlc_media_player_get_rate(media_player)+ 0.5);
}

void decrease_speed(){
   if((libvlc_media_player_get_rate(media_player)- 0.5)>0){
   libvlc_media_player_set_rate(media_player,libvlc_media_player_get_rate(media_player)- 0.5);
   }
}

void *radio_channel(void *input) {
    flag = 0;
    usleep(500000);
    // sleep(1);                                  /* DELAY TESTED : OK*/
    int opt = 1;
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
    uint32_t multicast_address = ((struct args *)input)->multicast_address;
    uint16_t data_port = ((struct args *)input)->data_port;
    uint32_t info_port = ((struct args *)input)->info_port;
    uint32_t BUF_SIZE = ((struct args *)input)->BUF_SIZE;

    printf("multi-cast adrress : %d\n", multicast_address);
    printf("PORT : %d\n", data_port);
    printf("INFO PORT : %d\n", info_port);
    printf("BUFFER SIZE : %d\n", BUF_SIZE);

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;
    char interface_name[100];
    struct ifreq ifr;
    char *mcast_addr;
    struct ip_mreq mcastjoin_req;      /* multicast join struct */
    struct sockaddr_in mcast_servaddr; /* multicast sender*/
    socklen_t mcast_servaddr_len;

    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("receiver: socket");
        exit(1);
    }
    printf("HERE\n");

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(data_port);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, IF_NAME1, sizeof(IF_NAME1) - 1);

    if (setsockopt(multi_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if ((setsockopt(multi_sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
                    sizeof(ifr))) < 0) {
        perror("receiver: setsockopt() error");
        close(multi_sockfd);
        exit(1);
    }

    if ((bind(multi_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) <
        0) {
        perror("receiver: bind()");
        exit(1);
    }

    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);

    if ((setsockopt(multi_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0) {
        perror("mcast join receive: setsockopt()");
        exit(1);
    }
    
    char buffer[BUF_SIZE];
    int recieve_size;
    
    FILE *mediaFile;
    char outputarray[BUF_SIZE5], verify = '1';
    mediaFile = fopen("output.mp4", "w+");

    if (mediaFile == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
        exit(1);
    }

    //  on_open();

    pthread_t vlc;
    pthread_create(&vlc, NULL, on_open, NULL);

    printf("\nReady to listen!\n\n");
    flag = 1;
    pause_flag = 0;

    while (flag) {
        if (pause_flag == 0) {
            memset(&mcast_servaddr, 0, sizeof(mcast_servaddr));
            mcast_servaddr_len = sizeof(mcast_servaddr);
            memset(buffer, '\0', BUF_SIZE);
            delay(1);
            memset(buffer, 0, sizeof(buffer));
            if ((recieve_size = recvfrom(multi_sockfd, buffer, BUF_SIZE, 0,
                                         (struct sockaddr *)&mcast_servaddr,
                                         &mcast_servaddr_len)) < 0) {
                perror("receiver: recvfrom()");
                exit(TRUE);
            }
            fwrite(buffer, 1, recieve_size, mediaFile);
            // fputs(buffer,stdout);

            if (recieve_size < BUF_SIZE) {
                flag = 0;
            }
        }
    }
    fclose(mediaFile);
    close(multi_sockfd);
    //system("rm output.mp4");
    printf("Successfully reieved!!");
}

void on_playpause(GtkWidget *widget, gpointer data) {

    if (libvlc_media_player_is_playing(media_player) == 1) {
        pause_flag = 1;

        pause_player();
    } else {
        flag = 0;
        //alreday running and user stopped it in somewhere
        if(pause_flag==1)
        	play();
        else{
               // first time running
		pthread_t my_radio_channel;
		struct args *station = (struct args *)malloc(sizeof(struct args));

		station->multicast_address =
		    movie_list.station_list[curr_radio_channel].multicast_address;
		station->data_port = movie_list.station_list[curr_radio_channel].data_port;
		station->info_port = movie_list.station_list[curr_radio_channel].info_port;
		station->BUF_SIZE = movie_list.station_list[curr_radio_channel].bit_rate;

		pthread_create(&my_radio_channel, NULL, radio_channel, (void *)station);
        }
    }
}

//when the client request to server to get the details of the available movie station using TCP connection
//TCP used because we want to send the whole request of the station to the server without any data loss
void button_fetch_clicked(gpointer data) {

    /*----------------------- TCP SOCKET------------------------------*/
     int tcp_client_fd;
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(8800);
    
    //socket creation
    if ((tcp_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }

    if (inet_pton(AF_INET, TCP_IP, &client_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }
    
    //socket connection
    if (connect(tcp_client_fd, (struct sockaddr *)&client_addr,
                sizeof(client_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    printf("Plateform's movies are fetching....\n");
    
    //read data
    read(tcp_client_fd, &movie_list, sizeof(Plateform_Info));

    char *movieList[movie_list.total_station][3];
    char var1[33], var2[33];

    
    gtk_clist_clear((GtkCList *)data);
    
    //loop over aviable movies
    for (int i = 0; i < movie_list.total_station; i++) {
        snprintf(var1, sizeof(var1), "%d", i+1);
        
        //sr no
        movieList[i][0] = var1;
        
        //movie name
        movieList[i][1] = movie_list.station_list[i].station_name;
        
        //address
        snprintf(var2, sizeof(var2), "%d", movie_list.station_list[i].multicast_address); 
        movieList[i][2] = var2;
        
        gtk_clist_append((GtkCList *)data, movieList[i]);
    }

    close(tcp_client_fd);
    return;
}    

void selection_made(GtkWidget *clist, gint row, gint column,
                    GdkEventButton *event, gpointer data) {
    // gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
    gchar *text;
    gtk_clist_get_text(GTK_CLIST(clist), row, 2, &text);
    g_print("IP address is %d\n\n", row);

    pthread_t my_radio_channel;
    struct args *station = (struct args *)malloc(sizeof(struct args));

    station->multicast_address = movie_list.station_list[row].multicast_address;
    station->data_port = movie_list.station_list[row].data_port;
    station->info_port = movie_list.station_list[row].info_port;
    station->BUF_SIZE = movie_list.station_list[row].bit_rate;

    curr_radio_channel = row;
    pthread_create(&my_radio_channel, NULL, radio_channel, (void *)station);
}
int main(int argc, gchar *argv[]) {

    //defining all the gtk widget needed
    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *scrolled_window, *clist;
    GtkWidget *button_fetch;
    GtkWidget *hbuttonbox;
    GtkWidget *player_widget;
    GtkWidget *stop_button;

    gchar *titles[3] = {"Movie No.", "Movie Name", "Movie Address Value"};

    //calling the gtk main function
    gtk_init(&argc, &argv);
    
   //making the window for the display of 1400X900 size
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(GTK_WIDGET(window), 1400, 900);

   //defining the title of the display and also defining the close function
    gtk_window_set_title(GTK_WINDOW(window), "MAD films- Video Streaming Application");
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
   /* GdkColor color;
    color.red = 0xffff;
    color.green = 0xffff;
    color.blue = 0xffff;                   
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, "800000");*/
    
    GdkColor color;
    gdk_color_parse("#141414", &color);
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

    //defining the list of available channels
    clist = gtk_clist_new_with_titles(3, titles);

    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 460);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 460);
    gtk_clist_set_column_width(GTK_CLIST(clist), 2, 460);
    
     //defining the vertical scroll-bar
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 05);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);
      
    //defining the horizontal scroll-bar 
    hbox = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    //the button for making the connection with the server for fetching the available channels
    button_fetch = gtk_button_new_with_label("Display Movie List");

    gtk_box_pack_start(GTK_BOX(hbox), button_fetch, TRUE, TRUE, 0);

    gtk_signal_connect_object(GTK_OBJECT(button_fetch), "clicked",
                              GTK_SIGNAL_FUNC(button_fetch_clicked),
                              (gpointer)clist);

    gtk_widget_show(button_fetch);
    
    ///////////////////////////////////
    player_widget = gtk_drawing_area_new();
    gtk_drawing_area_size(player_widget,1400,500);
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    //play-pause
    playpause_button = gtk_button_new_with_label("gtk-media-play");
    gtk_button_set_use_stock(GTK_BUTTON(playpause_button), TRUE);
    g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_playpause),
                     NULL);   
    //stop                 
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);             
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), NULL);
    
    //upload video button 
    button = gtk_button_new_with_label ("Upload");
   // g_signal_connect (button, "clicked", G_CALLBACK (destroy),NULL);
    g_signal_connect (button, "clicked", G_CALLBACK (crete_new_wind),window);
    gtk_button_set_use_stock(GTK_BUTTON(button), FALSE);
    
    //increase 
    inc_button = gtk_button_new_with_label ("+0.5x");
    g_signal_connect (inc_button, "clicked", G_CALLBACK (increase_speed),NULL);
    gtk_button_set_use_stock(GTK_BUTTON(inc_button), FALSE);
    
    //decrease
    dec_button = gtk_button_new_with_label ("-0.5x");
    g_signal_connect (dec_button, "clicked", G_CALLBACK (decrease_speed),NULL);
    gtk_button_set_use_stock(GTK_BUTTON(dec_button), FALSE);
    
    //download
    download_button = gtk_button_new_with_label("Download"); 
    g_signal_connect(download_button, "clicked", G_CALLBACK(download_video), NULL);
    gtk_button_set_use_stock(GTK_BUTTON(download_button), FALSE);
    
    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), inc_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), dec_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), download_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), button, FALSE, FALSE, 0);
    //gtk_container_add (GTK_CONTAINER (window), box);
    
    
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    vlc_inst = libvlc_new(0, NULL);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize",
                     G_CALLBACK(player_widget_on_realize), NULL);
    ////////////////////////////////////
  //  gtk_widget_modify_bg(player_widget, GTK_STATE_NORMAL, "B22222");
    GdkPixbuf *image = NULL;
    GdkPixmap *background = NULL;
    GtkStyle *style = NULL;

    image = gdk_pixbuf_new_from_file_at_size("background3.jpg",1400,900, NULL);
    gdk_pixbuf_render_pixmap_and_mask(image, &background, NULL, 0);
    style = gtk_style_new();
    style->bg_pixmap [0] = background;

    gtk_widget_set_style (GTK_WIDGET(player_widget), GTK_STYLE (style));
    
  
    //scroll
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_window);
     
     //the display for the available channels
    gtk_signal_connect(GTK_OBJECT(clist), "select_row",
                       GTK_SIGNAL_FUNC(selection_made), NULL);

   // gtk_widget_modify_bg(scrolled_window, GTK_STATE_NORMAL, "FF0000");
    
    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);

    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
     //gtk_widget_modify_bg(clist, GTK_STATE_NORMAL, "FF0000");
     
    GdkColor color1;
    gdk_color_parse("#000000", &color1);
    gtk_widget_modify_bg(scrolled_window, GTK_STATE_NORMAL, &color1);
    
    gtk_widget_show(clist);
    
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
