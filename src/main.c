#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"
	
#define COUNT_IDX 0
#define ISS_COOKIE 155

PBL_APP_INFO_SIMPLE(HTTP_UUID, "eBay Deals", "eBay", 1 /* App version */);


Window window;

TextLayer textLayer;
int error = 0;
int cnt = 0;
int i = 0;
int currId = 0;

static char message[100];
static char err[20];
static char durl[200] = "";
static char purl[200] = "http://scanchihackmit.herokuapp.com/?dealNumber=%d";

void start_http_request(int id) {
  DictionaryIterator *out;
  
  snprintf(durl, sizeof(durl), purl, id);
  HTTPResult	result = http_out_get(durl, ISS_COOKIE, &out);
	
  if (result != HTTP_OK) {
    error = result;
    return;
  }
  result = http_out_send();
  if (result != HTTP_OK) {
    error = result;
    return;
  }
}

// Called when the http request is successful. Updates the nextpass_time.
void handle_http_success(int32_t request_id, int http_status, DictionaryIterator* sent, void* context) {
  Tuple *nextpass_tuple = dict_find(sent, COUNT_IDX);
  if (nextpass_tuple) {
	cnt = (int)nextpass_tuple->value->uint32;
	  
	nextpass_tuple = dict_find(sent, 2);
	memcpy(message, nextpass_tuple->value->cstring, nextpass_tuple->length);
	  
	//memcpy(message, nextpass_tuple->value->cstring, nextpass_tuple->length);  
    //snprintf(message, sizeof(message), nextpass_tuple->value->cstring);
	//text_layer_set_text(&textLayer, "Success!");
	//snprintf(message, sizeof(message), "%d", (int)nextpass_tuple->value->uint32);
    text_layer_set_text(&textLayer, message);
  } else {
  	  text_layer_set_text(&textLayer, "Error!");
  }
  error = 0;
}

// Called when the http request fails. Updates the error variable.
void handle_http_failure(int32_t request_id, int http_status, void* context) {
  error = http_status;
  snprintf(err, sizeof(err), "Error: %d", http_status);
  text_layer_set_text(&textLayer, err);
}

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	currId = currId - 1;
	if (currId < 0) {
		currId = cnt - 1;
	}
	if (currId == 0) currId = 6;
	start_http_request(currId);
	text_layer_set_text(&textLayer, message);
}

void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	currId = (currId + 1) % cnt;
	if (currId == 0) currId = 1;
	start_http_request(currId);
	text_layer_set_text(&textLayer, message);
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	text_layer_set_text(&textLayer, "Connecting...");
	currId=1;
	start_http_request(1);
}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {

}


// This usually won't need to be modified

void click_config_provider(ClickConfig **config, Window *window) {

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}


// Standard app initialisation

void handle_init(AppContextRef ctx) {

  window_init(&window, "eBay Deals");
  window_stack_push(&window, true /* Animated */);

  text_layer_init(&textLayer, window.layer.frame);
  text_layer_set_text(&textLayer, "eBay Deals");
  text_layer_set_font(&textLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&window.layer, &textLayer.layer);

  // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 124,
      }
    }
  };
  HTTPCallbacks http_callbacks = {
    .failure = handle_http_failure,
    .success = handle_http_success
  };
  http_register_callbacks(http_callbacks, NULL);
  app_event_loop(params, &handlers);
}