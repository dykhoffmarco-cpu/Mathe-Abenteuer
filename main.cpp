#include <SDL.h>
#include <emscripten.h>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unordered_map>

const int W = 960, H = 640;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

enum Screen { HOME, SETTINGS, PLAY, RESULT };
enum Mode { PLUS, MINUS, UNLIMITED };
Screen screen = HOME;
Mode mode = PLUS;

int numberLimit = 10;
int questionCount = 10;
int questionIndex = 0;
int correctCount = 0;
int wrongCount = 0;
int points = 0;
int bestPlus = 0, bestMinus = 0, bestUnlimited = 0;
int a = 0, b = 0, solution = 0;
Uint32 feedbackUntil = 0;
bool feedbackVisible = false;
bool lastAnswerCorrect = false;
bool answerFocus = false;
bool nameFocus = false;
std::string playerName = "Mathe-Fan";
std::string answerText = "";
std::string feedbackText = "";

const SDL_Color BG      = {246, 248, 255, 255};
const SDL_Color NAVY    = {39, 52, 91, 255};
const SDL_Color PURPLE  = {113, 92, 220, 255};
const SDL_Color BLUE    = {75, 139, 220, 255};
const SDL_Color PALE    = {232, 235, 255, 255};
const SDL_Color WHITE   = {255, 255, 255, 255};
const SDL_Color GREEN   = {42, 158, 108, 255};
const SDL_Color RED     = {210, 78, 88, 255};
const SDL_Color YELLOW  = {255, 218, 115, 255};
const SDL_Color MUTED   = {112, 122, 151, 255};

using Glyph = std::array<const char*, 7>;
const std::unordered_map<char, Glyph> FONT = {
 {'A',{"01110","10001","10001","11111","10001","10001","10001"}},
 {'B',{"11110","10001","10001","11110","10001","10001","11110"}},
 {'C',{"01111","10000","10000","10000","10000","10000","01111"}},
 {'D',{"11110","10001","10001","10001","10001","10001","11110"}},
 {'E',{"11111","10000","10000","11110","10000","10000","11111"}},
 {'F',{"11111","10000","10000","11110","10000","10000","10000"}},
 {'G',{"01111","10000","10000","10111","10001","10001","01111"}},
 {'H',{"10001","10001","10001","11111","10001","10001","10001"}},
 {'I',{"11111","00100","00100","00100","00100","00100","11111"}},
 {'J',{"00111","00010","00010","00010","10010","10010","01100"}},
 {'K',{"10001","10010","10100","11000","10100","10010","10001"}},
 {'L',{"10000","10000","10000","10000","10000","10000","11111"}},
 {'M',{"10001","11011","10101","10101","10001","10001","10001"}},
 {'N',{"10001","11001","10101","10011","10001","10001","10001"}},
 {'O',{"01110","10001","10001","10001","10001","10001","01110"}},
 {'P',{"11110","10001","10001","11110","10000","10000","10000"}},
 {'Q',{"01110","10001","10001","10001","10101","10010","01101"}},
 {'R',{"11110","10001","10001","11110","10100","10010","10001"}},
 {'S',{"01111","10000","10000","01110","00001","00001","11110"}},
 {'T',{"11111","00100","00100","00100","00100","00100","00100"}},
 {'U',{"10001","10001","10001","10001","10001","10001","01110"}},
 {'V',{"10001","10001","10001","10001","10001","01010","00100"}},
 {'W',{"10001","10001","10001","10101","10101","10101","01010"}},
 {'X',{"10001","10001","01010","00100","01010","10001","10001"}},
 {'Y',{"10001","10001","01010","00100","00100","00100","00100"}},
 {'Z',{"11111","00001","00010","00100","01000","10000","11111"}},
 {'0',{"01110","10001","10011","10101","11001","10001","01110"}},
 {'1',{"00100","01100","00100","00100","00100","00100","01110"}},
 {'2',{"01110","10001","00001","00010","00100","01000","11111"}},
 {'3',{"11110","00001","00001","01110","00001","00001","11110"}},
 {'4',{"00010","00110","01010","10010","11111","00010","00010"}},
 {'5',{"11111","10000","10000","11110","00001","00001","11110"}},
 {'6',{"01110","10000","10000","11110","10001","10001","01110"}},
 {'7',{"11111","00001","00010","00100","01000","01000","01000"}},
 {'8',{"01110","10001","10001","01110","10001","10001","01110"}},
 {'9',{"01110","10001","10001","01111","00001","00001","01110"}},
 {' ',{"00000","00000","00000","00000","00000","00000","00000"}},
 {'-',{"00000","00000","00000","11111","00000","00000","00000"}},
 {'+',{"00000","00100","00100","11111","00100","00100","00000"}},
 {'=',{"00000","00000","11111","00000","11111","00000","00000"}},
 {':',{"00000","00100","00100","00000","00100","00100","00000"}},
 {'.',{"00000","00000","00000","00000","00000","00110","00110"}},
 {'!',{"00100","00100","00100","00100","00100","00000","00100"}},
 {'?',{"01110","10001","00001","00010","00100","00000","00100"}},
 {'/',{"00001","00010","00010","00100","01000","01000","10000"}}
};

void color(SDL_Color c) { SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a); }

void rect(int x, int y, int w, int h, SDL_Color c) {
    color(c);
    SDL_Rect r{x,y,w,h};
    SDL_RenderFillRect(renderer, &r);
}

void outline(int x, int y, int w, int h, SDL_Color c) {
    color(c);
    SDL_Rect r{x,y,w,h};
    SDL_RenderDrawRect(renderer, &r);
}

void text(const std::string& s, int x, int y, int scale, SDL_Color c) {
    color(c);
    int cursor = x;
    for (char raw : s) {
        char ch = raw;
        if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
        auto it = FONT.find(ch);
        if (it != FONT.end()) {
            for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (it->second[row][col] == '1') {
                        SDL_Rect p{cursor + col*scale, y + row*scale, scale, scale};
                        SDL_RenderFillRect(renderer, &p);
                    }
        }
        cursor += 6*scale;
    }
}

int textWidth(const std::string& s, int scale) { return (int)s.size()*6*scale; }

void centered(const std::string& s, int y, int scale, SDL_Color c) {
    text(s, (W-textWidth(s,scale))/2, y, scale, c);
}

void button(int x, int y, int w, int h, const std::string& label,
            SDL_Color fill, SDL_Color ink = WHITE) {
    rect(x,y,w,h,fill);
    outline(x,y,w,h,fill);
    int scale = 2;
    text(label, x+(w-textWidth(label,scale))/2, y+(h-14)/2, scale, ink);
}

bool inside(int x, int y, int bx, int by, int bw, int bh) {
    return x>=bx && x<=bx+bw && y>=by && y<=by+bh;
}

void newQuestion() {
    answerText.clear();
    answerFocus = true;
    if (mode == PLUS) {
        a = rand() % (numberLimit + 1);
        b = rand() % (numberLimit - a + 1);
        solution = a+b;
    } else {
        a = rand() % (numberLimit + 1);
        b = rand() % (a + 1);
        solution = a-b;
    }
}

int& bestForMode() {
    if (mode == PLUS) return bestPlus;
    if (mode == MINUS) return bestMinus;
    return bestUnlimited;
}

void startRound() {
    questionIndex = correctCount = wrongCount = points = 0;
    feedbackVisible = false;
    screen = PLAY;
    newQuestion();
}

std::string grade() {
    if (wrongCount == 0) return "SEHR GUT";
    if (wrongCount <= 2) return "GUT";
    if (wrongCount <= 5) return "GEHT SO";
    return "KOENNTE BESSER";
}

void finishRound() {
    bestForMode() = std::max(bestForMode(), points);
    screen = RESULT;
    answerFocus = false;
}

void submitAnswer() {
    if (answerText.empty() || feedbackVisible) return;
    int value = std::atoi(answerText.c_str());
    lastAnswerCorrect = (value == solution);
    if (lastAnswerCorrect) {
        ++correctCount;
        points += 10;
        feedbackText = "RICHTIG! +10 PUNKTE";
    } else {
        ++wrongCount;
        feedbackText = "LEIDER FALSCH";
    }
    ++questionIndex;
    feedbackVisible = true;
    feedbackUntil = SDL_GetTicks() + 850;
    answerFocus = false;
}

void handleClick(int x, int y) {
    if (screen == HOME) {
        if (inside(x,y,260,190,440,58)) {
            nameFocus = true; answerFocus = false; SDL_StartTextInput();
        } else if (inside(x,y,75,330,245,105)) {
            mode = PLUS; screen = SETTINGS; nameFocus = false;
        } else if (inside(x,y,357,330,245,105)) {
            mode = MINUS; screen = SETTINGS; nameFocus = false;
        } else if (inside(x,y,639,330,245,105)) {
            mode = UNLIMITED; screen = SETTINGS; nameFocus = false;
        }
    } else if (screen == SETTINGS) {
        if (inside(x,y,155,205,190,58)) numberLimit = 10;
        else if (inside(x,y,385,205,190,58)) numberLimit = 100;
        else if (inside(x,y,615,205,190,58)) numberLimit = 1000;
        else if (mode != UNLIMITED && inside(x,y,155,330,145,58)) questionCount = 10;
        else if (mode != UNLIMITED && inside(x,y,315,330,145,58)) questionCount = 20;
        else if (mode != UNLIMITED && inside(x,y,475,330,145,58)) questionCount = 50;
        else if (mode != UNLIMITED && inside(x,y,635,330,145,58)) questionCount = 100;
        else if (inside(x,y,360,465,240,65)) startRound();
        else if (inside(x,y,35,35,150,48)) screen = HOME;
    } else if (screen == PLAY) {
        if (inside(x,y,35,35,150,48)) finishRound();
        else if (inside(x,y,300,385,360,65)) { answerFocus = true; SDL_StartTextInput(); }
        else if (inside(x,y,380,485,200,58)) submitAnswer();
    } else if (screen == RESULT) {
        if (inside(x,y,255,485,200,62)) startRound();
        else if (inside(x,y,505,485,200,62)) screen = HOME;
    }
}

void drawHome() {
    centered("MATHE-ABENTEUER", 75, 5, NAVY);
    centered("LERNEN MIT SPASS!", 130, 2, PURPLE);
    text("DEIN NAME:", 260, 165, 2, MUTED);
    rect(260,190,440,58,WHITE); outline(260,190,440,58,nameFocus?PURPLE:PALE);
    text(playerName,280,212,2,NAVY);
    centered("WELCHES ABENTEUER WAEHLST DU?", 285, 2, NAVY);
    button(75,330,245,105,"PLUS",PURPLE);
    button(357,330,245,105,"MINUS",BLUE);
    button(639,330,245,105,"ENDLOS",GREEN);
    centered("KLICKE AUF EINEN MODUS, UM ZU STARTEN", 500, 1, MUTED);
    centered("*  *  *", 550, 3, YELLOW);
}

void drawSettings() {
    button(35,35,150,48,"ZURUECK",PALE,NAVY);
    centered("DEIN SPIEL EINSTELLEN", 90, 4, NAVY);
    centered("ZAHLENRAUM", 165, 2, PURPLE);
    button(155,205,190,58,"BIS 10",numberLimit==10?PURPLE:PALE,numberLimit==10?WHITE:NAVY);
    button(385,205,190,58,"BIS 100",numberLimit==100?PURPLE:PALE,numberLimit==100?WHITE:NAVY);
    button(615,205,190,58,"BIS 1000",numberLimit==1000?PURPLE:PALE,numberLimit==1000?WHITE:NAVY);
    if (mode != UNLIMITED) {
        centered("ANZAHL DER AUFGABEN", 290, 2, PURPLE);
        button(155,330,145,58,"10",questionCount==10?BLUE:PALE,questionCount==10?WHITE:NAVY);
        button(315,330,145,58,"20",questionCount==20?BLUE:PALE,questionCount==20?WHITE:NAVY);
        button(475,330,145,58,"50",questionCount==50?BLUE:PALE,questionCount==50?WHITE:NAVY);
        button(635,330,145,58,"100",questionCount==100?BLUE:PALE,questionCount==100?WHITE:NAVY);
    } else {
        centered("ENDLOS-MODUS: SPIELE SO LANGE DU MOECHTEST", 330, 2, MUTED);
    }
    button(360,465,240,65,"LOS GEHTS!",GREEN);
}

void drawPlay() {
    button(35,35,150,48,"BEENDEN",PALE,NAVY);
    text(playerName,220,50,2,NAVY);
    if (mode == UNLIMITED) text("ENDLOS",760,50,2,PURPLE);
    else {
        std::string p = std::to_string(std::min(questionIndex+1,questionCount)) + "/" + std::to_string(questionCount);
        text(p,800,50,2,PURPLE);
        rect(220,90,620,12,PALE);
        int filled = questionCount ? 620*questionIndex/questionCount : 0;
        rect(220,90,filled,12,PURPLE);
    }
    centered("WIE VIEL IST DAS?",155,3,NAVY);
    rect(190,220,580,125,WHITE);
    centered(std::to_string(a)+(mode==PLUS?" + ":" - ")+std::to_string(b)+" = ?",260,5,PURPLE);
    centered("DEINE ANTWORT",365,2,MUTED);
    rect(300,385,360,65,WHITE); outline(300,385,360,65,answerFocus?PURPLE:PALE);
    centered(answerText.empty()?"TIPPE DEINE ZAHL":answerText,407,2,NAVY);
    button(380,485,200,58,"PRUEFEN",GREEN);
    if (feedbackVisible) {
        SDL_Color c = lastAnswerCorrect ? GREEN : RED;
        centered(feedbackText,570,2,c);
    } else {
        centered("TIPP: ZAHL EINGEBEN UND ENTER DRUECKEN",570,1,MUTED);
    }
}

void drawResult() {
    centered("GESCHAFFT, "+playerName+"!",75,4,NAVY);
    centered("DEIN ERGEBNIS",145,2,PURPLE);
    rect(205,195,550,235,WHITE);
    text("PUNKTE:",250,225,2,MUTED); text(std::to_string(points),520,225,3,PURPLE);
    text("RICHTIG:",250,275,2,MUTED); text(std::to_string(correctCount),520,275,2,GREEN);
    text("FALSCH:",250,315,2,MUTED); text(std::to_string(wrongCount),520,315,2,RED);
    text("NOTE:",250,355,2,MUTED); text(grade(),520,355,2,NAVY);
    text("BESTLEISTUNG:",250,395,1,MUTED); text(std::to_string(bestForMode()),520,395,2,PURPLE);
    button(255,485,200,62,"NOCHMAL",GREEN);
    button(505,485,200,62,"STARTSEITE",PURPLE);
}

void render() {
    color(BG); SDL_RenderClear(renderer);
    if (screen==HOME) drawHome();
    else if (screen==SETTINGS) drawSettings();
    else if (screen==PLAY) drawPlay();
    else drawResult();
    SDL_RenderPresent(renderer);
}

void frame() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { }
        else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            handleClick(e.button.x, e.button.y);
        } else if (e.type == SDL_TEXTINPUT) {
            std::string typed = e.text.text;
            if (nameFocus && playerName.size()<18) {
                for (char ch : typed) if ((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||ch==' ')
                    playerName += ch;
            } else if (screen==PLAY && answerFocus && answerText.size()<5) {
                for (char ch : typed) if (ch>='0'&&ch<='9') answerText += ch;
            }
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                if (nameFocus && !playerName.empty()) playerName.pop_back();
                else if (answerFocus && !answerText.empty()) answerText.pop_back();
            } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                if (nameFocus) { nameFocus=false; SDL_StopTextInput(); }
                else if (screen==PLAY) submitAnswer();
            } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                if (screen==PLAY) finishRound();
                else if (screen==SETTINGS) screen=HOME;
            }
        }
    }

    if (screen==PLAY && feedbackVisible && SDL_GetTicks()>=feedbackUntil) {
        feedbackVisible=false;
        if (mode!=UNLIMITED && questionIndex>=questionCount) finishRound();
        else newQuestion();
    }
    render();
}

int main() {
    srand((unsigned int)time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    window = SDL_CreateWindow("Mathe-Abenteuer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              W, H, SDL_WINDOW_SHOWN);
    if (!window) return 1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) return 1;
    SDL_StartTextInput();
    emscripten_set_main_loop(frame, 0, 1);
    return 0;
}
