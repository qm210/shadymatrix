#include <stdio.h>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <vector>
#include "LED_WS.h"
#include "pixel.h"
#include "helper.h"

#define PI 3.141592
#define numpix P.size()

int count_LEDs_in_cross_matrix(int, int);
LED shader(float time, vec2 coord, int pixel, int segment);

const float distance_LED_in_cm = 100. / 60.;
const float distance_LED_in_m = 0.01 * distance_LED_in_cm;

bool debug = true;

struct Segment
{
    float origin_x;
    float origin_y;
    float direction; // 0 to 360 degrees
    int pixels;
    Segment(float x, float y, float to_x, float to_y) {set(x, y, to_x, to_y);}
    void set(float x, float y, float to_x, float to_y)
    {
        set_origin(x, y);
        set_direction_to(to_x, to_y);
        set_length_to(to_x, to_y);
    }
    void set_origin(float x, float y) {origin_x = x; origin_y = y;}
    void set_direction(float dir) {direction = fmod(dir + 360., 360.);}
    void set_direction_to(float to_x, float to_y) {set_direction(180./PI*atan2(origin_y - to_y, to_x - origin_x));}
    void set_length(float l) {pixels = (int)round((l + .01) / distance_LED_in_cm);}
    void set_length_to(float to_x, float to_y) {set_length(sqrt(pow(to_x - origin_x, 2) + pow(to_y - origin_y, 2)));}
    float get_length() {return sqrt(pow(get_last_pixel_plus_one().x - origin_x, 2) + pow(get_last_pixel_plus_one().y - origin_y, 2));}
    vec2 get_pixel(int i)
    {
        return vec2(
            origin_x + i * distance_LED_in_cm * cos(direction * PI/180.),
            origin_y - i * distance_LED_in_cm * sin(direction * PI/180.)
        );
    }
    vec2 get_last_pixel_plus_one() {return get_pixel(pixels);}
    float get_rightmost_x() {return direction > 90 && direction < 270 ? origin_x : get_last_pixel_plus_one().x;}
    float get_bottommost_y() {return direction < 180 ? origin_y : get_last_pixel_plus_one().y;}

    void move_horizontally(float inc)
    {
        origin_x += inc;
    }
    void move_vertically(float inc)
    {
        origin_y += inc;
    }
    void turn(float inc)
    {
        set_direction(direction + inc);
    }
    void change_length(int inc)
    {
        pixels = max(pixels + inc, 1);
    }
};

int main(int argc, char* argv[])
{
    SDL_Event e;

    std::vector<Segment> segments;
    std::vector<Pixel> P;

    const float D = distance_LED_in_cm;
    const float N = 40;
    const float A = N * D;
    const float B = A - D;

    float width = 78.5;
    float height = 58.5;
    float margin = 4;

    int selected_segment = 0;

    // DER ECHTE WAL
    segments.clear();
    segments.push_back(Segment(69.80, 52.30, 66.58, 48.48));                             // Segment 0    Length 5.00     Pixels: 3
    segments.push_back(Segment(66.05, 48.65, 54.66, 46.13));                             // Segment 1    Length 11.67    Pixels: 7
    segments.push_back(Segment(54.40, 46.18, 34.40, 46.10));                             // Segment 2    Length 20.00    Pixels: 12
    segments.push_back(Segment(46.85, 50.35, 43.84, 46.35));                             // Segment 3    Length 5.00     Pixels: 3
    segments.push_back(Segment(45.62, 53.47, 47.91, 51.05));                             // Segment 4    Length 3.33     Pixels: 2
    segments.push_back(Segment(36.92, 53.67, 45.25, 54.04));                             // Segment 5    Length 8.33     Pixels: 5
    segments.push_back(Segment(30.27, 48.35, 36.25, 54.15));                             // Segment 6    Length 8.33     Pixels: 5
    segments.push_back(Segment(34.23, 46.10, 20.99, 47.69));                             // Segment 7    Length 13.33    Pixels: 8
    segments.push_back(Segment(20.77, 47.67, 10.87, 46.29));                             // Segment 8    Length 10.00    Pixels: 6
    segments.push_back(Segment(12.77, 45.05, 33.93, 40.38));                             // Segment 9    Length 21.67    Pixels: 13
    segments.push_back(Segment(33.93, 40.10, 43.22, 36.41));                             // Segment 10   Length 10.00    Pixels: 6
    segments.push_back(Segment(43.70, 37.10, 49.48, 40.42));                             // Segment 11   Length 6.67     Pixels: 4
    segments.push_back(Segment(50.05, 39.63, 53.14, 38.37));                             // Segment 12   Length 3.33     Pixels: 2
    segments.push_back(Segment(52.17, 37.23, 51.28, 34.02));                             // Segment 13   Length 3.33     Pixels: 2
    segments.push_back(Segment(50.50, 34.05, 46.09, 31.69));                             // Segment 14   Length 5.00     Pixels: 3
    segments.push_back(Segment(46.40, 30.92, 42.86, 36.57));                             // Segment 15   Length 6.67     Pixels: 4
    segments.push_back(Segment(32.78, 22.55, 27.95, 21.27));                             // Segment 16   Length 5.00     Pixels: 3
    segments.push_back(Segment(27.68, 21.95, 24.80, 23.62));                             // Segment 17   Length 3.33     Pixels: 2
    segments.push_back(Segment(18.83, 24.13, 14.03, 22.75));                             // Segment 18   Length 5.00     Pixels: 3
    segments.push_back(Segment(13.78, 23.42, 10.98, 25.23));                             // Segment 19   Length 3.33     Pixels: 2
    segments.push_back(Segment(10.58, 45.58, 4.84, 39.54));                              // Segment 20   Length 8.33     Pixels: 5
    segments.push_back(Segment(5.30, 39.15, 4.59, 29.17));                               // Segment 21   Length 10.00    Pixels: 6
    segments.push_back(Segment(5.02, 29.03, 11.18, 17.21));                              // Segment 22   Length 13.33    Pixels: 8
    segments.push_back(Segment(11.65, 17.50, 16.91, 13.41));                             // Segment 23   Length 6.67     Pixels: 4
    segments.push_back(Segment(14.74, 12.77, 9.87, 8.21));                               // Segment 24   Length 6.67     Pixels: 4
    segments.push_back(Segment(9.42, 8.92, 4.64, 10.38));                                // Segment 25   Length 5.00     Pixels: 3
    segments.push_back(Segment(8.32, 4.97, 12.84, 7.11));                                // Segment 26   Length 5.00     Pixels: 3
    segments.push_back(Segment(12.87, 7.75, 16.90, 13.06));                              // Segment 27   Length 6.67     Pixels: 4
    segments.push_back(Segment(17.17, 11.78, 15.45, 5.34));                              // Segment 28   Length 6.67     Pixels: 4
    segments.push_back(Segment(16.97, 5.22, 20.17, 4.30));                               // Segment 29   Length 3.33     Pixels: 2
    segments.push_back(Segment(17.67, 13.30, 37.61, 11.77));                             // Segment 30   Length 20.00    Pixels: 12
    segments.push_back(Segment(34.72, 9.92, 35.78, 6.76));                               // Segment 31   Length 3.33     Pixels: 2
    segments.push_back(Segment(36.30, 7.00, 49.51, 5.22));                               // Segment 32   Length 13.33    Pixels: 8
    segments.push_back(Segment(49.33, 6.43, 50.48, 9.56));                               // Segment 33   Length 3.33     Pixels: 2
    segments.push_back(Segment(49.83, 9.75, 47.79, 16.10));                              // Segment 34   Length 6.67     Pixels: 4
    segments.push_back(Segment(37.38, 12.08, 54.47, 18.71));                             // Segment 35   Length 18.33    Pixels: 11
    segments.push_back(Segment(54.45, 19.48, 61.37, 26.70));                             // Segment 36   Length 10.00    Pixels: 6
    segments.push_back(Segment(61.33, 27.25, 63.82, 38.65));                             // Segment 37   Length 11.67    Pixels: 7
    segments.push_back(Segment(64.92, 38.43, 68.25, 38.53));                             // Segment 38   Length 3.33     Pixels: 2
    segments.push_back(Segment(67.92, 37.23, 69.77, 29.10));                             // Segment 39   Length 8.33     Pixels: 5
    segments.push_back(Segment(71.07, 30.38, 73.91, 32.12));                             // Segment 40   Length 3.33     Pixels: 2
    segments.push_back(Segment(73.33, 32.67, 71.77, 42.55));                             // Segment 41   Length 10.00    Pixels: 6
    segments.push_back(Segment(70.85, 42.32, 67.67, 43.31));                             // Segment 42   Length 3.33     Pixels: 2
    segments.push_back(Segment(69.30, 44.18, 74.18, 45.25));                             // Segment 43   Length 5.00     Pixels: 3
    segments.push_back(Segment(73.83, 46.25, 73.80, 51.25));                             // Segment 44   Length 5.00     Pixels: 3
    segments.push_back(Segment(73.08, 51.20, 70.02, 52.51));                             // Segment 45   Length 3.33     Pixels: 2
    segments.push_back(Segment(9.42, 8.83, 9.42, 8.83));                                 // Segment 46   Length 0.00     Pixels: 0

    // scale the shit cause matze made some stupid retardedness
    /*
    const float scale = .88;
    const float x_shift = 1.3;
    const float y_shift = 0.7;
    for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
    {
        float scaled_x0 = scale * iseg->origin_x + x_shift;
        float scaled_y0 = scale * iseg->origin_y + y_shift;
        float scaled_x1 = scale * iseg->wanted_to_x + x_shift;
        float scaled_y1 = scale * iseg->wanted_to_y + y_shift;
        (*iseg) = Segment(scaled_x0, scaled_y0, scaled_x1, scaled_y1);
    }
    // */

    float meters_required = numpix * distance_LED_in_m;
    printf("Pixels: %i\nMeters: %g\nWidth: %g\nHeight: %g\n", numpix, meters_required, width, height);

    const float SCALE = N * .3;
    const float MARGIN = margin * SCALE;
    const float WIDTH = width * SCALE;
    const float HEIGHT = height * SCALE;
    const float LEDSIZE = 5;

    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf ("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2_gfx test", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        printf ("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf ("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 3;
    }

    SDL_Rect margin_t, margin_b, margin_l, margin_r, margin_extra1, margin_extra2;
    margin_t.x = 0;
    margin_t.y = 0;
    margin_b.x = 0;
    margin_b.y = HEIGHT - MARGIN;
    margin_l.x = 0;
    margin_l.y = 0;
    margin_r.x = WIDTH - MARGIN;
    margin_r.y = 0;
    margin_t.w = margin_b.w = WIDTH;
    margin_t.h = margin_b.h = MARGIN;
    margin_l.w = margin_r.w = MARGIN;
    margin_l.h = margin_r.h = HEIGHT;
    margin_extra1.w = margin_extra2.w = 8.2 * SCALE;
    margin_extra1.h = margin_extra2.h = 5.5 * SCALE;
    margin_extra1.x = margin_extra2.x = WIDTH - margin_extra1.w;
    margin_extra1.y = 0;
    margin_extra2.y = HEIGHT - margin_extra2.h;

    float new_origin_x, new_origin_y, new_to_x, new_to_y;

    int quit = 0;
    long time = 0;
    bool firstClick = true;
    while (!quit)
    {
        if (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (firstClick)
                    {
                        new_origin_x = x / SCALE;
                        new_origin_y = y / SCALE;
                        printf("(%g, %g, ...\n", new_origin_x, new_origin_y);
                        firstClick = false;
                    }
                    else
                    {
                        new_to_x = x / SCALE;
                        new_to_y = y / SCALE;
                        Segment new_seg = Segment(new_origin_x, new_origin_y, new_to_x, new_to_y);
                        segments.push_back(new_seg);
                        for (int p = 0; p < new_seg.pixels; p++)
                        {
                            P.push_back(Pixel(new_seg.get_pixel(p), segments.size() - 1));
                        }
                        meters_required = numpix * distance_LED_in_m;
                        printf("(%g, %g, %g, %g) Pixels: %i, Meters: %g\n", new_origin_x, new_origin_y, new_to_x, new_to_y, numpix, meters_required);
                        firstClick = true;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_BACKSPACE:
                        {
                            Segment last_seg = segments.back();
                            segments.pop_back();
                            for (int p = 0; p < last_seg.pixels; p++)
                            {
                                P.pop_back();
                            }
                            break;
                        }
                        case SDLK_SPACE:
                            segments.clear();
                            P.clear();
                            printf("erased everything.\n");
                            break;
                        /*
                        case SDLK_LEFT:
                            P.pop_back();
                            break;
                        */

                        case SDLK_w:
                            segments[selected_segment].move_vertically(-0.1);
                            break;
                        case SDLK_s:
                            segments[selected_segment].move_vertically(+0.1);
                            break;
                        case SDLK_d:
                            segments[selected_segment].move_horizontally(+0.1);
                            break;
                        case SDLK_a:
                            segments[selected_segment].move_horizontally(-0.1);
                            break;
                        case SDLK_e:
                            segments[selected_segment].turn(-1);
                            break;
                        case SDLK_q:
                            segments[selected_segment].turn(+1);
                            break;
                        case SDLK_r:
                            segments[selected_segment].change_length(1);
                            break;
                        case SDLK_f:
                            segments[selected_segment].change_length(-1);
                            break;

                        case SDLK_UP:
                            selected_segment = (selected_segment + 1) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x, segments[selected_segment].origin_y,
                                                                segments[selected_segment].get_last_pixel_plus_one().x, segments[selected_segment].get_last_pixel_plus_one().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x, height - segments[selected_segment].get_last_pixel_plus_one().y);
                            break;
                        case SDLK_DOWN:
                            selected_segment = (selected_segment - 1 + segments.size()) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x, segments[selected_segment].origin_y,
                                                                segments[selected_segment].get_last_pixel_plus_one().x, segments[selected_segment].get_last_pixel_plus_one().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x, height - segments[selected_segment].get_last_pixel_plus_one().y);
                            break;

                        case SDLK_ESCAPE:
                            quit = 1;
                            break;

                        case SDLK_RETURN:
                            printf("\n");
                            printf("    segments.clear();\n");
                            for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
                            {
                                printf("    segments.push_back(Segment(%.2f, %.2f, %.2f, %.2f)); \t\t\t\t // Segment %i \t Length %.2f \t Pixels: %g \n",
                                    iseg->origin_x, iseg->origin_y, iseg->get_last_pixel_plus_one().x, iseg->get_last_pixel_plus_one().y,
                                    (iseg - segments.begin()), iseg->get_length(), iseg->get_length()/distance_LED_in_cm);
                            }
                            printf("\n");
                            break;
                    }
                    break;

                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        //////////// MARGIN /////////////
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderFillRect(renderer, &margin_t);
        SDL_RenderFillRect(renderer, &margin_b);
        SDL_RenderFillRect(renderer, &margin_l);
        SDL_RenderFillRect(renderer, &margin_r);
        SDL_RenderFillRect(renderer, &margin_extra1);
        SDL_RenderFillRect(renderer, &margin_extra2);

        //////////// PATTERN ////////////
        P.clear();
        for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
        {
            int segcount = iseg - segments.begin();
            for (int p = 0; p < iseg->pixels; p++)
            {
                Pixel pixel = Pixel(iseg->get_pixel(p), segcount);
                vec2 relative_coord = vec2(pixel.x/width, pixel.y/height);
                pixel.L = shader(time, relative_coord, numpix, segcount - selected_segment);
                P.push_back(pixel);
            }
        }

        //////////// RENDER ////////////
        for(int p = 0; p < numpix; p++)
        {
            if(numpix < 1000 && !debug)
            {
                filledCircleColor(renderer, SCALE * P[p].x, SCALE * P[p].y, LEDSIZE * 2.5, LEDColor(P[p].L, 0.125));
                filledCircleColor(renderer, SCALE * P[p].x, SCALE * P[p].y, LEDSIZE * 1.5, LEDColor(P[p].L, 0.25));
            }
            filledCircleColor(renderer, SCALE * P[p].x, SCALE * P[p].y, LEDSIZE, LEDColor(P[p].L));
        }

        SDL_RenderPresent(renderer);
//        SDL_Delay(5);
        time++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

float pos0 = 0;
float pos1 = 0;
float pos2 = 0;
float pos3 = 0;
float vel0 = 0;
float vel1 = 0;
float ang0 = 0;
float ang1 = 0;


LED shader(float time, vec2 coord, int pixel, int segment)
{
  bool wal = true;

  float mod_time = fmod(time, 100.);
  if(mod_time < .01)
  {
    pos0 = pseudorandom(time);
    pos1 = pseudorandom(time + 1.);
    ang0 = 2.*PI*pseudorandom(2.*time);
  }
  float edge1 = .01 * mod_time;
  ang0 += 0.0033 * mod_time * 1e-4;
  float x = coord.x - pos0;
  float y = coord.y - pos1;
  float xx =  cos(ang0) * x + sin(ang0) * y;
  float yy = -sin(ang0) * x + cos(ang0) * y;
  float intensity = exp(-15. * fabs( min(edge1-fabs(xx), edge1-fabs(yy)) )) * (smoothstep(0,10,mod_time) - smoothstep(80, 100, mod_time));
  LED cube1 = wal ? LED(250, 0, .6 * intensity) : LED(266, .3, intensity);

  float mod_time2 = fmod(time + 83., 167.);
  if(mod_time2 < .02)
  {
    pos2 = pseudorandom(7.*time);
    pos3 = pseudorandom(7.*time + 1.);
    ang1 = 2.*PI*pseudorandom(3.*time);
  }
  float edge2 = .004 * (mod_time2);
  ang1 -= 0.0021 * mod_time2 * 1e-4;
  x =  cos(ang1) * coord.x + sin(ang1) * coord.y - pos2;
  y = -sin(ang1) * coord.x + cos(ang1) * coord.y - pos3;
  intensity = exp(-12.4 * fabs( min(edge2-fabs(x), edge2-fabs(y)) )) * (smoothstep(0,10,mod_time) - smoothstep(140, 167, mod_time2));
  LED cube2 = wal ? LED(166, .2, intensity) : LED(111, 0, intensity);

  cube1.mix_shitty(cube2, 1.);
  return cube1;
}
    /*
    bool wal = true;

    if(debug)
    {
        float hue = fmod(35. * segment, 360.);
        float white = 0;
        float lumi = 0.6;
        if(segment == 0)
        {
            white = 1;
            lumi = 1;
        }
        return LED(hue, white, lumi);
    }

    if(false)
    {
        float hue_cyan = 180.;
        float hue_magenta = 300.;
        float hue_avg = .5 * (hue_cyan + hue_magenta);
        float hue_spread = hue_magenta - hue_avg;

        LED base = LED(hue_cyan, 0, 1);

        float amount = sin(1.4 * coord.x + 4.7 * coord.y + .06 * time);
        LED add = LED(hue_magenta, (1-amount), amount);

        LED mix = (1-amount) * base + amount * add;

        float mod_time = fmod(time, 240.);
        float yellow_mixing = smoothstep(160., 200., mod_time) * (1. - smoothstep(200., 240., mod_time));
        mix = (1-yellow_mixing) * mix + yellow_mixing * LED(90, .6, 1);

        return mix;
    }
    else if(false)
    {
        LED led_bg = wal ? LED(250, 0, .25 + .25 * sin(0.001*time)) : LED(330, 0, .6);
        LED led_shine = wal ? LED(210, .4, 1.) : LED(90, .3, 1.);

        if(time < 0.01)
        {
            pos0 = pos1 = 0.5;
            vel0 = 3e-5; //.00010;
            vel1 = 1e-5; //.00003;
        }
        if(pos0 > 1 || pos0 < 0) vel0 *= -1;
        if(pos1 > 1 || pos1 < 0) vel1 *= -1;

        pos0 += vel0;
        pos1 += vel1;

        float distance = coord.get_distance_to(vec2(pos0, pos1));
        float shine = exp(-pow(distance*4.,2.));
        return shine * led_shine + (1-shine) * led_bg;
    }
    else if (false){
        float mod_pos = fmod(pixel + 0.6*time, 80);
        float intensity = exp(-.07 * mod_pos) * (mod_pos < 0 ? 0. : 1.);
        return LED(150, .3, intensity);
    }
    else if (true) {
        float mod_time = fmod(time, 100.);
        if(mod_time < .01)
        {
            pos0 = pseudorandom(time);
            pos1 = pseudorandom(time + 1.);
            ang0 = 2.*PI*pseudorandom(2.*time);
        }
        float edge1 = .01 * mod_time;
        ang0 += 0.0033 * mod_time * 1e-4;
        float x = coord.x - pos0;
        float y = coord.y - pos1;
        float xx =  cos(ang0) * x + sin(ang0) * y;
        float yy = -sin(ang0) * x + cos(ang0) * y;
        float intensity = exp(-15. * fabs( min(edge1-fabs(xx), edge1-fabs(yy)) )) * (1 - smoothstep(80, 100, mod_time));
        LED cube1 = wal ? LED(250, 0, .6 * intensity) : LED(266, .3, intensity);

        float mod_time2 = fmod(time + 83., 167.);
        if(mod_time2 < .02)
        {
            pos2 = pseudorandom(7.*time);
            pos3 = pseudorandom(7.*time + 1.);
            ang1 = 2.*PI*pseudorandom(3.*time);
        }
        float edge2 = .004 * (mod_time2);
        ang1 -= 0.0021 * mod_time2 * 1e-4;
        x =  cos(ang1) * coord.x + sin(ang1) * coord.y - pos2;
        y = -sin(ang1) * coord.x + cos(ang1) * coord.y - pos3;
        intensity = exp(-12.4 * fabs( min(edge2-fabs(x), edge2-fabs(y)) )) * (1 - smoothstep(140, 167, mod_time2));
        LED cube2 = wal ? LED(166, .2, intensity) : LED(111, 0, intensity);

        cube1.mix_shitty(cube2, 1.);

        return cube1;
    }
    else if (false) {
        float repeat = 200.;
        float mod_time = fmod(time, repeat);
        float div_time = floor(time/repeat)*repeat;

        LED cube;
        int n = 4;
        for(int c=0; c < n; c++)
        {
            if(c>0)
            {
                mod_time -= repeat/n;
                div_time += (float)c/n;
            }
            ang0 = pseudorandom(2.*div_time) * (2.*PI* + 0.033 * mod_time);
            float edge = .01 * (mod_time - c*50.);

            float center_x = 0.4 + 0.2 * pseudorandom(div_time);
            float center_y = 0.4 + 0.2 * pseudorandom(div_time * 7.);
            float x = coord.x - center_x;
            float y = coord.y - center_y;
            float xx =  cos(ang0) * x + sin(ang0) * y;
            float yy = -sin(ang0) * x + cos(ang0) * y;
            float intensity = exp(-15. * fabs( min(edge-fabs(xx), edge-fabs(yy)) )) * (1 - smoothstep(.8*repeat, repeat, mod_time));
            if (c==0)
            {
                LED cube = LED(266, .3, intensity);
            }
            else
            {
                cube.mix_shitty(LED(111, 0, intensity), 1.);
            }
        }

        return cube;
    }
}
*/
