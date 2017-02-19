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
Mat destarray;

Mat redcolor;
Mat bluecolor;
Mat greencolor;
Mat orangecolor;


int flag = 0, val = 0;

VideoWriter video;

 Point pos1,pos2;
Point2f centerPos;
Point2f centerTemp;
float radiusPos;

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
void thresh_callbackRed(int,void*);
void thresh_callbackBlue(int,void*);
void thresh_callbackGreen(int,void*);
void thresh_callbackOrange(int,void*);

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
    namedWindow("Frame");
    
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
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(src)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        
        /*    Size S = Size((int)capture.get(CV_CAP_PROP_FRAME_WIDTH),(int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
         video.open("out.avi",CV_FOURCC('X','V','I','D'),10,S,true);
         if(!video.isOpened()){
         cerr << "could not open videowriter" << endl;
         cerr << "Exiting ..." << endl;
         exit(EXIT_FAILURE);
         } */
        
        medianBlur(src, src_gray, 3);
        cvtColor(src_gray, src_gray, CV_BGR2HSV);
        
        inRange(src_gray, cv::Scalar(170, 50, 50), cv::Scalar(180, 255, 255), redcolor);
        
        inRange(src_gray, cv::Scalar(60, 50, 50), cv::Scalar(80, 255, 255), greencolor);
        inRange(src_gray, cv::Scalar(0, 53, 185), cv::Scalar(15, 255, 255), orangecolor);
        
        dilate(orangecolor, orangecolor, element);
        dilate(orangecolor, orangecolor, element);
        imshow("Frame",src);
    
        bitwise_or(redcolor, greencolor, destarray);
        bitwise_or(destarray, orangecolor, destarray);
        erode(destarray, destarray, element);
      
        thresh_callbackOrange(0, 0);

        keyboard = waitKey( 50 );
    }
    //delete capture object
    capture.release();
}

void thresh_callbackRed(int, void* )
{
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Detect edges using Threshold
    threshold( redcolor, threshold_output, thresh, 255, THRESH_BINARY );
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
        
        if(area > 500 && pt.y >=30){
            
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
                
                
                if(count==1){
                    rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color,2 );
                    if(flag==0){
                        if(centerPos.x > pt1.x && centerPos.x < pt2.x && centerPos.y > pt1.y && centerPos.y < pt2.y){
                            flag = 1;
                            val = 3;
                            centerTemp.x = (pt2.x - (pt.y/2)+10);
                            centerTemp.y = pt2.y - 10;
                            circle( src, centerPos, 10, color, 2, 8, 0 );
                        }
                    }
                    else if(flag==1 && val==3){
                        centerTemp.x = (pt2.x - (pt.y/2)+15);
                        centerTemp.y = pt2.y - 15;
                        circle( src, centerTemp, 15, color, 2, 8, 0 );
                    }
                    
                }
            }
            
        }
    }
    
}

void thresh_callbackGreen(int, void* )
{
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Detect edges using Threshold
    threshold( greencolor, threshold_output, thresh, 255, THRESH_BINARY );
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
        
        if(area > 500 && pt.y >=30){
            
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
                
                if(count==1){
                    rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color,2 );
                    if(flag==0){
                        if(centerPos.x > pt1.x && centerPos.x < pt2.x && centerPos.y > pt1.y && centerPos.y < pt2.y){
                            flag = 1;
                            val = 2;
                            centerTemp.x = (pt2.x - (pt.y/2)+10);
                            centerTemp.y = pt2.y - 10;
                            circle( src, centerTemp, (int)10, color, 2, 8, 0 );
                        }
                    }
                    else if(flag==1 && val==2){
                        centerTemp.x = (pt2.x - (pt.y/2)+15);
                        centerTemp.y = pt2.y - 15;
                        circle( src, centerTemp, 15, color, 2, 8, 0 );
                    }
                    
                }
                
            }
            
        }
    }
}

void thresh_callbackOrange(int, void* )
{
    Mat threshold_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    /// Detect edges using Threshold
    threshold( orangecolor, threshold_output, thresh, 255, THRESH_BINARY );
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
        
        if(area > 1000 && pt.y >=30 && pt1.y >=85 && area < 2400){
            
            
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
                
                if(count==1){
                    flag = 0, val = 0;
                    rectangle( src, boundRect[i].tl(), boundRect[i].br(), color,2 );
                    pos1 = boundRect[i].tl();
                    pos2 = boundRect[i].br();
                    centerPos = center[i];
                    radiusPos = (int)radius[i];
                }
                
            }
            
        }
    }
    
    if(count==0){
        
        if(flag==0){
            thresh_callbackGreen(0, 0);
        }
        
        if(flag==0){
            thresh_callbackRed(0, 0);
        }
        
        if(flag==1){
            if(val==2){
                thresh_callbackGreen(0, 0);
            }
            else if(val==3){
                thresh_callbackRed(0, 0);
            }
        }
        
    }
    
    /// Show in a window
    namedWindow( "ContoursOrange", CV_WINDOW_AUTOSIZE );
    imshow( "ContoursOrange", src );
    //  video.write(src_gray);
    waitKey(30);

}