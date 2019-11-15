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
  int sample_count = 50;
  int width = 90;
  int height = 90;
  int diff_count = 0;
};


// prototypes
PNG_IMAGE generate(PNG_IMAGE &input, parameters &params);
void process_pixel(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v);
PNG_PIXEL find_match(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v);
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
  PNG_PIXEL match = find_match(input, output, params, progress, u, v);
  
  output.set_pixel(u, v, match);
  
  // mark as processed
  progress[u][v] = 1;
}


PNG_PIXEL find_match(PNG_IMAGE &input, PNG_IMAGE &output, parameters &params, int ** progress, int u, int v)
{
  // find minimum diff
  int minDiff = 768 * params.sample_size * params.sample_size;
  PNG_PIXEL match = input.get_pixel(0, 0);
  int minX = 0;
  int minY = 0;
  
  // dimension
  int inputWidth = input.get_width();
  int inputHeight = input.get_height();
  
  // selection range
  int clampW = inputWidth - params.sample_size;
  int clampH = inputHeight - params.sample_size;
  int clampOffset = params.sample_size * 0.5f;
  
  for(int i = 0; i < params.sample_count; i++)
  {
    // select random sample coordinate
    int x = clampOffset + std::rand() % clampW;
    int y = clampOffset + std::rand() % clampH;
    
    int diff = area_diff(input, output, params, progress, x, y, u, v);
    
    if(diff < minDiff)
    {
      minDiff = diff;
      minX = x;
      minY = y;
    }
  }
  
  match = input.get_pixel(minX, minY);
  
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
