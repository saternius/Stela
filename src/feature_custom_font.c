#include <stdlib.h>
#include <string.h>
#include "pebble.h"
#include "pebble_fonts.h"

#define ACCEL_STEP_MS 50

static Window *window;
static TextLayer *display_text;
static TextLayer *connection_text;
static BitmapLayer *rewind_layer;
Layer *window_layer;
bool menu = true;
static AppTimer *timer;
static char *rate_text;
static BitmapLayer *image_layer;
GBitmap *image;
GBitmap *font_banner; 
GBitmap *game_bg;
GBitmap *rewind_icon;
GBitmap *pause_icon;
GBitmap *nothing_icon;


enum {MENU,BOOK,SETTINGS} frame;
const char* fonts[4] = {FONT_KEY_GOTHIC_28, FONT_KEY_GOTHIC_28_BOLD, FONT_KEY_ROBOTO_CONDENSED_21 };
GFont disp_font;
int text_x = -20;
int text_y = 70;
int font_id = 0;
int hold = 5;
int speed = 40;


static char* body_text[200]; 
int max_length = 12;
int head_char=0;
int space_pos = 0;
int tail_char=200-1;
bool end = false;
int push_x = 0;
bool pause = true;
bool started = false;
bool fast_rewind  = false;

char* con_text;
int con_timer = 0;


static void hard_code(){
  //int wps = ((125/speed)*60);
  if(speed == 100){
  }else if(speed == 95){
    text_layer_set_text(connection_text, "WPS: 375");
  }else if(speed == 90){
    text_layer_set_text(connection_text, "WPS: 300");
  }else if(speed == 85){
    text_layer_set_text(connection_text, "WPS: 250");
  }else if(speed == 80){
    text_layer_set_text(connection_text, "WPS: 214");
  }else if(speed == 75){
    text_layer_set_text(connection_text, "WPS: 187");
  }else if(speed == 70){
    text_layer_set_text(connection_text, "WPS: 166");
  }else if(speed == 65){
    text_layer_set_text(connection_text, "WPS: 136");
  }else if(speed == 60){
    text_layer_set_text(connection_text, "WPS: 125");
  }else if(speed == 55){
    text_layer_set_text(connection_text, "WPS: 115");
  }else if(speed == 50){
    text_layer_set_text(connection_text, "WPS: 107");
  }else if(speed == 45){
    text_layer_set_text(connection_text, "WPS: 100");
  }else if(speed == 40){
    text_layer_set_text(connection_text, "WPS: 93");
  }else if(speed == 35){
    text_layer_set_text(connection_text, "WPS: 88");
  }else if(speed == 30){
    text_layer_set_text(connection_text, "WPS: 83");
  }else if(speed == 25){
    text_layer_set_text(connection_text, "WPS: 78");
  }else if(speed == 20){
    text_layer_set_text(connection_text, "WPS: 75");
  }
  
  
}




static void getNextWord(char *string[200], char* word[30]){
  if(end){
    return;
  }
  if(word==NULL){
    //nothing this is stupid
  }
  
    int space_dex = -1;
    int length = strlen(*string);
    //finds index of next space if it exists, if not return done.
    int current_pos = 0;
    for(int i=0; i<length; i++){
      if((*string)[i]==' '){
        current_pos++;
        if(current_pos == space_pos+1){
           space_pos++;
           space_dex = i;
           //APP_LOG(APP_LOG_LEVEL_DEBUG,"space_dex = %u",space_dex);
           break;
        }
      }
    }
  *word = "";
  
  
   //APP_LOG(APP_LOG_LEVEL_DEBUG,"outside space_dex = %u",space_dex);
   if(space_dex== -1){ //tere are no spaces so return done.
     int move = 0;
     while((*string)[head_char]!='\0'){
       (*word)[move] = (*string)[head_char];
       move++;
       head_char++;
     }
     end = true;
       //APP_LOG(APP_LOG_LEVEL_DEBUG,"DONE");
      return;
   }
  
  
  for(int k=0; k<30; k++){
    (*word)[k] = '\0';
  }
  //char * next_word[30];
  //copies the word to next word
  int mov = 0;
  for(int j=head_char+1; j<space_dex; j++){
      (*word)[mov] = (*string)[j];
    mov++;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG,*word);
  head_char = space_dex;
  //word = next_word;  
}


static void getPreviousWord(char *string[200], char* word[30]){
  if(head_char<=1){
    return;
  }
 
  if(word==NULL){
    //nothing this is stupid
  }
  
    int space_dex = -1;
    int length = strlen(*string);
    //finds index of next space if it exists, if not return done.
    int current_pos = 0;
    for(int i=0; i<length; i++){
      if((*string)[i]==' '){
        current_pos++;
        if(current_pos == space_pos-1){
           space_pos--;
           space_dex = i;
           //APP_LOG(APP_LOG_LEVEL_DEBUG,"space_dex = %u",space_dex);
           break;
        }
      }
    }
  *word = "";
  
  
   //APP_LOG(APP_LOG_LEVEL_DEBUG,"outside space_dex = %u",space_dex);
   if(space_dex<1 || space_pos<1 || head_char<0){ //tere are no spaces so return done.
     return;
   }

  for(int k=0; k<30; k++){
    (*word)[k] = '\0';
  }
  //char * next_word[30];
  //copies the word to next word
  int mov = 0;
  for(int j=space_dex; j<head_char+1; j++){
      (*word)[mov] = (*string)[j];
       mov++;
  }
  //APP_LOG(APP_LOG_LEVEL_DEBUG,*word);
  head_char = space_dex;
  //word = next_word;  
}


int getLength(char* word[30]){
  if(end){
    return 100;
  }
  int length =0;
  for(int i=0;i<30;i++){
    if((*word)[i]!='\0'){
      length++;
    }
  }
  return length;
}
void redraw_text(int type){
  if(type == 0){
  hold--;
  }
  if(hold<0 || type>0){
      GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
      char* word[30];
      if(type == 0 || type == 2){
        getNextWord(body_text,word);
      }else if(type == 1){
         getPreviousWord(body_text,word);
      }
      int wordLength=0;
      if(end){
          hold = 1000000;
      }else{
        wordLength = getLength(word);
        hold = (125/speed) + (wordLength*4/speed);
        int shift_x = 0;
        if(wordLength<=2){
           shift_x = 1;
        }else
        if(wordLength <= 5){
          shift_x = 2;
        }else if(wordLength <= 8){
          shift_x = 3;
        }else if(wordLength <= 12){
          shift_x = 4;
        }else if(wordLength <= 14){
          shift_x = 5;
        }else if(wordLength <= 16){
          shift_x = 6;
        }else if(wordLength <= 20){
          shift_x = 7;
        }
        
        char crop[20] = "";
        for(int i=0; i<shift_x; i++){
          crop[i] = (*word)[i];
        }
        
        GSize size = graphics_text_layout_get_content_size(crop,disp_font,move_pos2,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter);	
        int16_t size_x = size.w;
        //APP_LOG(APP_LOG_LEVEL_DEBUG,"shift_x = %i theSize = %i",shift_x,size_x);
        push_x = size_x/2;
        
      }
      //hold = 10;
      move_pos2 = (GRect) { .origin = { text_x+push_x, text_y }, .size = { 180, 180 } };
      text_layer_set_text(display_text,*word);
      
      //text_layer_set_text(connection_text,*con_text);
      
      layer_set_frame(text_layer_get_layer(display_text),move_pos2);
  }
}


static void timer_callback(void *data) {
  if(!pause){
  redraw_text(0);
  }else{
    if(fast_rewind){
      redraw_text(1);
    }
  }

  if(con_timer>0){
    con_timer--;
    if(con_timer==49){
      layer_add_child(window_layer, text_layer_get_layer(connection_text));
    }
  }else{
      layer_remove_from_parent(text_layer_get_layer(connection_text));
  }
  
  
  //animation actionEvent
  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}



static void change_to_menu(){
   frame = MENU;
   text_layer_set_text(connection_text,"Waiting for Device..");
   text_layer_set_text(display_text,"");
  
   GRect move_pos2 = (GRect) { .origin = { -15, 105 }, .size = { 180, 180 } };
   layer_set_frame(text_layer_get_layer(display_text),move_pos2);
   GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
   layer_set_frame(text_layer_get_layer(connection_text),move_pos3);
  
   GRect move_pos4 = (GRect) { .origin = {-18, -15 }, .size = { 180, 180 } };
   layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
  
   bitmap_layer_set_bitmap(image_layer, image);
   bitmap_layer_set_alignment(image_layer, GAlignCenter);
  
}

static void change_to_settings(){
   frame = SETTINGS;
   text_layer_set_text(connection_text,"");
   text_layer_set_text(display_text,"FONT");
   GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
   layer_set_frame(text_layer_get_layer(display_text),move_pos2);
   GRect move_pos4 = (GRect) { .origin = { -18, -80 }, .size = { 180, 180 } };
   layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
  
   bitmap_layer_set_compositing_mode(image_layer, GCompOpAssign);
   bitmap_layer_set_bitmap(image_layer, font_banner);
   bitmap_layer_set_alignment(image_layer, GAlignCenter);
  
}

static void change_to_book(){
   frame = BOOK;
   started = true;
    if(font_id==0){
      text_x=-40;
      text_y=70;
    }else if(font_id ==1 ){
      text_x =-40;
      text_y=70;
    }else if(font_id==2){
      text_x =-40;
      text_y = 73;
    }
    con_text = "WPM: 200";  
   //layer_remove_from_parent(text_layer_get_layer(connection_text));
   //text_layer_set_text(connection_text,"");
   text_layer_set_text(display_text,"Starting..");
  
   GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
   layer_set_frame(text_layer_get_layer(display_text),move_pos2);
  /*
   GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
   layer_set_frame(text_layer_get_layer(connection_text),move_pos3);
  */
  // text_layer_set_font(connection_text, disp_font);
  
   GRect move_pos4 = (GRect) { .origin = { -18, 0 }, .size = { 180, 180 } };
   layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
   
   bitmap_layer_set_compositing_mode(image_layer, GCompOpClear);
  
   bitmap_layer_set_bitmap(image_layer, game_bg);
   bitmap_layer_set_alignment(image_layer, GAlignCenter);
  
   timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}


void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(frame == MENU){
    change_to_settings();
  }else
  if(frame == SETTINGS){
    if(!started){
      change_to_menu();
    }else{
      change_to_book();
    }
  }
  else if(frame == BOOK){
    if(pause){
      //redraw_text(2);
    }else{
      if(speed<95){
        speed+=5;
        hard_code();     
        con_timer = 50;
       // APP_LOG(APP_LOG_LEVEL_DEBUG,"speed_text = %s",con_text);
      }
    }
  }
}

void middle_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(frame==MENU){
    change_to_book();
  }
  if(frame == SETTINGS){
      font_id++;
    if(font_id>2){
      font_id = 0;
    }
    if(font_id==0){
      text_x=-20;
      text_y=70;
    }else if(font_id ==1 ){
      text_x =-20;
      text_y=70;
    }else if(font_id==2){
      text_x =-20;
      text_y = 73;
    }
      
      disp_font = fonts_get_system_font(fonts[font_id]);
      GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
      layer_set_frame(text_layer_get_layer(display_text),move_pos2);
      text_layer_set_font(display_text, disp_font);
  }else if(frame == BOOK){
      pause = !pause;
      if(pause == true){
            bitmap_layer_set_bitmap(rewind_layer, pause_icon);
      }else{
             bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
      }
  }
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(frame == MENU){
    
  }else if(frame == BOOK){
    if(pause){
        end = false;
        hold = 0;
        redraw_text(1);
        fast_rewind = true;
       // layer_add_child(window_layer, bitmap_layer_get_layer(rewind_layer));
         bitmap_layer_set_bitmap(rewind_layer, rewind_icon);
        
    }else{
      if(speed>20){
        speed-=5; 
        con_timer = 50;
        hard_code();
        
        //APP_LOG(APP_LOG_LEVEL_DEBUG,"speed_text = %s",*con_text);
      }
    }
  }
}


void down_up_click_handler(ClickRecognizerRef recognizer, void *context) {
    //nothing as of now
    fast_rewind = false;
   // layer_remove_from_parent(bitmap_layer_get_layer(rewind_layer));
  if(pause){
     bitmap_layer_set_bitmap(rewind_layer, pause_icon);
  }
}



void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, middle_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 50, down_click_handler, down_up_click_handler);
}

static void init() {
  window = window_create();
  window_set_fullscreen(window, true);
  window_stack_push(window, true /* Animated */);
  window_set_click_config_provider(window, config_provider);
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  display_text = text_layer_create(bounds);
  connection_text = text_layer_create(bounds);
  image_layer = bitmap_layer_create(bounds);
  rewind_layer = bitmap_layer_create(bounds);
  
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STELA_ICON);
  font_banner = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FONT_BANNER);
  game_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAME_PANE_BLACK);
  rewind_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWIND);
  pause_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  nothing_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOTHING);
  
  disp_font = fonts_get_system_font(fonts[0]);
  text_layer_set_font(display_text, disp_font);
  
  bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
  bitmap_layer_set_alignment(rewind_layer, GAlignCenter);
  GRect move_pos2 = (GRect) { .origin = { -20, -60 }, .size = { 180, 180 } };
  layer_set_frame(bitmap_layer_get_layer(rewind_layer),move_pos2);
  
  change_to_menu();
  

  text_layer_set_text_alignment(display_text, GTextAlignmentCenter);
  text_layer_set_text_alignment(connection_text, GTextAlignmentCenter);
  text_layer_set_overflow_mode(connection_text, GTextOverflowModeWordWrap);
  
  layer_add_child(window_layer, text_layer_get_layer(display_text));
  layer_add_child(window_layer, text_layer_get_layer(connection_text));
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(rewind_layer));
  
  *body_text = " Using Stela, you can read articles from around the web quickly and easily, right on your wrist! Just surf to the article on your phone and hit the Stela button and you can start reading instantly from your Pebble smartwatch. You can read in the shower, and you can even turn your phone off to save that last 5% of your battery. Stela uses a simple, clean, and intuitive interface to show the text one word at a time. Not only does this allow you to read comfortably on such a small screen, but it can even help you to read faster. Words are flashed on the screen one at a time, and they are optically centered on the screen. This allows you to spend less time moving your eyes from word to word (called saccades) and more time reading the content.";
}

static void deinit() {
  gbitmap_destroy(image);
  gbitmap_destroy(game_bg);
 // gbitmap_destory(font_banner);
  bitmap_layer_destroy(image_layer);
  text_layer_destroy(display_text);
  text_layer_destroy(connection_text);
  window_destroy(window);
  
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}