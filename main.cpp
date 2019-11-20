// libpng++ library
#include <png++/png.hpp>

// include
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <ctime>

// replace
#define PNG_IMAGE png::image<png::rgb_pixel>
#define PNG_PIXEL png::rgb_pixel


// parameters
struct parameters
{
  int sample_size = 6;
  int sample_count = 20;
  int double_sample_size = 0;
  int double_sample_count = 0;
  int width = 90;
  int height = 90;
  int diff_count = 0;
};

// 2d point
struct point2D
{
	int x = 0;
	int y = 0;
};


// prototypes
PNG_IMAGE generate(PNG_IMAGE &input, parameters &params);
void process_pixel(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v);
point2D find_match(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v, int x0 = 0, int x1 = 0, int y0 = 0, int y1 = 0, bool double_sample = false);
int area_diff(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int x, int y, int u, int v);
int pixel_diff(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int x, int y, int u, int v);
int set_avg_diff_time(int time, int count);
int get_avg_diff_time();


// start procedure
int main(int argc, char ** argv)
{
  // random
  std::srand(std::time(0));
  
  // process parameters
  parameters params;
  
  // filenames
  std::string inputName = "input.png";
  std::string outputName = "output.png";
  if(argc >= 2)
    inputName = argv[1];
  if(argc >= 3)
    params.sample_size = std::atoi(argv[2]);
  if(argc >= 4)
    params.sample_count = std::atoi(argv[3]);
  if(argc >= 6)
  {
    params.width = std::atoi(argv[4]);
    params.height = std::atoi(argv[5]);
  }
  if(argc >= 8)
  {
  	params.double_sample_size = std::atoi(argv[6]);
  	params.double_sample_count = std::atoi(argv[7]);
  }
  
  // load
  PNG_IMAGE input(inputName);
  
  // render
  PNG_IMAGE output = generate(input, params);
  output.write(outputName);
  
  
  return 0;
}


// implementation
PNG_IMAGE generate(PNG_IMAGE &input, parameters &params)
{
  // create new canvas
  PNG_IMAGE output(params.width, params.height);
  
  // progress mask
  int ** progress = new int*[params.width];
  for(int x = 0; x < params.width; x++)
  {
    progress[x] = new int[params.height];
    for(int y = 0; y < params.height; y++)
      progress[x][y] = 0;
  }
  
  
  // get center
  int centerX = params.width / 2;
  int centerY = params.height / 2;
  
  // max dimension
  int maxSquareSize = params.width;
  if(params.height > params.width)
    maxSquareSize = params.height;
  
  // extra margin
  maxSquareSize += 5;
  
  
  
  // copy center pixel
  int inputWidth = input.get_width();
  int inputHeight = input.get_height();
  PNG_PIXEL startingPixel = input.get_pixel(inputWidth * 0.5f, inputHeight * 0.5f);
  output.set_pixel(centerX, centerY, startingPixel);
  progress[centerX][centerY] = 1;
  
  
  // iterate along square (grows in size)
  for(int s = 0; s < maxSquareSize; s += 2)
  {
    printf("s: %d\n", s);
    int x0 = centerX - s * 0.5f;
    int y0 = centerY - s * 0.5f;
    int x1 = x0 + s;
    int y1 = y0 + s;
    
    // top and bottom walls
    for(int x = x0; x <= x1; x++)
    {
      process_pixel(input, output, params, progress, x, y0);
      process_pixel(input, output, params, progress, x, y1);
    }
    
    // left and right walls
    for(int y = y0 + 1; y < y1; y++)
    {
      process_pixel(input, output, params, progress, x0, y);
      process_pixel(input, output, params, progress, x1, y);
    }
  }
  
  
  return output;
}


void process_pixel(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v)
{
  // out of bounds
  if(u < 0 || v < 0 || u >= params.width || v >= params.height)
    return;
  
  // already processed
  if(progress[u][v] == 1)
    return;
  
  // apply match
  
  // single sample
  point2D match = find_match(input, output, params, progress, u, v);
  
  // double sample;
  int x0 = match.x - params.double_sample_size * 0.5f;
  int x1 = match.x + params.double_sample_size * 0.5f;
  int y0 = match.y - params.double_sample_size * 0.5f;
  int y1 = match.y + params.double_sample_size * 0.5f;
  match = find_match(input, output, params, progress, u, v, x0, x1, y0, y1, true);
  
  // retrieve pixel
  PNG_PIXEL inputPixel = input.get_pixel(match.x, match.y);
  
  output.set_pixel(u, v, inputPixel);
  
  // mark as processed
  progress[u][v] = 1;
}


point2D find_match(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v, int x0, int x1, int y0, int y1, bool double_sample)
{
  // find minimum diff
  int minDiff = 768 * params.sample_size * params.sample_size;
  point2D match;
  int minX = 0;
  int minY = 0;
  
  // dimension
  int inputWidth = input.get_width();
  int inputHeight = input.get_height();
  
  // selection range
  if(x0 >= x1)
  {
  	x0 = 0;
  	x1 = inputWidth;
 	}
  if(y0 >= y1)
  {
  	y0 = 0;
  	y1 = inputHeight; 	
 	}
  x0 = std::max(x0, int(params.sample_size * 0.5f));
  x1 = std::min(x1, int(inputWidth - params.sample_size * 0.5f));
  y0 = std::max(y0, int(params.sample_size * 0.5f));
  y1 = std::min(y1, int(inputHeight - params.sample_size * 0.5f));
  int clampW = x1 - x0;
  int clampH = y1 - y0;
  
  // determine sample count
  int sample_count = params.sample_count;
  if(double_sample)
  	sample_count = params.double_sample_count;
  
  // do samples
  for(int i = 0; i < params.sample_count; i++)
  {
    // select random sample coordinate
    int x = x0 + std::rand() % clampW;
    int y = y0 + std::rand() % clampH;
    
    int diff = area_diff(input, output, params, progress, x, y, u, v);
    
    if(diff < minDiff)
    {
      minDiff = diff;
      minX = x;
      minY = y;
    }
  }
  
  // return position
  match.x = minX;
  match.y = minY;
  
  return match;
}


int area_diff(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int x, int y, int u, int v)
{
  // iteration offset
  int s0 = params.sample_size * -0.5f;
  int s1 = s0 + params.sample_size;
  
  // dimensions
  int inputWidth = input.get_width();
  int inputHeight = input.get_height();
  
  int diff = 0;
  
  for(int i = s0; i <= s1; i++)
  {
    for(int j = s0; j <= s1; j++)
    {
      diff += pixel_diff(input, output, params, progress, x + i, y + j, u + i, v + j);
    }
  }
  
  return diff;
}


int pixel_diff(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int x, int y, int u, int v)
{
  // input(x, y)
  // output(u, v)
  
  // out of bounds
  if(u < 0 || v < 0 || u >= params.width || v >= params.height)
    return 0;
  
  // dimensions
  int inputWidth = input.get_width();
  int inputHeight = input.get_height();
  
  // out of bounds
  if(x < 0 || y < 0 || x >= inputWidth || y >= inputHeight)
    return 0;
  
  
  // no progress
  if(progress[u][v] == 0)
    return 0;
  
  
  int diff = 0;
  
  PNG_PIXEL inputPixel = input.get_pixel(x, y);
  PNG_PIXEL outputPixel = output.get_pixel(u, v);
  
  // diff red
  diff += abs(inputPixel.red - outputPixel.red);
  
  // diff green
  diff += abs(inputPixel.green - outputPixel.green);
  
  // diff blue
  diff += abs(inputPixel.blue - outputPixel.blue);
  
  // increment diffcount
  params.diff_count++;
  
  return diff;
}
