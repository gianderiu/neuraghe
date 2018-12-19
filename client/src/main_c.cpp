#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <errno.h>

#include <pthread.h> 
#include <semaphore.h>

#include <argp.h>
#include <dirent.h>

#include <iostream>

#include "types.h"
#include "imagenet.h"

using namespace cv;
using namespace std;

extern NAME imagenet[];

static int linkfd;

//static sem_t free_space;
//static sem_t images_in_buffer;

static int width;
static int height;
static int planes;
static uint32_t* top_res;

static int out_classes;

static char address[50];
static int port;

static VARNAME image_path = { 0 };
static VARNAME cnn_imgs = { 0 };

struct data_res
{
    char  image_name[100];
    double cnn_time;
    unsigned int top_res[5];
};

static struct data_res net_data, gt_data;

static struct argp_option options[] =
{
    { "address",   'a',  "IP address ",    0, "IP address"},
    { "port",      'P',  "IP port ",       0, "IP port"},    
    { "images",    'i',  "images folder ", 0, "Path to images folder"},
    { "planes",    'p',  "planes number ", 0, "Input image planes"},
    { "height",    'h',  "image height  ", 0, "Input image height"},
    { "width",     'w',  "image width   ", 0, "Input image width"},
    { "classes",   'o',  "# of classes  ", 0, "Number of output classes"},
    { 0 }
};

static int parse_opt(int key, char *arg, struct argp_state *state)
{

    switch (key)
    {
        case 'a':
            sprintf(address, "%s", arg);
            break;
        case 'P':
            port = atoi(arg);
            break;
        case 'i':
            sprintf(image_path, "%s", arg);
            break;
        case 'p':
            planes = atoi(arg);
            break;
        case 'h':
            height = atoi(arg);
            break;
        case 'w':
            width = atoi(arg);
            break;
        case 'o':
            out_classes = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            argp_error(state, "Too many arguments");
        case ARGP_KEY_END:
            if (state->argc < 2)/* Not enough arguments. */
                argp_error(state, "Not enough arguments");
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
} 

static int init_link(int PORT, char* address)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr; 

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 


    if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    return sockfd;
}

static void init()
{
//    sem_init(&free_space, 0, MAX_IMAGES);
//    sem_init(&images_in_buffer, 0, 0);
}

static void free()
{
//    sem_destroy(&free_space);
//    sem_destroy(&images_in_buffer);

    close(linkfd);
}

static void check_res(unsigned int* bgr, unsigned int* hw_res, unsigned int* sw_res, int num_top)
{
    bgr[0] = 0;
    bgr[1] = 0;
    bgr[2] = 255;

    if(hw_res[0] == sw_res[0]){
	   bgr[2] = 0;
       bgr[1] = 255;
    }
    else{
        for(int i=0; i<num_top; i++)
        {
            if(hw_res[i] == sw_res[0])
                bgr[1] = 255;
        }
    }

    return;
}

int main( int argc, char** argv ) {

    struct dirent** file_list;
    int images_in_folder;
    int num_top;
    VARNAME top_res_text = { 0 };
    VARNAME gt_path = { 0 };
    FILE* gt;
    float value;
    bool quit = false; 
    unsigned int bgr[3];
    int tp = 20;
    int red = 0;
    int green = 0;
    int yellow = 0;

    //Parse input parameters
    struct argp argp = {options, parse_opt, 0, 0};
    if(argp_parse(&argp, argc, argv, 0, 0, 0))
    {
        fprintf(stderr, "Error in parsing input parameters");
        return 1;
    }                              

    linkfd = init_link(port, address);
    
    init();

    if(out_classes == 1000){
        sprintf(cnn_imgs, "%s", image_path);
        num_top = 5;
    }
    else {
        if(out_classes == 10){
            sprintf(cnn_imgs, "%s", image_path);
            num_top = 2;
        }
        else{
            printf("CNN not supported\n");
            return 1;
        }
    }

    /* Scanning the Imgs directory */    
    images_in_folder = scandir(cnn_imgs, &file_list, NULL, alphasort);
    if(images_in_folder==-1)
    {
        fprintf(stderr, "Error : Failed to open image directory\n");
        exit(1);
    }

    uint32_t check = 10;

    Mat image = Mat::zeros(512, 512, CV_8UC3);
    Mat subImg = Mat::zeros(350, 600, CV_8UC3);

    int n = 3;      //skip current and parent directories
  
    namedWindow("Display window", WINDOW_NORMAL );// Create a window for display.
    moveWindow("Display window", 128, 64);
    namedWindow("Results", WINDOW_AUTOSIZE );
    moveWindow("Results", 32, 512);

    int baseline=0;
    Size text_size = getTextSize("TEST",FONT_HERSHEY_PLAIN,1.2,1.6,&baseline);
    int th = text_size.height + 8; 

    imshow("Display window", image);
    imshow("Results", subImg);                   // Show our image inside it. 

    uint32_t nb_recv = 0;
    uint32_t nb_send = 0;


    FILE *red_file;
    FILE *yellow_file;
    FILE *green_file;

    red_file = fopen("red_imgs", "w");
    yellow_file = fopen("yellow_imgs", "w");
    green_file = fopen("green_imgs", "w");
    int countimg=0;
    while(!quit)
    {   

        nb_recv = recv(linkfd, &net_data, sizeof(net_data), 0);

        if(nb_recv == 0) {
            printf("error.\n");
        } else if (nb_recv == sizeof(uint) && *((uint *) &net_data) == 0xdeadbeef) { 
            quit = true;
            //Free space
            free();
            continue;
        }

        if(nb_recv != sizeof(net_data)) 
            printf("Error sending packet!\n");
                
        sprintf(image_path, "%s/%s", cnn_imgs, net_data.image_name);
        sprintf(gt_path, "%s/GT/%s.txt", cnn_imgs, net_data.image_name);

        //printf("%s\n", image_path);
        //printf("GT: %s\n", gt_path);
        countimg++;
        if (countimg>950)
          exit(0);
        image = imread(image_path, CV_LOAD_IMAGE_COLOR);   // Read the file
        gt = fopen(gt_path, "r");

        if (gt == NULL)
        {
            fprintf(stderr, "Error : Failed to open ground truth file\n");
            exit(1);
        }

        if(! image.data )                              // Check for invalid input
        {
            cout <<  "Could not open or find the image" << std::endl ;
            exit(1);
        }
        
        subImg.setTo(0);
        fscanf(gt, "%lf\n\n", &gt_data.cnn_time);
        for (int i = 0; i < num_top; i++) {
            fscanf(gt, "%f %d\n", &value, &gt_data.top_res[i]);
            if(out_classes == 1000) 
                gt_data.top_res[i] = gt_data.top_res[i] - 1;
        }
	
        check_res(bgr, net_data.top_res, gt_data.top_res, num_top);

        
        if(memcmp(bgr, (const unsigned int[]){0,0,255}, sizeof(bgr)) == 0){
            fprintf(red_file, "%s\n", net_data.image_name);
            printf(RED "%s\n" NC, net_data.image_name);
            red++;
        }
        if(memcmp(bgr, (const unsigned int[]){0,255,0}, sizeof(bgr)) == 0){
            fprintf(green_file, "%s\n", net_data.image_name);
            printf(GREEN "%s\n" NC, net_data.image_name);
            green++;
        }
        if(memcmp(bgr, (const unsigned int[]){0,255,255}, sizeof(bgr)) == 0){
            fprintf(yellow_file, "%s\n", net_data.image_name);
            printf(YELLOW "%s\n" NC, net_data.image_name);
            yellow++;
        }

        //for(int i = 0; i < num_top; i++)
        //    printf("\t%4d == %4d\n", net_data.top_res[i], gt_data.top_res[i]);
        

        putText(subImg, "NEURAGHE output", Point2f(0, tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,0,255), 1.6);

        if(out_classes == 1000)        
            sprintf(top_res_text, "Inference Time: %5.3f ms   FPS: %3.2f", net_data.cnn_time/1000, 1.0/((net_data.cnn_time/1000)/1000));
        else
            sprintf(top_res_text, "Inference Time: %5.3f ms   FPS: %3.2f", net_data.cnn_time/1000, 1.0/(net_data.cnn_time/1000000));

        putText(subImg, top_res_text, Point2f(0, th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,255,255), 1.6);

        putText(subImg, "Ground Truth (Tensorflow, Intel i7-6700 8 x 3.40GHz)", Point2f(0,(num_top+3)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,0,255), 1.6);

        sprintf(top_res_text, "Inference Time: %5.3f ms   FPS: %3.2f", gt_data.cnn_time*1000, 1.0/gt_data.cnn_time);
        putText(subImg, top_res_text, Point2f(0, (num_top+4)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,255,255), 1.6);
        
        for (int i = 0; i < num_top; i++) {
            //printf(LGREEN "our top %u estimate is:\t[%d] -> %s\n" NC, i, net_data.top_res[i], imagenet[net_data.top_res[i]]);
            if(out_classes == 1000){
                sprintf(top_res_text, "Top %d:  %s", i, imagenet[net_data.top_res[i]]);
                putText(subImg, top_res_text, Point2f(0,(i+2)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(bgr[0],bgr[1],bgr[2],255), 1.6);

                sprintf(top_res_text, "Top %d:  %s", i, imagenet[gt_data.top_res[i]]);
                putText(subImg, top_res_text, Point2f(0,(i+num_top+5)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,255,255), 1.6);
            }
            else {
                sprintf(top_res_text, "Top %d:  %d", i, net_data.top_res[i]);
                putText(subImg, top_res_text, Point2f(0,(i+2)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(bgr[0],bgr[1],bgr[2],255), 1.6);

                sprintf(top_res_text, "Top %d:  %d", i, gt_data.top_res[i]);
                putText(subImg, top_res_text, Point2f(0,(i+num_top+5)*th+tp), FONT_HERSHEY_PLAIN, 1.2,  Scalar(255,255,255,255), 1.6);
            }
        }

        imshow("Display window", image);
        imshow("Results", subImg);                   // Show our image inside it. 
        waitKey(1);
        
        fclose(gt);

    }

    
    fclose(red_file);
    fclose(yellow_file);
    fclose(green_file);
    

    printf("green:  %5d\n", green);
    printf("yellow: %5d\n", yellow);
    printf("red:    %5d\n", red);
    printf("total:  %5d\n", green+yellow+red);

    waitKey(0); 

//    pthread_exit(0);
//    //Thread create
//    pthread_t frame;
//    pthread_t cnn;
//    void* frame_capture(void*);
//    void* results_capture(void*);
//
//    if( pthread_create(&frame, NULL, frame_capture, (void*) file_list) != 0) {
//        printf("Error creating frame_capture thread!\n");
//        return 1;       
//    } 
//    if( pthread_create(&cnn, NULL, results_capture, 0) != 0) {
//        printf("Error creating results_capture thread!\n");
//        return 1;       
//    } 
//    
//    pthread_join(frame, NULL);
//    pthread_join(cnn, NULL);



    return 0;
}
