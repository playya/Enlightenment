#include "Etox.h"
#include "loadfile.h"

E_Font_Style **style_list;
int            num_styles=0;

static int     fspath_num = 0;
static char  **fspath = NULL;

void etox_add_path_to_font_style_path(char *path) {
  fspath_num++;
  if (!fspath)
    fspath = malloc(sizeof(char *));
  else
    fspath = realloc(fspath, (fspath_num * sizeof(char *)));
  fspath[fspath_num - 1] = strdup(path);
}

void etox_remove_path_to_font_style_path(char *path) {
  int i, j;
  
  for (i = 0; i < fspath_num; i++)
    {
      if (!strcmp(path, fspath[i]))
	{
	  fspath_num--;
	  for (j = i; j < fspath_num; j++)
	    fspath[j] = fspath[j + 1];
	  if (fspath_num > 0)
	    fspath = realloc(fspath, fspath_num * sizeof(char *));
	  else
	    {
	      free(fspath);
	      fspath = NULL;
	    }
	}
    }
}

char **etox_list_font_style_path(int *number_return) {
  *number_return = fspath_num;
  return fspath;
}

void E_Font_Style_calculate_shift(E_Font_Style *style) 
{
   int i;
   
   if (style->num_bits)
     {
	style->left = style->right = style->bits[0].x;
	style->up   = style->down  = style->bits[0].y;
	for(i=1; i<style->num_bits; i++)
	  {
	     if (style->bits[i].x < style->left)
	       style->left = style->bits[i].x;
	     if (style->bits[i].x > style->right)
	       style->right = style->bits[i].x;
	     if (style->bits[i].y < style->down)
	       style->down = style->bits[i].y;
	     if (style->bits[i].y > style->up)
	       style->up = style->bits[i].y;
	  }
     }     
   else
     style->left = style->right = style->up = style->down = 0;
}

E_Font_Style *E_load_font_style(char *path) {

  E_Font_Style *style;
  FILE *font_file;
  char s[4096];
  int i1, i2, i3, fields;
  char s2[4096];
  char nbuf[4096];
  
  if(!path)
    return NULL;
  
  style = malloc(sizeof(E_Font_Style));
  style->in_use = 0;
  
  style->bits = NULL;
  style->name = malloc((strlen(path) * sizeof(char)) + 1);
  strcpy(style->name,path);
  style->num_bits = 0;
  
  /* Look for the style file */
  
  if (!fileGood(path))
    {
      int ok = 0;
      int fspath_iter = 0;

      if (!strstr(path,".style") )
	{
	  strncpy(nbuf, path, 4000);
	  strcat(nbuf, ".style");
	  
	  ok = fileGood(nbuf);
	}
      while (!ok) {
	if (fspath_iter >= fspath_num)
	  return NULL;
	strncpy(nbuf, fspath[fspath_iter], 4000);
	if (!(nbuf[strlen(nbuf)-1] == '/'))
	  strcat(nbuf,"/");
	strncat(nbuf, path, (4000 - strlen(nbuf)));
	ok = fileGood(nbuf);
	if (!ok)
	  if (!strstr(path,".style") )
	    {
	      strcat(nbuf, ".style");
	      
	      ok = fileGood(nbuf);
	    }
	fspath_iter++;
      }
    }
  else
    strncpy(nbuf, path, 4000);

  font_file = fopen(nbuf,"r");
  while(GetLine(s,4096,font_file)) {
    i1=i2=i3=0;
    memset(s2,0,4096);
    fields = sscanf(s,"%4000[^=]= %i %i %i",s2,&i1,&i2,&i3);
    if(fields < 3) {
      fclose(font_file);
      return style;
    }
    style->num_bits++;
    if(style->bits){
      style->bits = realloc(style->bits,(style->num_bits * 
					 sizeof(E_Style_Bit) + 1));
    } else {
      style->bits = malloc(style->num_bits * sizeof(E_Style_Bit) + 1);
    }
    if(!strcmp(s2,"sh"))
      style->bits[style->num_bits - 1].type = STYLE_TYPE_SHADOW;
    if(!strcmp(s2,"fg"))
      style->bits[style->num_bits - 1].type = STYLE_TYPE_FOREGROUND;
    if(!strcmp(s2,"ol"))
			style->bits[style->num_bits - 1].type = STYLE_TYPE_OUTLINE;
    style->bits[style->num_bits - 1].x = i1;
    style->bits[style->num_bits - 1].y = i2;
    style->bits[style->num_bits - 1].alpha = i3;
  }
  fclose(font_file);

   E_Font_Style_calculate_shift(style);
  (style->in_use)++;
  return style;
  
}

void E_Font_Style_free(E_Font_Style *style) {

	if(!style)
		return;

	(style->in_use)--;

	if(style->in_use <=0) {
		if(style->name)
			free(style->name);
		if(style->bits)
			free(style->bits);
		free(style);
	}
}
