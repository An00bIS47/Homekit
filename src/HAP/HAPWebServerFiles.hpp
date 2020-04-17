//
// HAPWebServerFiles.hpp
// Homekit
//
//  Created on: 12.1.2020
//      Author: michael
//
#ifndef HAPWEBSERVERFILES_HPP_
#define HAPWEBSERVERFILES_HPP_

#include <Arduino.h>


#ifdef PLATFORMIO
// index.html
extern const uint8_t html_template_index_start[] 		asm("_binary_www_index_html_start");
extern const uint8_t html_template_index_end[] 			asm("_binary_www_index_html_end");


// vanilla_qr
// extern const uint8_t html_template_vanilla_qr_start[] 	asm("_binary_vanilla_qr_js_start");
// extern const uint8_t html_template_vanilla_qr_end[] 	asm("_binary_vanilla_qr_js_end");


// qr font
extern const uint8_t html_template_qrcode_font_start[]  asm("_binary_www_qrcode_font_css_start");
extern const uint8_t html_template_qrcode_font_end[]    asm("_binary_www_qrcode_font_css_end");

// qr code container svg
extern const uint8_t html_template_qrcode_container_start[] asm("_binary_www_qrcode_container_svg_start");
extern const uint8_t html_template_qrcode_container_end[]   asm("_binary_www_qrcode_container_svg_end");

#else

// index.html
extern const uint8_t html_template_index_start[] 		asm("_binary_index_html_start");
extern const uint8_t html_template_index_end[] 			asm("_binary_index_html_end");


// vanilla_qr
// extern const uint8_t html_template_vanilla_qr_start[] 	asm("_binary_vanilla_qr_js_start");
// extern const uint8_t html_template_vanilla_qr_end[] 	asm("_binary_vanilla_qr_js_end");


// qr font
extern const uint8_t html_template_qrcode_font_start[]  asm("_binary_qrcode_font_css_start");
extern const uint8_t html_template_qrcode_font_end[]    asm("_binary_qrcode_font_css_end");

// qr code container svg
extern const uint8_t html_template_qrcode_container_start[] asm("_binary_qrcode_container_svg_start");
extern const uint8_t html_template_qrcode_container_end[]   asm("_binary_qrcode_container_svg_end");


#endif

#endif /* HAPWEBSERVERFILES_HPP_ */