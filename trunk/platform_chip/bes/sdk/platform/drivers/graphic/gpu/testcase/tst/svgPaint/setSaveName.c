#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "vg_lite_util.h"

void SaveRectSVG(int x,int y,int width,int height,vg_lite_buffer_t *fb){
	char save_name[50];
	char rect_name[50];
	char savename1[5] = "rect";
	char x_name[5];
	char y_name[5];
	char width_name[10];
	char height_name[10];
	itoa(x,x_name, 10);
	itoa(y,y_name, 10);
	itoa(width,width_name, 10);
	itoa(height,height_name, 10);
	sprintf(rect_name,"%s_x%s_y%s_width%s_height%s.png",savename1,x_name,y_name,width_name,height_name);
	strcpy(save_name,rect_name);
	vg_lite_save_png(save_name, fb);
}
