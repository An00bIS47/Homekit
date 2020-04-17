//
// HAPSVG.hpp
// Homekit
//
//  Generated on: 23.09.2019
//

#include "HAPSVG.hpp"

#define QR_CODE_X_OFFSET    0
#define QR_CODE_Y_OFFSET    0
#define QR_CODE_ZOOM_FACTOR 5


void HAPSVG::init(Print* prt, QRCode* qrcode){
    int width = (((4 * qrcode->version) + 17) * QR_CODE_ZOOM_FACTOR ) + QR_CODE_X_OFFSET + QR_CODE_Y_OFFSET;
    int height = width;

    // Serial.println(String(width) + "x" + String(height));

    String svg = "";
    // appendstringtosvg(_svg, "<svg width='");
    svg += "<svg width='";    
    
    // appendnumbertosvg(_svg, width);
    svg += String(width);

    // appendstringtosvg(_svg, "px' height='");
    svg += "px' height='";

    // appendnumbertosvg(_svg, height);
    svg += String(height);

    // appendstringtosvg(_svg, "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n");
    svg += "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>";

    prt->print(svg);
}


void HAPSVG::finalize(Print* prt)
{
    // appendstringtosvg(_svg, "</svg>");
    String svg = "</svg>";
    // _finalized = true;

    prt->print(svg);
}



void HAPSVG::rectangle(Print* prt, int width, int height, int x, int y, const char* fill, const char* stroke, int strokeWidth, int radiusx, int radiusy)
{   
    String svg = "";
    

    svg += "<rect width='";
    svg += String(width);
    
    svg += "' height='";
    svg += String(height);
    

    svg += "' y='";    
    svg += String(y);
    
    
    svg += "' x='";
    svg += String(x);
    
    // appendstringtosvg(_svg, "' ry='");
    svg += "' ry='";
    
    // appendnumbertosvg(_svg, radiusy);
    svg += String(radiusy);
    
    // appendstringtosvg(_svg, "' rx='");
    svg += "' rx='";
    
    // appendnumbertosvg(_svg, radiusx);
    svg += String(radiusx);

    if (strcmp(fill, "#000") != 0 ) {
        svg += "' fill='";        
        svg += String(fill);
    }

    if (strokeWidth > 0) {
        svg += "' stroke='";
        svg += String(stroke);

        svg += "' stroke-width='";
        svg += String(strokeWidth);
    }

    
    // appendstringtosvg(_svg, "' />\n");
    svg += "' />";

    prt->print(svg);    
}

void HAPSVG::drawQRCode(Print* prt, QRCode* qrcode, const char* colorFill, const char* colorBackground){


    init(prt, qrcode);

    int k = 0;         
    for (uint8_t y = 0; y < qrcode->size; y++) {
        int n = 0;
        for (uint8_t x = 0; x < qrcode->size; x++) {

            int cx = x + QR_CODE_X_OFFSET + (n * QR_CODE_ZOOM_FACTOR) - n;
            int cy = y + QR_CODE_Y_OFFSET + (k * QR_CODE_ZOOM_FACTOR) - k;

            if (qrcode_getModule(qrcode, x, y) ) {  
                // draw black rect                
                rectangle(prt, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR, cx, cy, colorFill, "", 0 , 0, 0);	
            } else {
                // draw white rect
                // rectangle(prt, QR_CODE_ZOOM_FACTOR, QR_CODE_ZOOM_FACTOR, cx, cy, colorBackground, "", 0, 0, 0);	
            }
            n++;
        }            
        k++;            
    }

    finalize(prt);
}