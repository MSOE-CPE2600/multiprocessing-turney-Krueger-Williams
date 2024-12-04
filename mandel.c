/********************************************************************
 *  Filename: mandel.c
 *  Description: Provides a program that generates 50 mandelbrot
 * 			images designed to be put into a movie
 *  Author: Krueger 'Mac' Williams
 *  Date:  12/4/2024
 *  Note: compile with make
 * 			execute with ./mandel -h
 *******************************************************************/

/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
///
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <string.h>
#include <pthread.h>

#define IMAGES_TO_GENERATE 50

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int num_threads );
static void show_help();

static void* thread_separate();

struct thread_data {
	imgRawImage* img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int height;
	int width;
	int start_height;
	pthread_mutex_t mutex;
};

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.
	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;

	int	   num_processes = 1; // Determine number of processing
	int    num_threads = 1; // Number of threads used for image gen

	// For each command line argument given,
	// override the appropriate configuration value.
	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				num_processes = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)
	yscale = xscale / image_width * image_height;

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max,outfile);

	int images_per_process = IMAGES_TO_GENERATE / num_processes; // Number of images for each process
	int remainder = IMAGES_TO_GENERATE % num_processes;	// Capture the remaining images

	int main_pid = getpid(); // Main PID of program
	printf("MAIN PID: %d\n",main_pid);
	int assignments[num_processes][2];
	assignments[0][0] = getpid();
	assignments[0][1] = 0;

	for (int i = 0; i < num_processes - 1; i++) {
		if (getpid() == main_pid) { // Check if the main process calls
			fork();
			if (getpid() != main_pid && getpid() != 0) {
				assignments[i + 1][0] = getpid();
				assignments[i + 1][1] = i + 1;
			}
			
		}
	}

	int assignment_block; // Process assignment number
	int start_value;	// Image start number

	// Get process assignment and image start number
	for (int i; i < num_processes; i++) {
		//printf("%d %d\n",i,assignments[i][0]);
		
		if (assignments[i][0] == getpid()) {
			printf("Assignment: %d %d\n",assignments[i][0], getpid());
			assignment_block = i;
			start_value = assignment_block * images_per_process;
		}
	}

	// Generate images for processing block
	for (int i = 1; i <= images_per_process; i++) {
		char file_name[20];
		
		sprintf(file_name, "mandel%d.jpg",start_value + i);
		
		// Create a raw image of the appropriate size.
		imgRawImage* img = initRawImage(image_width,image_height);

		// Fill it with a black
		setImageCOLOR(img,0);

		// Set scaling
		double current_xscale = xscale + 0.1 * (start_value + i);
		double current_yscale = current_xscale / image_width * image_height;
		
		
		// Compute the Mandelbrot image
		compute_image(img, xcenter - current_xscale / 2, xcenter + current_xscale / 2, ycenter - current_yscale / 2, ycenter + current_yscale / 2, max, num_threads);

		// Save the image in the stated file.
		storeJpegImageFile(img,file_name);

		// free the mallocs
		freeRawImage(img);
		printf("%s\n",file_name);
	}
	
	// Assign remainders
	if (assignment_block < remainder) {
		char file_name[20];
		int image_number = images_per_process * num_processes + assignment_block + 1;
		sprintf(file_name, "mandel%d.jpg",image_number);

		// Create a raw image of the appropriate size.
		imgRawImage* img = initRawImage(image_width,image_height);

		// Fill it with a black
		setImageCOLOR(img,0);

		// Set scaling
		double current_xscale = xscale + 0.2 * image_number; 
		double current_yscale = current_xscale / image_width * image_height;

		// Compute the Mandelbrot image
		compute_image(img, xcenter - current_xscale / 2, xcenter + current_xscale / 2, ycenter - current_yscale / 2, ycenter + current_yscale / 2, max, num_threads);

		// Save the image in the stated file.
		storeJpegImageFile(img,file_name);

		// free the mallocs
		freeRawImage(img);

		printf("%s\n",file_name);
	}

	if (getpid() != main_pid) {
		return 0;	// End process not main
	} else {
		for (int i = 0; i < num_processes - 1; i++) {
			wait(NULL);	// Wait for other processes to end
		}
	}


	printf("\n\nCreation Completed\n");

	return 0;
}




/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads)
{
    // Create array of threads
	pthread_t threads[num_threads];

    int height = img->height;	// Get image height
    int split_height = height / num_threads;	// Divide height by threads

    for (int i = 0; i < num_threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data)); // Allocate separate data for each thread
        data->img = img;
        data->max = max;
        data->xmax = xmax;
        data->xmin = xmin;
        data->ymax = ymax;
        data->ymin = ymin;
        data->width = img->width;

        // Set starting and ending height for each thread
        data->start_height = i * split_height;

		// Check if last thread
        if (i == num_threads - 1) {
			data->height = height;
		} else {
			// Give last thread any remaining rows
			data->height = (i + 1) * split_height;
		}


        // Create thread
        int result = pthread_create(&threads[i], NULL, &thread_separate, data);
        if (result != 0) {
            printf("Unable to make thread\n");
            free(data);
        }
    }

    // Wait for the threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Method for threads designed to process a set part of the image
void* thread_separate(void* arg) {
    struct thread_data *data = arg;
    int width = data->width;
    int start_height = data->start_height;
    int end_height = data->height;
    int total_height = data->img->height;
    int i, j;

	// Process specific region of image
    for (j = start_height; j < end_height; j++) {
        for (i = 0; i < width; i++) {
            // Determine the point in x, y space for that pixel.
            double x = data->xmin + i * (data->xmax - data->xmin) / width;
            double y = data->ymin + j * (data->ymax - data->ymin) / total_height;

            // Compute the iterations at that point.
            int iters = iterations_at_point(x, y, data->max);

            // Set the pixel in the bitmap.
            setPixelCOLOR(data->img, i, j, iteration_to_color(iters, data->max));
        }
    }

	// Free data structure passed to thread
    free(data);
    return NULL;
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}