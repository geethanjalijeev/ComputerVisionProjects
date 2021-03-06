//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/background_segm.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>
#include <math.h>
using namespace cv;
using namespace std;
// Global variables
Mat frame; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Mat image;
Mat erode_dest(fgMaskMOG2.size(),fgMaskMOG2.type());
Mat dilate_dest(fgMaskMOG2.size(),fgMaskMOG2.type());
Mat element = getStructuringElement(MORPH_RECT, Size(3,3));

int checkPedestrian = 0, checkCar = 0, checkBus = 0;
int Pedestrian = 0, Car = 0, Bus = 0;
int setFlag = 0, flag = 0, set = 0, z=0;
Point2f centerPos;
Point px;
float diff;
Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

Mat canny_output;
vector<vector<Point> > contours;
vector<Rect> saved(5);
vector<Rect> BoundingBox;
vector<Rect>::iterator it;
vector<Vec4i> hierarchy;
int maxArea = 0;
int checkingD = 0;
void thresh_callback(int,void*);


Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
Ptr<BackgroundSubtractor> pMOG;
int keyboard; //input from keyboard

void processVideo(char* videoFilename);

int main(int argc, char* argv[])
{
    
    //check for the input parameter correctness
    if(argc != 3) {
        cerr <<"Incorret input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    //create GUI windows
    namedWindow("Frame",CV_WINDOW_AUTOSIZE);
    namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
    
    //create Background Subtractor objects
    pMOG2 = createBackgroundSubtractorKNN(); //MOG2 approach
    
    if(strcmp(argv[1], "-vid") == 0) {
        //input data coming from a video
        processVideo(argv[2]);
    }
    else {
        //error in reading input parameters
        cerr <<"Please, check the input parameters." << endl;
        cerr <<"Exiting..." << endl;
        return EXIT_FAILURE;
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}
void processVideo(char* videoFilename) {
    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }
    
    VideoWriter video("out.mp4",CV_FOURCC('H','2','6','4'),10, Size(350,320),true);
    
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
       
        if(!capture.read(src)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        
        if(set==3 && (z!=36)){
            imshow("Contours",src);
            //video.write(src);
            z++;
            if(!capture.read(src)) {
                cerr << "Unable to read next frame." << endl;
                cerr << "Exiting..." << endl;
                exit(EXIT_FAILURE);
            }
            if(z==35){
                set=0;
                flag = 0;
            }
        }
        else{

            cvtColor(src, src_gray, CV_RGB2GRAY);
            imshow("Frame",src);
            
            pMOG2->apply(src_gray, fgMaskMOG2);
            dilate(fgMaskMOG2,src_gray,element);
            erode(src_gray,src_gray,element);
            GaussianBlur(src_gray, src_gray, Size(3,3), 0,0);
            dilate(src_gray,src_gray,element);
            dilate(src_gray,src_gray,element);
            dilate(src_gray,src_gray,element);
            
            thresh_callback(0, 0);
        }

        keyboard = waitKey( 30 );
    }
    //delete capture object
    capture.release();
}

void thresh_callback(int, void* )
{
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    int check1 = 0;
    
    /// Detect edges using Threshold
    threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
    /// Find contours
    findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Approximate contours to polygons + get bounding rects and circles
    vector<vector<Point> > contours_poly( contours.size() );
    vector<Rect> boundRect( contours.size() );
    vector<Point2f>center( contours.size() );
    vector<float>radius( contours.size() );
    
    for( int i = 0; i < contours.size(); i++ )
    { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
        minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
    }
    
    
    /// Draw polygonal contour + bonding rects + circles
    Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
    Point pt1,pt2,pt;
    Point ptx1,ptx2;
    int area;
    int count = 0,countx = 0;
    
    
    
    for( int i = 0; i< contours.size(); i++ )
    {
        countx++;
        int check = 0;
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        
        pt1 = boundRect[i].tl();
        pt2 = boundRect[i].br();
        pt = pt2 - pt1;
        area = pt.x * pt.y;
        
        if(area > 1000 && pt.y >=30){
            
          
           if(area > maxArea){
                maxArea = area;
            }
            else if(area < 510){
                maxArea = 0;
            }
            
            
            for(int j=0; j<saved.size();j++){
                ptx1 = saved[j].tl();
                ptx2 = saved[j].br();
                
                if(pt1.x > ptx1.x && pt1.y > ptx1.y && pt2.x < ptx2.x && pt2.y < ptx2.y){
                    check++;
                    break;
                }
                
            }
            
            if(check == 0){
            count++;
             saved[count] = boundRect[i];
            
                if(count==1 && pt.y >= 30 && pt.y <=65 && pt.x >= 100 && pt.x <= 140 && pt1.y >=95 && area >= 3000 && area <= 8400 && (set==0 || set==1 || set==2) ){
                    
                    if(flag==0 && boundRect[i].br().x >= 1 && boundRect[i].br().x <= 280){
                        centerPos = center[i];
                        px = boundRect[i].br();
                        flag = 1;
                    }
                    else if(flag==1){
                        diff = center[i].x - centerPos.x;
                        
                        if(diff > 0.0){
                            set++;
                            if(set==3){
                                z = 0;
                            }
                        }
                        else{
                            set = 0;
                            flag = 0;
                        }
                     }
                }
            }
        }
    }
}

