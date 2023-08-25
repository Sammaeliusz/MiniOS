#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>
const int WIDTH=800, HEIGHT=600, ISIZE = 50, IWIDTH = WIDTH/ISIZE, IHEIGHT = HEIGHT/ISIZE, FONTSIZE = 24, APPNUM = IWIDTH*IHEIGHT;
//new icon width/heigh
//iterators
int i=0, j=0;
SDL_Window *window = nullptr;
SDL_Event windowEvent;
SDL_Renderer *renderer = nullptr;
std::vector<SDL_Rect> grid;
std::vector<SDL_Color> colorAtlas;
std::vector<TTF_Font*> fontAtlas;
struct button{
    int id;
    SDL_Rect place;
    void (*funptr)(int);
    std::string text_on = "", img_link = "";
};
struct app{
    std::string name;
    std::string icon = "img/.png";
    std::vector<button> buttonList;
    std::vector<SDL_Rect> rectObjects;
    int lastPressBtn = -1;
    bool appGenerated = false;
};
struct vector2{
    int x;
    int y;
};
app apps[APPNUM] = {};
//initialisation and deinitialisation
void addColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a){
    SDL_Color kolor = {r, g, b, a};
    colorAtlas.emplace_back(kolor);
}
void initcolorAtlas(){
    addColor(255, 255, 255, 255);    //white 0
    addColor(0, 0, 0, 255);          //black 1
    addColor(128, 128, 128, 255);    //gray 2
    addColor(255, 0, 0, 255);        //red 3
    addColor(0, 255, 0, 255);        //green 4
    addColor(0, 0, 255, 255);        //blue 5
    addColor(255, 255, 0, 255);      //yellow 6
    addColor(255, 0, 255, 255);      //purple 7
    addColor(0, 255, 255, 255);      //cyjan 8
    addColor(62, 180, 137, 255);     //mint 9
    addColor(173, 216, 250, 255);    //light_blue 10
}
void addFont(std::string path){
    int tmp = fontAtlas.size();
    fontAtlas.emplace_back(TTF_OpenFont(path.c_str(), FONTSIZE));
    if ( !fontAtlas.size() == tmp ) {
    	std::cout << "Failed to load font: " << TTF_GetError() << "\n";
    }
}
void initFontAtlas(){
    addFont("fonts/CONSOLA.TTF");
    addFont("fonts/GreatVibes-Wmr4.ttf");
}
int init(){
    SDL_Init( SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    SDL_CreateWindowAndRenderer(WIDTH+2*ISIZE, HEIGHT+2*ISIZE, 0, &window, &renderer);
    if (NULL == window){
        std::cout << "Window is not corectly created\n";
        std::cout << SDL_GetError();
        return 1;
    }
    if (NULL == renderer){
        std::cout<< "Renderer is not corectly created\n";
        std::cout<< SDL_GetError();
        return 1;
    }
    SDL_SetWindowTitle(window, "Mini Operating System");
    initcolorAtlas();
    if ( TTF_Init() < 0 )
	    std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << "\n";
    initFontAtlas();
}
int freeMem(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    std::for_each(fontAtlas.crbegin(), fontAtlas.crend(), TTF_CloseFont);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
//rozne dziwne funkcje
void setRendererColor(SDL_Renderer *renderer, int kolor){
    SDL_SetRenderDrawColor(renderer, colorAtlas.at(kolor).r, colorAtlas.at(kolor).g, colorAtlas.at(kolor).b, colorAtlas.at(kolor).a);
}
SDL_Texture *loadImage(SDL_Renderer *renderer, std::string path){
    SDL_Surface *icon_surf = IMG_Load(path.c_str());
    SDL_Texture *out = SDL_CreateTextureFromSurface(renderer, icon_surf);
    SDL_FreeSurface(icon_surf);
    if(out != NULL)
	    return out;
    else
        loadImage(renderer, path);
}
void genText(SDL_Renderer *renderer, int color, std::string text_in, vector2 place, int font = 0){
    SDL_Texture* text_texture;
    SDL_Surface* text;
    text = TTF_RenderText_Solid(fontAtlas.at(font), text_in.c_str(), colorAtlas.at(color) );
    if ( !text ) {
	std::cout << "Failed to render text: " << TTF_GetError() << "\n";
    }
    text_texture = SDL_CreateTextureFromSurface( renderer, text );
    SDL_Rect dest = { place.x, place.y, text->w, text->h };
    SDL_RenderCopy( renderer, text_texture, NULL, &dest);
    SDL_DestroyTexture( text_texture );
    SDL_FreeSurface( text );
}
std::vector<std::string> readText(std::string path){
    std::ifstream plik;
    std::vector<std::string> out;
    std::string tmp;
    plik.open(path);
    while(getline(plik, tmp)){
        //std::cout << tmp << "\n";
        out.emplace_back(tmp);
    }
    plik.close();
    return out;
}
void genTextFile(SDL_Renderer *renderer, std::string path, int x, int y, int font =0){
    std::vector<std::string> text;
    if(text.size()==0)
        text = readText(path);
    for(i=0; i<text.size(); i+=2){
        //std::cout<< *text.at(i).data() << "\n";
        genText(renderer, std::stoi(text.at(i)), text.at(i+1), vector2{x, y+FONTSIZE*i}, font);
    }
}
button whichBtn(int appn){
    int x=0, y=0;
    int state = SDL_GetMouseState(&x, &y);
    SDL_Rect tt = {0, 0, 0, 0};
    button t = {-1, tt};
    int s = apps[appn].buttonList.size();
    for(j=0; j<s; j++){
        button currentBtn = apps[appn].buttonList.at(j);
        if (state==1 && x>currentBtn.place.x && y>currentBtn.place.y && y<currentBtn.place.y+currentBtn.place.h && x<currentBtn.place.x+currentBtn.place.w)
            return currentBtn;
    }
    return t;
}
bool lastBtn(int appn){
    button pre_btn = whichBtn(appn);
    if(pre_btn.id != -1 && pre_btn.id != apps[appn].lastPressBtn){
        apps[appn].lastPressBtn = pre_btn.id;
        return true;
    }
    if(pre_btn.id == -1)
        apps[appn].lastPressBtn = -1;
    return false;
}
vector2 rectCenter(SDL_Rect inRect){
    vector2 out;
    out.x = inRect.x+(inRect.w/2)-8;
    out.y = inRect.y+(inRect.h/2)-8;
    return out;
}
//main menu functions
void genGrid(std::vector<SDL_Rect> *grid, int iheight, int iwidth, int iconSize, int wshift = 0, int hshift =0){
    for(i=0; i<iheight; ++i){
        for(j=0; j<iwidth; ++j){
            SDL_Rect rect = {((j+1)*iconSize)+wshift, ((i+1)*iconSize)+hshift, iconSize, iconSize};
            (*grid).emplace_back(rect);
        }
    }
}
void drawGrid(SDL_Renderer *renderer, std::vector<SDL_Rect> grid, int iheight, int iwidth){
    for(i=0; i<iwidth*iheight; ++i){
        SDL_RenderDrawRect(renderer, &grid.at(i));
    }
}
void newIcon(SDL_Renderer *renderer, app ikon, int id){
    SDL_Texture *icon_texture = loadImage(renderer, ikon.icon);
    if(icon_texture != NULL)
	    SDL_RenderCopy(renderer, icon_texture, NULL, &grid.at(id));
    else
        newIcon(renderer, ikon, id);
}
int whichIcon(int iconSize, int height, int width){
    int x=0, y=0;
    int state = SDL_GetMouseState(&x, &y);
    if (state==1 && x>iconSize && y>iconSize && y<height+iconSize && x<width+iconSize)
        return ((y-iconSize)/iconSize)*IWIDTH + (x-iconSize)/iconSize;
    else
        return -1;
}
void iconGenerate(){
    
}
//apps functions
unsigned long long factorial(short int number){
    if(number==0)
        return 1;
    else
        return number * factorial(number-1);
}
double bernoulli(unsigned short int k, unsigned short int n, float p, float q){
    return pow(p, k)*pow(q, n-k)*(factorial(n)/(factorial(k)*factorial(n-k)));
}
int dice(unsigned short int diceWalls){
    return rand()%diceWalls+1;
}
bool dice_success(unsigned short int diceWalls, unsigned short int minWin){
    int rzut = dice(diceWalls);
    if (rzut >= minWin)
        return true;
    else
        return false;
}
short int dice_success(unsigned short int diceWalls, unsigned short int minWin, unsigned short int maxFail){
    int rzut = dice(diceWalls);
    if (rzut >= minWin)
        return 1;
    else if (rzut<=maxFail)
        return -1;
    else
        return 0;
}
//apps
void app0(){
    SDL_Rect test = {50, 50, WIDTH, HEIGHT};
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    setRendererColor(renderer, 2);
    //SDL_RenderDrawRect(renderer, &test);
    SDL_RenderFillRect(renderer, &test);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void app1_gen(){
    genGrid(&apps[1].rectObjects, 4, 4, 100, 150, 100);
    button t = {0, apps[1].rectObjects.at(0), nullptr, "7"};
    apps[1].buttonList.emplace_back(t);
    t = {1, apps[1].rectObjects.at(1), nullptr, "8"};
    apps[1].buttonList.emplace_back(t);
    t = {2, apps[1].rectObjects.at(2), nullptr, "9"};
    apps[1].buttonList.emplace_back(t);
    t = {3, apps[1].rectObjects.at(3), nullptr, "+"};
    apps[1].buttonList.emplace_back(t);
    t = {4, apps[1].rectObjects.at(4), nullptr, "4"};
    apps[1].buttonList.emplace_back(t);
    t = {5, apps[1].rectObjects.at(5), nullptr, "5"};
    apps[1].buttonList.emplace_back(t);
    t = {6, apps[1].rectObjects.at(6), nullptr, "6"};
    apps[1].buttonList.emplace_back(t);
    t = {7, apps[1].rectObjects.at(7), nullptr, "-"};
    apps[1].buttonList.emplace_back(t);
    t = {8, apps[1].rectObjects.at(8), nullptr, "1"};
    apps[1].buttonList.emplace_back(t);
    t = {9, apps[1].rectObjects.at(9), nullptr, "2"};
    apps[1].buttonList.emplace_back(t);
    t = {10, apps[1].rectObjects.at(10), nullptr, "3"};
    apps[1].buttonList.emplace_back(t);
    t = {11, apps[1].rectObjects.at(11), nullptr, "*"};
    apps[1].buttonList.emplace_back(t);
    t = {12, apps[1].rectObjects.at(12), nullptr, "."};
    apps[1].buttonList.emplace_back(t);
    t = {13, apps[1].rectObjects.at(13), nullptr, "0"};
    apps[1].buttonList.emplace_back(t);
    t = {14, apps[1].rectObjects.at(14), nullptr, "/"};
    apps[1].buttonList.emplace_back(t);
    t = {15, apps[1].rectObjects.at(15), nullptr, "="};
    apps[1].buttonList.emplace_back(t);
    apps[1].appGenerated = true;
}
void app1(){
    if(!apps[1].appGenerated)
        app1_gen();
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    SDL_Rect display = {200, 50, 500, 100};
    std::string textOnScreen = "";
    int last_id = -1;
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        setRendererColor(renderer, 0);
        drawGrid(renderer, apps[1].rectObjects, 4, 4);
        setRendererColor(renderer, 10);
        SDL_RenderFillRect(renderer, &display);
        vector2 displayCen = {210, 92};
        for(i =0; i<apps[1].rectObjects.size(); ++i){
            std::string tmp = apps[1].buttonList.at(i).text_on;
            genText(renderer, 0, tmp, rectCenter(apps[1].rectObjects[i]));
        }
        if(textOnScreen.size()<=36 && lastBtn(1))
            textOnScreen.append(apps[1].buttonList.at(apps[1].lastPressBtn).text_on);
        if(textOnScreen!="")
            genText(renderer, 5, textOnScreen, displayCen);
        SDL_RenderPresent(renderer);
    }
}
void app2(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void app3(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void app4(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void app5(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void app6(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        SDL_RenderPresent(renderer);
    }
}
void credits(){
    setRendererColor(renderer, 1);
    SDL_RenderClear(renderer);
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        genTextFile(renderer, "txt/credits.txt", 100, 100);
        SDL_RenderPresent(renderer);
    }
}
int main(int argc, char *argv[]){
    init();
    int program;
    apps[0] = {"test", "img/icon.png"};
    apps[1] = {"kalk", "img/kalk.png"};
    apps[2] = {"prop","img/kot1.png"};
    apps[3] = {"diceSim", "img/kot2.png"};
    apps[4] = {"plc4", "img/kot3.png"};
    apps[5] = {"ledy", "img/led.png"};
    apps[6] = {"psmo", "img/czefo.png"};
    //app psdy = {7};
    //app psas = {8};
    apps[191] = {"credits", "img/c.png"};
    genGrid(&grid, IHEIGHT, IWIDTH, ISIZE);
    SDL_Rect rect = {100, 100, 100, 100};
    while (!( SDL_PollEvent( &windowEvent) && SDL_QUIT == windowEvent.type)){
        setRendererColor(renderer, 1);
        SDL_RenderClear(renderer);
        setRendererColor(renderer, 0);
        drawGrid(renderer, grid, IHEIGHT, IWIDTH);
        for(i=0; i<=APPNUM; ++i){
            if(apps[i].name=="")
                ++i;
            else
                newIcon(renderer, apps[i], i);
        }
        program = whichIcon(ISIZE, HEIGHT, WIDTH);
        if(program != -1){
            switch (program){
            case 0:
                app0();
                break;
            case 1:
                app1();
                break;
            case 2:
                app2();
                break;
            case 3:
                app3();
                break;
            case 4:
                app4();
                break;
            case 5:
                app5();
                break;
            case 6:
                app6();
                break;
            case 191:
                credits();
                break;
            default:
                std::cout << program;
                break;
            }
            
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
    freeMem();
    return EXIT_SUCCESS;
}