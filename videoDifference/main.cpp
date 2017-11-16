//
//  main.cpp
//  videoDifference
//
//  Created by Rafael M Mudafort on 9/14/15.
//  Copyright (c) 2015 Rafael M Mudafort. All rights reserved.
//

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

//hide the local functions in an anon namespace
namespace {
    
    void help(char** av) {
        cout << "The program captures frames from a video file, image sequence (01.jpg, 02.jpg ... 10.jpg) or camera connected to your computer." << endl
        << "Usage:\n" << av[0] << " <video file, image sequence or device number>" << endl
        << "q,Q,esc -- quit" << endl
        << "space   -- save frame" << endl << endl
        << "\tTo capture from a camera pass the device number. To find the device number, try ls /dev/video*" << endl
        << "\texample: " << av[0] << " 0" << endl
        << "\tYou may also pass a video file instead of a device number" << endl
        << "\texample: " << av[0] << " video.avi" << endl
        << "\tYou can also pass the path to an image sequence and OpenCV will treat the sequence just like a video." << endl
        << "\texample: " << av[0] << " right%%02d.jpg" << endl;
    }
    
    Mat framediff(Mat image0, Mat image1, Mat image2) {
//        return (image0 - 2*image1 + image2)/((image0 - image1) + (image1 - image2));
        return image2 - image0;
    }
    
    Mat laplace(Mat image) {
        Mat kernel = Mat::zeros( 3, 3, CV_8S );
        kernel.at<uchar>(0,1) = 1;
        kernel.at<uchar>(1,0) = 1;
        kernel.at<uchar>(1,1) = -4;
        kernel.at<uchar>(1,2) = 1;
        kernel.at<uchar>(2,1) = 1;
        Mat output;
        filter2D(image, output, -1, kernel);
        return output;
    }
    
    Mat mod_laplace(Mat image) {
        Mat kernel = Mat::ones( 3, 3, CV_8S );
        kernel.at<uchar>(0,1) = 2;
        kernel.at<uchar>(1,0) = 2;
        kernel.at<uchar>(1,1) = -12;
        kernel.at<uchar>(1,2) = 2;
        kernel.at<uchar>(2,1) = 2;
        Mat output;
        filter2D(image, output, -1, kernel);
        return output;
    }

    Mat sharp(Mat image) {
        Mat filter = laplace(image);
        return image - filter;
    }
    
    Mat negative(Mat image) {
        Mat result;
        image.convertTo(result, CV_8S, -1, 0);
        return result;
    }
    
    int process(VideoCapture& capture) {
        int n = 0;
        char filename[200];
        
        string window_name = "this is live! - q or esc to quit - space to save frame";
        namedWindow(window_name, WINDOW_KEEPRATIO);

        int effectFlag = 1; // 1: framediff, 2: modified lapacian

        Mat i0;              // current frame
        Mat im1;             // frame at i-1
        Mat im2;             // frame at i-2
        Mat processed_frame; // resultant frame

        capture >> im2;
        waitKey(1);
        capture >> im1;
        waitKey(1);

        for (;;) {
            
            capture >> i0;
            
            switch (effectFlag) {
                case 1:
                    processed_frame = framediff(i0, im1, im2);
                    im1.copyTo(im2);
                    i0.copyTo(im1);
                    break;
                case 2:
                    processed_frame = mod_laplace(i0);
                    break;
                case 3:
                    processed_frame = sharp(i0);
                    break;
                case 4:
                    processed_frame = mod_laplace(i0);
                    processed_frame = negative(processed_frame);
                    break;
                default:
                    processed_frame = i0;
                    break;
            }
            
            imshow(window_name, processed_frame);
            char key = (char)waitKey(1000/32); //delay for 32 frames per second
            switch (key) {
                case 'q':
                case 'Q':
                case 27: //escape key
                    return 0;
                case ' ': //Save an image
                    sprintf(filename, "processed_frame%.3d.png", n++);
                    imwrite(filename, processed_frame);
                    cout << "Saved " << filename << endl;
                    break;
                case '0':
                    effectFlag = 0;
                    break;
                case '1':
                    effectFlag = 1;
                    break;
                case '2':
                    effectFlag = 2;
                    break;
                case '3':
                    effectFlag = 3;
                    break;
                case '4':
                    effectFlag = 4;
                    break;
                default:
                    break;
            }
        }
        return 0;
    }
}

int main(int ac, char** av) {
    
    if (ac != 2) {
        help(av);
        return 1;
    }
    
    std::string arg = av[1];

    VideoCapture capture(atoi(arg.c_str())); //try to open input argument as int... this will attempt to open it as a camera
    
    if (!capture.isOpened())  { //if this fails, try to open as a video file or stream
        capture.open(arg);
        if (!capture.isOpened()) {
            cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
            help(av);
            return 1;
        }
    }

    return process(capture);
}
