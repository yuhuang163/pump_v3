/*

 only supports uncompressed 24-bit RGB or 32-bit RGBA bitmaps.

*/

//g++ lodepng.cpp example_bmp2png.cpp -ansi -pedantic -Wall -Wextra -O3

#include "lodepng.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <fstream>
#include <dirent.h>

using namespace std;

/*

用法：
  GetDirAllFile("./train", "", vsFileName);   //获得文件夹的名字

  GetDirAllFile("./train", ".bmp", vsFileName);   //获得文件的名字

*/

void GetDirAllFile(string sPath, string sTemplate, vector<string> &_vsFileName)
{
    DIR *dir;   
    struct dirent *ptr;    
    _vsFileName.clear();
    string s = " ";
    string ss = " ";
    int n = sTemplate.length();

    if ((dir=opendir(sPath.c_str())) == NULL)   
    {   
        printf("Open dir error...");   
        
    }   
    // readdir() return next enter point of directory dir  
    while ((ptr=readdir(dir)) != NULL)   
    {    
        s = ptr->d_name; //cout<<s<<endl;
        
        if (n > 0)
        {
           if (n < s.length())
           {
              ss = s.substr(s.length()-n); //从索引11往后的子串
              if (!ss.compare(sTemplate)) //==
                _vsFileName.push_back(s);
           }
        }
        else
        {  
           if (s.compare(".") && s.compare("..")) //compare() 返回true,表示等于
           {
              _vsFileName.push_back(s);
           }
        }
    }
  
    closedir(dir);  
}

 
//returns 0 if all went ok, non-0 if error
//output image is always given in RGBA (with alpha channel), even if it's a BMP without alpha channel
unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, unsigned &c, const std::vector<unsigned char>& bmp) {
  static const unsigned MINHEADER = 54; //minimum BMP header size

  if(bmp.size() < MINHEADER) return -1;
  if(bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
  unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
  //read width and height from BMP header
  w = bmp[18] + bmp[19] * 256;
  h = bmp[22] + bmp[23] * 256;
  //read number of channels from BMP header
  if(bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
  unsigned numChannels = bmp[28] / 8;

  //The amount of scanline bytes is width of image times channels, with extra bytes added if needed
  //to make it a multiple of 4 bytes.
  unsigned scanlineBytes = w * numChannels;
  if(scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

  unsigned dataSize = scanlineBytes * h;
  if(bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

  //numChannels = 4; //可以强制将 A 增加到image

  c = numChannels;

  image.resize(w * h * numChannels);

  /*
  There are 3 differences between BMP and the raw image buffer for LodePNG:
  -it's upside down
  -it's in BGR instead of RGB format (or BRGA instead of RGBA)
  -each scanline has padding bytes to make it a multiple of 4 if needed
  The 2D for loop below does all these 3 conversions at once.
  */
  for(unsigned y = 0; y < h; y++)
  for(unsigned x = 0; x < w; x++) {
    //pixel start byte position in the BMP
    unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
    //pixel start byte position in the new raw image
    unsigned newpos = numChannels * y * w + numChannels * x;
    if(numChannels == 3) {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      //image[newpos + 3] = 255;            //A
    } else {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      image[newpos + 3] = bmp[bmpos + 3]; //A
    }
  }
  return 0;
}

// int split_img(std::vector<unsigned char>& image, unsigned tile_w, unsigned tile_h, std::vector<unsigned char>& bmp ){


// }

int encode_loseless( std::string bmpfile, std::string  pngfile, unsigned int unmesh_w = 1, unsigned int unmesh_h = 1 ){

  std::fstream _file;
  _file.open(bmpfile.c_str(),ios::in);
  if(!_file)
  {   
    printf("bmpfile : %s is not exist!!! \n", bmpfile.c_str() );
    return 0;        
  }

  bool bsplit_output_file = false;


  std::vector<unsigned char> bmp;
  lodepng::load_file(bmp, bmpfile);
  std::vector<unsigned char> image;
  unsigned w, h, c;
  unsigned error = decodeBMP(image, w, h, c, bmp);
  if(error) {
    std::cout << "BMP decoding error " << error << std::endl;
    return 0;
  }

  printf("bes: raw data info: w = %d, h = %d, c = %d, size = %d\n", w, h, c, w*h*c );

  // out put file name 
  string sOut_put_dir;
  if (bsplit_output_file){

    sOut_put_dir = pngfile;
  }
   

  // 计算 split 

  LodePNGColorType eColor = LCT_RGBA;

  unsigned in_chanel = c;

  unsigned out_chanel = 4;
  
  if (out_chanel==4){ 
     eColor = LCT_RGBA ;
     } 
   else if( out_chanel==3){
     eColor = LCT_RGB;

   }
   else{
      printf("error : no support channel , in_chanel = %d, out_chanel = %d\n", in_chanel, out_chanel );

   }
    printf("in_chanel = %d, out_chanel = %d\n", in_chanel, out_chanel );



  const unsigned char* in = &image[0];


  unsigned int untile_w = w / unmesh_w ;
  unsigned int untile_h = h / unmesh_h;

  unsigned int untilelineBytes_in = untile_w * in_chanel;
  unsigned int untilelineBytes_out = untile_w * out_chanel;



  std::vector<unsigned char> tile;
  tile.resize(untile_w * untile_h * out_chanel); // for encode 
  unsigned char* in_tile = &tile[0];


  size_t un_tile_size =0;
  for ( int i = 0; i < unmesh_h; i++ )
  {
    for (int j = 0; j < unmesh_w; j++) // 一个一个 tile 的 encode
  // int i = 3;
  // int j = 3;
    {
      //取得tile的内容
       for ( int m = 0; m < untile_h; m++)
       {
          for ( int n = 0; n < untile_w; n++)       
          {
              unsigned inPos =  i * untile_h * w * in_chanel  + j * untilelineBytes_in + m * w * in_chanel + n * in_chanel; // 头两项为该 tile 左上角
              //pixel start byte position in the new raw image
              unsigned tilePos = m * untilelineBytes_out + out_chanel * n;

              if(out_chanel == 4 && in_chanel == 3 ){

                  in_tile[tilePos + 0] = in[inPos + 0]; //R
                  in_tile[tilePos + 1] = in[inPos + 1]; //G
                  in_tile[tilePos + 2] = in[inPos + 2]; //B
                  in_tile[tilePos + 3] = 255;            //A


              }
              else if(out_chanel == 3 && in_chanel == 3 ) {

                    in_tile[tilePos + 0] = in[inPos + 0]; //R
                    in_tile[tilePos + 1] = in[inPos + 1]; //G
                    in_tile[tilePos + 2] = in[inPos + 2]; //B
                    //image[newpos + 3] = 255;            //A
                  } 
              else if(out_chanel == 4 && in_chanel == 4 )  {

                    in_tile[tilePos + 0] = in[inPos + 0]; //R
                    in_tile[tilePos + 1] = in[inPos + 1]; //G
                    in_tile[tilePos + 2] = in[inPos + 2]; //B
                    in_tile[tilePos + 3] = in[inPos + 3]; //A
              }

          }
         
       }

      std::vector<unsigned char> png;
      size_t bug_size=0; 
      error = lodepng::encode(png, in_tile, untile_w, untile_h, eColor, 8, &bug_size);
      if(error) {
        std::cout << "PNG encoding error " << error << ": " << lodepng_error_text(error) << std::endl;
        return 0;
      }
      un_tile_size += bug_size;

      if( bsplit_output_file)
      {
          char str_tile_output_file[256];
          sprintf(str_tile_output_file, "%s_%d_%d.png", pngfile.c_str(), i, j);
          lodepng::save_file(png, str_tile_output_file);
      }
      else{ // merge buffer 
          lodepng::save_file(png, pngfile);
      }

    }
  }
  if( bsplit_output_file){
      printf("bes: compress data info: total size = %ld\n", un_tile_size);
  }
  else{

  }

}

void decToBin(int num,int base)
{
  if(num>0)
  {
    decToBin(num/base,base);
    printf("%d",num%base);
  }

}

// 归并排序
void debug_test_sort_merge()
{
  unsigned a[12] = {1,3,5,2,0,5,6,7,9,4,5,6}; 
  debug_mergesort( a, 12);
}

// 
void debug_test_HuffmanTree_code()
{
  char str[] = "aaaabbbbccccccccccdfsdfsfgffhfherjyljfdljptyk';tky;gfkh;dfkjl;sjdlfgjlsjhfldgkml;sdkgld;kgsdg;'sk;hgfkwelmnflsd\0"; 
  debug_HuffmanTree_makeFromFrequencies( str, 8);
}

void debug_test_reverseBits()
{
  // 逆序测试
  /*
  unsigned symbol = 221;
  decToBin(symbol,2);
   printf("%\n");
  unsigned num =5;
  //for( int i = 0; i < symbol ; i++){
  unsigned ret = debug_reverseBits( symbol, num);
  // printf(" i = %d, result= %d\n", i, ret);

  printf("result= %d\n",ret);
  decToBin(ret,2);
  printf("result= %d\n", ret);
  */

  // 逆序数的意义： 动态中位数 ???
  unsigned symbol = 32;
   printf("%\n");
  unsigned num =4;
  for( int i = 0; i < symbol ; i++){
    unsigned ret = debug_reverseBits( i, num);
    printf(" i = %d, result= %d\n", i, ret);
  }

  

  //symbol = symbol >> (6 - num);


}
int main(int argc, char *argv[]) {
  // if(argc < 3) {
  //   std::cout << "Please provide input BMP and output PNG file names" << std::endl;
  //   return 0;
  // }

  /*
   用法 ：
    ./loseless /home/lqz/ar_vr/ar_vr/test_code/img_file/png/panel/001_true_color_ori.bmp /home/lqz/ar_vr/ar_vr/test_code/img_file/png/panel/test_out/001_true_color_ori



  */

    char fsrcFileName[256];
	  char fobjFileName[256];

    strcpy(fsrcFileName,argv[1]);  


    if(argc < 3){



        strcpy(fobjFileName, fsrcFileName);
        unsigned long len_filename=strlen(fsrcFileName);
        strcpy(fobjFileName+(len_filename-3),"png");

    }
    else{
        
        strcpy(fobjFileName,argv[2]);

    }


  encode_loseless(fsrcFileName, fobjFileName, 1 ,1);

  //debug_test_sort_merge();

  //debug_test_HuffmanTree_code();

  //debug_test_reverseBits();

}
