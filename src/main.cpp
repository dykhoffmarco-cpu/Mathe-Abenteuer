#include <SDL.h>
#include <emscripten.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>

enum Screen { MENU, GAME, RESULT };
enum Mode { PLUS_MODE, MINUS_MODE, ENDLESS_MODE };

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static Screen screen = MENU;
static Mode mode = PLUS_MODE;
static int numberLimit = 10;
static int questionCount = 10;
static int questionIndex = 0;
static int correctCount = 0;
static int wrongCount = 0;
static int a = 1, b = 1, solution = 2;
static int bestEndless = 0;
static std::string playerName = "Spieler";
static std::string answerInput;
static std::string feedback;
static bool feedbackCorrect = false;
static bool soundOn = true;
static Uint32 feedbackUntil = 0;
static Uint32 lastQuestionA = 0, lastQuestionB = 0;
static bool hasPreviousQuestion = false;

struct Color { Uint8 r,g,b,a; };
static const Color BG{245,247,252,255}, NAVY{34,45,74,255}, BLUE{75,105,220,255};
static const Color PURPLE{132,91,210,255}, GREEN{43,166,112,255}, RED{214,75,82,255};
static const Color WHITE{255,255,255,255}, MUTED{115,126,148,255}, PALE{229,234,245,255};
static const Color YELLOW{255,196,77,255};

static void setColor(Color c) { SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a); }
static void fillRect(int x,int y,int w,int h,Color c) {
    setColor(c); SDL_Rect r{x,y,w,h}; SDL_RenderFillRect(renderer,&r);
}
static void outlineRect(int x,int y,int w,int h,Color c) {
    setColor(c); SDL_Rect r{x,y,w,h}; SDL_RenderDrawRect(renderer,&r);
}
static void line(int x1,int y1,int x2,int y2,Color c) {
    setColor(c); SDL_RenderDrawLine(renderer,x1,y1,x2,y2);
}

// Kleine eingebaute 5x7-Schrift; dadurch werden keine zusätzlichen Font-Dateien benoetigt.
static const char* glyph(char c) {
    switch(c) {
    case 'A': return "01110""10001""10001""11111""10001""10001""10001";
    case 'B': return "11110""10001""10001""11110""10001""10001""11110";
    case 'C': return "01111""10000""10000""10000""10000""10000""01111";
    case 'D': return "11110""10001""10001""10001""10001""10001""11110";
    case 'E': return "11111""10000""10000""11110""10000""10000""11111";
    case 'F': return "11111""10000""10000""11110""10000""10000""10000";
    case 'G': return "01111""10000""10000""10111""10001""10001""01111";
    case 'H': return "10001""10001""10001""11111""10001""10001""10001";
    case 'I': return "11111""00100""00100""00100""00100""00100""11111";
    case 'J': return "00111""00010""00010""00010""10010""10010""01100";
    case 'K': return "10001""10010""10100""11000""10100""10010""10001";
    case 'L': return "10000""10000""10000""10000""10000""10000""11111";
    case 'M': return "10001""11011""10101""10101""10001""10001""10001";
    case 'N': return "10001""11001""10101""10011""10001""10001""10001";
    case 'O': return "01110""10001""10001""10001""10001""10001""01110";
    case 'P': return "11110""10001""10001""11110""10000""10000""10000";
    case 'Q': return "01110""10001""10001""10001""10101""10010""01101";
    case 'R': return "11110""10001""10001""11110""10100""10010""10001";
    case 'S': return "01111""10000""10000""01110""00001""00001""11110";
    case 'T': return "11111""00100""00100""00100""00100""00100""00100";
    case 'U': return "10001""10001""10001""10001""10001""10001""01110";
    case 'V': return "10001""10001""10001""10001""10001""01010""00100";
    case 'W': return "10001""10001""10001""10101""10101""10101""01010";
    case 'X': return "10001""10001""01010""00100""01010""10001""10001";
    case 'Y': return "10001""10001""01010""00100""00100""00100""00100";
    case 'Z': return "11111""00001""00010""00100""01000""10000""11111";
    case '0': return "01110""10001""10011""10101""11001""10001""01110";
    case '1': return "00100""01100""00100""00100""00100""00100""01110";
    case '2': return "01110""10001""00001""00010""00100""01000""11111";
    case '3': return "11110""00001""00001""01110""00001""00001""11110";
    case '4': return "00010""00110""01010""10010""11111""00010""00010";
    case '5': return "11111""10000""10000""11110""00001""00001""11110";
    case '6': return "01110""10000""10000""11110""10001""10001""01110";
    case '7': return "11111""00001""00010""00100""01000""01000""01000";
    case '8': return "01110""10001""10001""01110""10001""10001""01110";
    case '9': return "01110""10001""10001""01111""00001""00001""01110";
    case '+': return "00000""00100""00100""11111""00100""00100""00000";
    case '-': return "00000""00000""00000""11111""00000""00000""00000";
    case '=': return "00000""11111""00000""11111""00000""00000""00000";
    case ':': return "00000""00100""00100""00000""00100""00100""00000";
    case '.': return "00000""00000""00000""00000""00000""00110""00110";
    case '!': return "00100""00100""00100""00100""00100""00000""00100";
    case '?': return "01110""10001""00001""00010""00100""00000""00100";
    case '/': return "00001""00010""00010""00100""01000""01000""10000";
    case ' ': return "00000""00000""00000""00000""00000""00000""00000";
    default: return "00000""00000""00000""00000""00000""00000""00000";
    }
}
static std::string upper(std::string s) {
    for(char& c:s) if(c>='a'&&c<='z') c=char(c-'a'+'A');
    return s;
}
static int textWidth(const std::string& s,int scale) { return (int)s.size()*6*scale-scale; }
static void drawText(const std::string& raw,int x,int y,int scale,Color color,bool centered=false) {
    std::string s=upper(raw);
    if(centered) x-=textWidth(s,scale)/2;
    setColor(color);
    for(size_t i=0;i<s.size();++i) {
        const char* bits=glyph(s[i]);
        for(int row=0;row<7;++row) for(int col=0;col<5;++col)
            if(bits[row*5+col]=='1') {
                SDL_Rect r{x+(int)i*6*scale+col*scale,y+row*scale,scale,scale};
                SDL_RenderFillRect(renderer,&r);
            }
    }
}
static bool inside(int x,int y,int rx,int ry,int rw,int rh) {
    return x>=rx&&x<=rx+rw&&y>=ry&&y<=ry+rh;
}
static void button(int x,int y,int w,int h,const std::string& label,Color c,bool selected=false) {
    fillRect(x,y,w,h,selected?PURPLE:c);
    outlineRect(x,y,w,h,selected?PURPLE:PALE);
    drawText(label,x+w/2,y+h/2-7,2,WHITE,true);
}
static void loadBest() {
    bestEndless = EM_ASM_INT({ return Number(localStorage.getItem('matheAbenteuerBest') || 0); });
}
static void saveBest(int score) {
    EM_ASM({ localStorage.setItem('matheAbenteuerBest', $0); }, score);
}
static void makeQuestion() {
    answerInput.clear();
    feedback.clear();
    do {
        if(mode==PLUS_MODE) {
            a=1+rand()%(numberLimit-1);
            b=1+rand()%(numberLimit-a);
            solution=a+b;
        } else {
            a=1+rand()%numberLimit;
            b=1+rand()%a;
            solution=a-b; // Null als Ergebnis ist erlaubt.
        }
    } while(hasPreviousQuestion &&
             (a==lastQuestionA && b==lastQuestionB));
    lastQuestionA=a; lastQuestionB=b; hasPreviousQuestion=true;
}
static void beginGame() {
    questionIndex=0; correctCount=0; wrongCount=0;
    hasPreviousQuestion=false; screen=GAME; makeQuestion();
}
static void finishGame() { screen=RESULT; }
static void submitAnswer() {
    if(answerInput.empty() || feedback.size()) return;
    int entered=0;
    try { entered=std::stoi(answerInput); } catch(...) { return; }
    if(entered==solution) { ++correctCount; feedback="RICHTIG!"; feedbackCorrect=true; }
    else { ++wrongCount; feedback="LEIDER FALSCH"; feedbackCorrect=false; }
    feedbackUntil=SDL_GetTicks()+750;
}
static void nextQuestion() {
    ++questionIndex;
    if(mode!=ENDLESS_MODE && questionIndex>=questionCount) { finishGame(); return; }
    makeQuestion();
}
static void drawMenu() {
    drawText("MATHE ABENTEUER",400,48,4,NAVY,true);
    drawText("RECHNEN MACHT SPASS",400,90,2,MUTED,true);
    drawText("DEIN NAME:",90,145,2,NAVY);
    fillRect(90,168,620,44,WHITE); outlineRect(90,168,620,44,PALE);
    drawText(playerName,108,181,2,BLUE);
    drawText("MODUS",90,238,2,NAVY);
    button(90,270,190,58,"PLUS",BLUE,mode==PLUS_MODE);
    button(305,270,190,58,"MINUS",GREEN,mode==MINUS_MODE);
    button(520,270,190,58,"ENDLOS",PURPLE,mode==ENDLESS_MODE);
    drawText("ZAHLEN BIS",90,355,2,NAVY);
    button(90,385,190,52,"10",BLUE,numberLimit==10);
    button(305,385,190,52,"100",BLUE,numberLimit==100);
    button(520,385,190,52,"1000",BLUE,numberLimit==1000);
    if(mode!=ENDLESS_MODE) {
        drawText("ANZAHL AUFGABEN",90,465,2,NAVY);
        button(90,495,130,48,"10",PURPLE,questionCount==10);
        button(235,495,130,48,"20",PURPLE,questionCount==20);
        button(380,495,130,48,"50",PURPLE,questionCount==50);
        button(525,495,130,48,"100",PURPLE,questionCount==100);
    } else {
        drawText("ENDLOS REKORD: "+std::to_string(bestEndless),400,480,2,PURPLE,true);
    }
    button(235,570,330,62,"LOS GEHTS!",GREEN);
    drawText(std::string("SOUND: ")+(soundOn?"AN":"AUS"),650,24,1,MUTED);
}
static void drawGame() {
    drawText(mode==PLUS_MODE?"PLUS MODUS":mode==MINUS_MODE?"MINUS MODUS":"ENDLOS MODUS",400,40,3,NAVY,true);
    std::string progress=mode==ENDLESS_MODE?"AUFGABE "+std::to_string(questionIndex+1):
        "AUFGABE "+std::to_string(questionIndex+1)+" / "+std::to_string(questionCount);
    drawText(progress,400,92,2,MUTED,true);
    fillRect(90,130,620,12,PALE);
    int total=mode==ENDLESS_MODE?std::max(1,questionIndex+1):questionCount;
    fillRect(90,130,620*(mode==ENDLESS_MODE?((questionIndex%10)+1)/10.0:std::min(1.0,(double)questionIndex/questionCount)),12,BLUE);
    drawText(std::to_string(a)+(mode==PLUS_MODE?" + ":" - ")+std::to_string(b)+" = ?",400,220,5,NAVY,true);
    fillRect(250,315,300,64,WHITE); outlineRect(250,315,300,64,PALE);
    drawText(answerInput.empty()?"ANTWORT":answerInput,400,337,3,BLUE,true);
    drawText("TASTATUR: ZAHL EINGEBEN, ENTER DRUECKEN",400,410,1,MUTED,true);
    drawText("RICHTIG: "+std::to_string(correctCount)+"    FEHLER: "+std::to_string(wrongCount),400,475,2,NAVY,true);
    if(!feedback.empty()) drawText(feedback,400,535,3,feedbackCorrect?GREEN:RED,true);
    button(285,585,230,52,"ANTWORT PRUEFEN",BLUE);
}
static void drawResult() {
    drawText("GESCHAFFT!",400,65,4,NAVY,true);
    drawText(playerName+" - DEIN ERGEBNIS",400,145,2,MUTED,true);
    drawText("RICHTIG: "+std::to_string(correctCount),400,220,3,GREEN,true);
    drawText("FEHLER: "+std::to_string(wrongCount),400,270,3,RED,true);
    int total=correctCount+wrongCount;
    int percent=total?correctCount*100/total:0;
    drawText("TREFFERQUOTE: "+std::to_string(percent)+"%",400,340,3,BLUE,true);
    if(mode==ENDLESS_MODE && correctCount>bestEndless) {
        bestEndless=correctCount; saveBest(bestEndless);
        drawText("NEUER REKORD!",400,405,2,PURPLE,true);
    } else if(mode==ENDLESS_MODE) drawText("REKORD: "+std::to_string(bestEndless),400,405,2,PURPLE,true);
    std::string rating=wrongCount==0?"SEHR GUT":wrongCount<=2?"GUT":wrongCount<=5?"GEHT SO":"UEBEN LOHNT SICH";
    drawText("BEWERTUNG: "+rating,400,465,2,NAVY,true);
    button(235,565,330,62,"ZUM STARTBILDSCHIRM",GREEN);
}
static void render() {
    setColor(BG); SDL_RenderClear(renderer);
    if(screen==MENU) drawMenu(); else if(screen==GAME) drawGame(); else drawResult();
    SDL_RenderPresent(renderer);
}
static void handleClick(int x,int y) {
    if(screen==MENU) {
        if(inside(x,y,90,270,190,58)) mode=PLUS_MODE;
        else if(inside(x,y,305,270,190,58)) mode=MINUS_MODE;
        else if(inside(x,y,520,270,190,58)) mode=ENDLESS_MODE;
        else if(inside(x,y,90,385,190,52)) numberLimit=10;
        else if(inside(x,y,305,385,190,52)) numberLimit=100;
        else if(inside(x,y,520,385,190,52)) numberLimit=1000;
        else if(mode!=ENDLESS_MODE && inside(x,y,90,495,130,48)) questionCount=10;
        else if(mode!=ENDLESS_MODE && inside(x,y,235,495,130,48)) questionCount=20;
        else if(mode!=ENDLESS_MODE && inside(x,y,380,495,130,48)) questionCount=50;
        else if(mode!=ENDLESS_MODE && inside(x,y,525,495,130,48)) questionCount=100;
        else if(inside(x,y,235,570,330,62)) beginGame();
        else if(inside(x,y,90,168,620,44)) { /* Name can be entered with keyboard after clicking field. */ }
        else if(inside(x,y,630,12,100,30)) soundOn=!soundOn;
    } else if(screen==GAME) {
        if(inside(x,y,285,585,230,52)) {
            if(feedback.empty()) submitAnswer();
            else if(SDL_GetTicks()>=feedbackUntil) nextQuestion();
        }
    } else if(screen==RESULT) {
        if(inside(x,y,235,565,330,62)) screen=MENU;
    }
}
static void mainLoop() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type==SDL_QUIT) { emscripten_cancel_main_loop(); return; }
        if(e.type==SDL_MOUSEBUTTONDOWN) handleClick(e.button.x,e.button.y);
        if(e.type==SDL_TEXTINPUT) {
            if(screen==MENU) {
                if(playerName=="Spieler") playerName.clear();
                if(playerName.size()<14) playerName+=e.text.text;
            } else if(screen==GAME && feedback.empty()) {
                std::string t=e.text.text;
                for(char c:t) if(c>='0'&&c<='9'&&answerInput.size()<5) answerInput+=c;
            }
        }
        if(e.type==SDL_KEYDOWN) {
            if(e.key.keysym.sym==SDLK_BACKSPACE) {
                if(screen==MENU && !playerName.empty()) playerName.pop_back();
                if(screen==GAME && !answerInput.empty() && feedback.empty()) answerInput.pop_back();
            }
            if(e.key.keysym.sym==SDLK_RETURN && screen==GAME) {
                if(feedback.empty()) submitAnswer();
                else if(SDL_GetTicks()>=feedbackUntil) nextQuestion();
            }
            if(e.key.keysym.sym==SDLK_RETURN && screen==MENU) beginGame();
        }
    }
    if(screen==GAME && !feedback.empty() && SDL_GetTicks()>=feedbackUntil) {
        // Nach dem Feedback Enter oder den Pruefen-Button druecken, um weiterzugehen.
    }
    render();
}
int main() {
    srand((unsigned)time(nullptr));
    loadBest();
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)!=0) return 1;
    window=SDL_CreateWindow("Mathe-Abenteuer",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,680,SDL_WINDOW_SHOWN);
    if(!window) return 1;
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) renderer=SDL_CreateRenderer(window,-1,0);
    if(!renderer) return 1;
    SDL_StartTextInput();
    emscripten_set_main_loop(mainLoop,0,1);
    return 0;
}
