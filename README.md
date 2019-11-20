# Time Complexity
The calculation of this algorithm on CPU alone is relatively slow for accurate rendering, so I am providing the time complexity in detail.
> 
> 2 * (sample_size)^2 * sample_count * output_width * output_height
> 
This time complexity is represented in terms of the total number of pixel_diff calls in the program execution, the most atomic operation of the program.

As you can see, the "sample_size^2" term expands quadratically, so it is easily the heaviest burden on the calculation time. "sample_count" is responsible for how many random samples are collected, so reducing it for the sake of performance will directly reduce the quality of the output image. Doing so results in a "fuzzy" render and is noticable.

# IMPROVEMENT FEATURE!
>Double Sampling Method

I implemented a new feature to be more strategic in samples. I call this "Double Sampling", in which the best match sample is taken, and then fed back into the find_match() function to look for samples within a small area. The concept is to throw out a fewer number of samples, but to take the best one and inch it over to a slightly better position, as it is likely not exactly the best sample. Think of this like a binary tree search of depth 2, though it's more similar to what's called "spacial hashing", so this will drastically improve render quality while saving time.

Take a look at "output_100x_single_sample.png" and "output_100x_double_sample.png" and observe the fuzziness of the single-sampled iteration. Both used the same number of samples, but the double-sampled iteration redistributed 15 samples out of the sample_count term, and fed those samples into double-sampling, where it takes a more narrow search. This produced a better image for both 100x and 200x. Since this program is not multithreaded, nor is it GPU accelerated, it will run slowly when searching for high quality results, but this "Double Sample" method at least allows for better results with fewer resources available.


# texturesynth
C++ Implementation of Texture Synthesis via Non-Parametric Sampling

# Collaberated with Kevin Ngyuen (1495435)

# Dependencies
>libpng++-dev
>
>install: sudo apt install libpng++-dev

# How to Build
execute build.sh on Linux via:
>./build.sh

# How to Execute
execute hw5 on Linux via:
>./hw5 <input_file> <sample_size> <sample_count> <width> <height>
>
> "input_file" is the sample png image
>
> "sample_size" is the size of neighborhood reads
>
> "sample_count" is the number of neighborhoods sampled from the source image
>
> "width" and "height" are the output dimensions
>
> "double_sample_size" is the size of the double-sampling window
>
> "double_sample_count" is the number of double-samples taken. Redistribute some of "sample_count" into "double_sample_count" to improve image quality.
