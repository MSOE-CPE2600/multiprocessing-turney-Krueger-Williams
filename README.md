# System Programming Lab 11 Multiprocessing

## Implementation

This program takes an argument (-n) with a number that then controls how many processes are then used by the program to generate the 50 mandel images. The processes are created using the fork command in a for loop with each process being spawned off of the parent process.
Additionally, this program takes a flag (-t) that sets the number of threads used for each image. For each thread, a structure with values for the image are passed including the start height and end height area that needs to be processed. This allows the image to be vertically split the specified number of regions and be processed by that number of threads. 

## Benchmarking Results

Benchmarks were ran using the command line 'time' command. Values of 1, 2, 5, 10, and 20 were used to benchmark. The results are below.

| Processes | Time (s) |
|:----------|----------:|
| 1   | 83.391 |
| 2   | 63.196 |
| 5   | 37.828 |
| 10  | 21.703 |
| 20  | 14.347 |

![Graph of data table](./Graph.png "Data table graph")

The results show that as more processes are added the program completes faster, however the speedup evens out towards the end as we reach the number of logical processor in the device. If we were to add more than 20 we would see it even out or possibly e slower due to making more processes without additional device processors.

## Benchmarking Results (Lab 12)

|         | Processes (Vertical) |        |        |        |        |        |
| ------- | --------- | ------ | ------ | ------ | ------ | ------ |
| Threads (Horizontal) |           | 1      | 2      | 5      | 10     | 20     |
|         | 1         | 80.03  | 56.417 | 39.328 | 22.572 | 17.042 |
|         | 2         | 63.856 | 45.528 | 25.234 | 17.123 | 12.463 |
|         | 5         | 46.315 | 27.465 | 15.22  | 11.614 | 10.856 |
|         | 10        | 26.095 | 17.469 | 12.188 | 13.603 | 12.429 |
|         | 20        | 14.276 | 10.898 | 10.837 | 12.358 | 11.914 |

The amount of time taken to run the different iterations are heavily dependent on both the thread count and the number of processors being used. It appears that the program is impacted more by the number of processors being used. This is most likely due to the earlier pictures in the sequence of 50 taking significantly longer to compute as supposed to the end ones. While breaking these images into smaller pieces helps, the program still has to wait for the slowest process to finish. Thus having more processes helps splits the early images up. It also seems as if there is an optimal run point at either 5 threads and 20 processes or 5 processes and 20 threads. These areas provided the best runtime of the bunch. This again is likely due to creating structures and saving variables between processes and threads, along with reaching the core limit on these computers. It is worth noting that a better approach would be to allocate the images better for the processes, finding a way to determine which images will take the longest to compute and splitting those evenly amongst the processes. This would help keep the processes at similar runtimes thus making the program execute quicker.