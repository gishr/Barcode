
////////////////////////////////////////////////////////////////
// Skeleton program for TRC3500
// Grabs images from a USB camera using OpenCV
// Written by Andy Russell 09th February 2006
// Modified by Michael Curtis 2011-2012 - updated for newer OpenCV
/////////////////////////////////////////////////////////////////
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <stdio.h>

using namespace std;

void thresholding(int threshold, uchar* row, int x, int gray);       //takes in: the desired threshold value, the uchar for the row, x location, gray value)

const char polarity[4][4] = {
    {'E', 'O', 'E', 'O'},
    {'O', 'E', 'O', 'E'},
    {'E', 'O', 'E', 'O'},
    {'O', 'E', 'O', 'E'} };


const int T_table[6][6][4] = { 
    { {0, 0, 0,0}, {0, 0, 0,0}, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 } },
    { {0, 0, 0,0}, {0, 0, 0,0}, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 } },
    { {0, 0, 0,0}, {0, 0, 0,0}, { 6,6,6,6 }, { 0,0,0,0 }, { 4,4,4,4 }, { 3,3,3,3 } }, //lookup table to decode the T-values 
    { {0, 0, 0,0}, {0, 0, 0,0}, { 9,9,9,9 }, { 0,2,8,0 }, { 7,1,0,0 }, { 5,5,5,5 } },
    { {0, 0, 0,0}, {0, 0, 0,0}, { 9,9,9,9 }, { 0,2,8,0 }, { 1,7,0,0 }, { 5,5,5,5 } },
    { {0, 0, 0,0}, {0, 0, 0,0}, { 6,6,6,6 }, { 0,0,0,0 }, { 4,4,4,4 }, { 3,3,3,4 } } };


////////////////////////////////////////////////////////////////

int main()
{
    CvCapture* capture = 0;
    IplImage* processedImage = 0;

    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1280);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 720);

    cout << "Group 2 " << endl << "Press 'q' to quit" << endl;

    // Initializes capturing video from camera
    capture = cvCaptureFromCAM(0);
    if (!capture) {
        fprintf(stderr, "Could not initialize capturing...\n");
        return -1;
    }


    // Creates window
    cvNamedWindow("Camera image", 1);
    //cvNamedWindow("Barcode", 1);

    // Camera image
    IplImage* frame = 0;
    //IplImage* barcode_image = 0;

    // Grabs and returns a frame from camera
    frame = cvQueryFrame(capture);

    // Print details of image
    cout << "image width =" << frame->width << " height =" << frame->height;
    cout << " depth =" << frame->depth << " channels =" << frame->nChannels << endl;
    cout << "Press 'w' to grab frame" << endl;

    do {
        // Grabs and returns a frame from camera
        frame = cvQueryFrame(capture);
        if (!frame) {
            break;
        }


        for (int y = 0; y < frame->height; y++) {
            // Iterate from middle top of image to the right
            for (int x = 0; x < frame->width; x++) {
                // This is a pointer to the start of the current row.
                //  Note: The image is stored as a 1-D array which is mapped back
                //  into 2-space by multiplying the widthStep (the image width rounded to
                //  a "nice" value, eg a multiple of 4 or 8 depending on the OS and CPU)
                //  by the row number.
                uchar* row = (uchar*)(frame->imageData + frame->widthStep * y);

                int gray = (row[x * 3] + row[x * 3 + 1] + row[x * 3 + 2]) / 3;

                row[x * 3] = gray;
                row[x * 3 + 1] = gray;
                row[x * 3 + 2] = gray;

                thresholding(70, row, x, gray);                 //call funtion with: the desired threshold value, the uchar for the row, x location, gray value of the pixel

            }
        }
   
        // Shows the resulting image in the window
        cvShowImage("Camera image", frame);

        //Image Grab Section//
        if ('w' == cvWaitKey(20)) {

            cout << "Image grabbed" << endl;

            int image_mid = frame->height / 2;      //half of the height
            int width = frame->width;               //width of image to use as length of array
            int* scan{ new int[width] {} };      //barcode is dynamically allocated the memory of however long the width of the image is

            uchar* row = (uchar*)(frame->imageData + frame->widthStep * image_mid);


            //Scanning middle of Barcode for pixels and filling array//
            for (int x = 0; x < frame->width; x++) {

                if (row[x * 3] == 0) {       //if pixel is dark, set barcode to true value
                    scan[x] = 1;
                }
                else {
                    scan[x] = 0;
                }
            }

            for (int i = 0; i < width; i++) {
                cout << scan[i];
            }
            cout << endl;



            int ct1 = 0, ct2 = 0, black_count = 0, white_count = 0, j = 0;
            int edges[30], bits[61];
            float unit = 0.0, Ti = 0.0;

            //Scanning for black and white bar lengths
            for (int i = 0; i < width;)
            {
                if (scan[i + 1] == scan[i])
                {
                    if (black_count) black_count++;
                    else if (white_count) white_count++;
                    i++;
                }

                else if (scan[i + 1] > scan[i])
                {
                    bits[j] = white_count;
                    white_count = 0;
                    black_count++;
                    i++;
                    j++;
                }

                else if (scan[i + 1] < scan[i])
                {
                    bits[j] = black_count;
                    black_count = 0;
                    white_count++;
                    i++;
                    j++;
                }

                if (j == 61) break;
            }


            //Scanning in the T-Values of the array
            int m = 0;
            //counter 2 calculates t1, 
            for (int i = 0; i < width;)
            {
                if (scan[i + 1] == scan[i])
                {                                       //iterate area
                    ct1++;
                    ct2++;
                    i++;
                }
                else if (scan[i + 1] > scan[i])
                {      //if the next value is dark and previous one is white
                    ct1++;
                    edges[m] = ct2;                     //count white before first black area // gives t1 values
                    m++;
                    i++;
                    ct2 = 0;
                }
                else if (scan[i + 1] < scan[i])
                {                                       //going from black area to white
                    ct2++;
                    edges[m] = ct1;
                    m++;
                    i++;
                    ct1 = 0;
                }
                if (m == 30) break;                      //max number of black strips is 30
            }


            //Convert T4e values to T4
            int k = 3;
            for (int i = 4; i < 29; i += 4)
            {
                edges[i] = edges[i] - bits[k];
                k += 4;
            }


            unit = (float)(edges[2] + edges[3]) / 4;            //dividing t1 and t2 of guard bar into 4 to find length of one bit

            //Conversion to modules
            for (int i = 0; i < 29; i++)
            {

                Ti = edges[i] / unit;                 //iterate through the array and convert to standard length

                if (Ti <= 1.5)
                    edges[i] = 1;
                else if (Ti <= 2.5)
                    edges[i] = 2;
                else if (Ti <= 3.5)
                    edges[i] = 3;

                else if (Ti <= 4.5)
                    edges[i] = 4;

                else if (Ti <= 5.5)
                    edges[i] = 5;

                else if (Ti <= 6.5)
                    edges[i] = 6;

                else if (Ti <= 7.5)
                    edges[i] = 7;

                else if (Ti <= 8.5)
                    edges[i] = 8;
            }

            //COUT for debugging purposes the T Values

            cout << "1 T1 = " << edges[7] << " T2 = " << edges[6] << " T3 = " << edges[5] << " T4 = " << edges[4] << endl;

            cout << "2 T1 = " << edges[11] << " T2 = " << edges[10] << " T3 = " << edges[9] << " T4 = " << edges[8] << endl;

            cout << "3 T1 = " << edges[15] << " T2 = " << edges[14] << " T3 = " << edges[13] << " T4 = " << edges[12] << endl;

            cout << "4 T1 = " << edges[19] << " T2 = " << edges[18] << " T3 = " << edges[17] << " T4 = " << edges[16] << endl;

            cout << "5 T1 = " << edges[23] << " T2 = " << edges[22] << " T3 = " << edges[21] << " T4 = " << edges[20] << endl;

            cout << "6 T1 = " << edges[27] << " T2 = " << edges[26] << " T3 = " << edges[25] << " T4 = " << edges[24] << endl;

            cout << endl;

            int dig1 = 0, dig2 = 0, dig3 = 0, dig4 = 0, dig5 = 0, dig6 = 0;

            dig1 = T_table[edges[7]][edges[6]][edges[4]-1];

            dig2 = T_table[edges[11]][edges[10]][edges[8]-1];

            dig3 = T_table[edges[15]][edges[14]][edges[12]-1];
        
            dig4 = T_table[edges[19]][edges[18]][edges[16]-1];

            dig5 = T_table[edges[23]][edges[22]][edges[20]-1];

            dig6 = T_table[edges[27]][edges[26]][edges[24]-1];

            cout << dig1 << dig2 << dig3 << dig4 << dig5 << dig6 << endl;
            cout << polarity[edges[7]-2][edges[6]-2] << polarity[edges[11] - 2][edges[10] - 2] << polarity[edges[15] - 2][edges[14] - 2]
                << polarity[edges[19] - 2][edges[18] - 2] << polarity[edges[23] - 2][edges[22] - 2] << polarity[edges[27] - 2][edges[26] - 2] << endl;

            
            //for (int y = 0; y < frame->height; y++) {
            //    // Iterate from middle top of image to the right
            //    for (int x = frame->width / 2; x < frame->width; x++) {

            //    }



            //    //Iterate from middle to the left
            //    for (int x = frame->width / 2; x >= 0; x--) {


            //    }

            //}
            delete[] scan;
            while ('w' != cvWaitKey(20));       //loop while w is not pressed again
        }


    } while ('q' != cvWaitKey(10));

    //tidy up

    // Releases the CvCapture structure
    cvReleaseCapture(&capture);
    // Destroys all the HighGUI windows
    cvDestroyAllWindows();

    return 0;

} //end of main 


void thresholding(int threshold, uchar* row, int x, int gray)      //takes in: the desired threshold value, the uchar for the row, x location, gray value
{
    // Thresholding
    if (gray < 90) {                       //set dark values to 0
        row[x * 3] = 0;
        row[x * 3 + 1] = 0;
        row[x * 3 + 2] = 0;

        
    }

    else {                                  //set light values to 255
        row[x * 3] = 255;
        row[x * 3 + 1] = 255;
        row[x * 3 + 2] = 255;
    }
}
