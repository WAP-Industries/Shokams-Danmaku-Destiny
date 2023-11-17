#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"

#include <iostream>
#include <cassert>
#include <windows.h>
#include <string>
#include <vector>
#include <variant>
#include <map>
#include <optional>
#include <functional>
#include <chrono>
#include <thread>
#include <cmath>
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#define SEEDRAND std::srand((unsigned) time(NULL))
#define CHECKPAUSE if (Game.isPaused) {while (Game.isPaused);}
#define CHECKEND if (Game.Ending.has_value()) {CLEARATTACK; return;} \
                    if (Game.isTransitioning) {while (Game.isTransitioning);}
#define CHECKATKEND if (Game.Ending.has_value() || Game.isTransitioning) {CLEARATTACK; return;} CHECKPAUSE;
#define RENDERALLGUI(C) std::for_each(C.begin(), C.end(), [](Sprite* g){g->Render();});
#define RENDERSCREEN return SDL_RenderPresent(Renderer)
#define INSTRUCT(K, T) "Press [" + std::string(K) + "] to " + std::string(T)
#define ISPHASE2 Shokam.Health<Settings.ShokamHealth/2
#define EMPTYLIST(L) for (auto i:L) delete i; L.clear();
#define GETMILLI(T) T*1000/Settings.MaxFPS

#include "gameplay/settings.hpp"
#include "gameplay/game.hpp"

SDL_Window* Window = SDL_CreateWindow(
    Settings.Title.c_str(),
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    Settings.WindowWidth, Settings.WindowWidth,
    SDL_WINDOW_SHOWN
);

SDL_Renderer* Renderer = SDL_CreateRenderer(
    Window, 
    -1, 
    SDL_RENDERER_ACCELERATED
);

std::map<std::string, SDL_Texture*> Assets;
std::map<std::string, Mix_Music*> Music;

#include "gameplay/threading.hpp"
#include "gameplay/audio.hpp"

void Close(){
    SDL_DestroyRenderer(Renderer);
    SDL_DestroyWindow(Window);
    SDL_Quit();
    Mix_Quit();
}

#include "gameplay/sprite.hpp"
#include "gameplay/entities.hpp"
#include "gameplay/gui.hpp"
#include "gameplay/timer.hpp"

std::vector<Sprite*> Sprites = {&Shokam, &Player, &PlayerHeart};
std::vector<Sprite*> Hearts = {};
std::vector<Sprite*> Effects;
std::vector<Projectile*> Projectiles;
std::vector<Sprite*> GUI = {&ShokamHealthBar, &ShokamNamebar, &Timer.Display};
std::vector<Sprite*> SuperAttackGUI = {&SuperAttackMask, &SuperAttackName};
std::vector<Sprite*> ScreenGUI = {&EndScreen, &TopText, &ScoreText, &BottomText, &InstructText};
std::vector<TextObject*> MenuGUI;

int RandInt(int min, int max){
    return std::rand()%(max-min+1)+min;
}

void DestroyProjectile(int i){
    delete Projectiles.at(i);
    Projectiles.erase(Projectiles.begin()+i);
}

#include "gameplay/effects.hpp"
#include "gameplay/attacks.hpp"
#include "gameplay/bossfight.hpp"
#include "gameplay/ending.hpp"

void LoadAssets(){
    int channel = 0;
    for (const auto& outer: std::filesystem::directory_iterator("assets")){
        if (outer.is_directory()){
            for (const auto& inner: std::filesystem::directory_iterator(outer.path().string())){
                std::string src = inner.path().string();
                if (inner.is_regular_file()){
                    SDL_Texture* Texture = IMG_LoadTexture(Renderer, src.c_str());
                    if (Texture==nullptr)
                        throw std::runtime_error("Failed to load image: "+src);
                    Assets.insert({src, Texture});
                }
                else{
                    for (const auto &audiofile: std::filesystem::directory_iterator(src)){
                        std::string af = audiofile.path().string();
                        if (src=="assets\\audio\\music") Music[af] = Mix_LoadMUS(af.c_str());
                        else LoadAudio(af, channel);
                    }
                }
                channel++;
            }
        }
    }
}

void PlayerAttack(){
    CHECKEND;
    Projectiles.push_back(
        new Projectile(
            "11",
            Player.Position.X+(Player.Width-Settings.PlayerProjectileSize.first)/2,
            Player.Position.Y-Settings.PlayerProjectileSize.second,
            Settings.PlayerProjectileSize.first, Settings.PlayerProjectileSize.second,
            Settings.PlayerProjectileSpeed, M_PI,
            true
        )
    );
}

void Init(){
    Player.Speed = Settings.PlayerSpeed;
    Player.Position.X = Shokam.Position.X = (Settings.WindowWidth-Player.Width)/2;
    Player.Position.Y = Settings.WindowWidth-Player.Height;
    Player.RotationRate = Shokam.RotationRate = 0;
    Player.Rotation = Shokam.Rotation = 0;

    int width = 30, margin = 10;
    PlayerHeart.Health = Settings.PlayerHealth;
    EMPTYLIST(Hearts);
    for (int i=0;i<Settings.PlayerHealth;i++){
        Hearts.push_back(
            new Sprite(
                "assets\\entities\\heart.png",
                margin+i*width+i*margin, Settings.WindowWidth-width-margin,
                width, width
            )
        );
    }

    ShokamNamebar.Text = Settings.ShokamName.first;
    Shokam.Image = "assets\\entities\\shokam1.png";
    Shokam.Health = Settings.ShokamHealth;
    Shokam.Speed = Settings.ShokamSpeed;
    Shokam.Position.Y = 50;
    ShokamHealthBar.Width = (int)((float)Shokam.Health/(float)Settings.ShokamHealth*Settings.WindowWidth*0.8);
    ShokamEffect.ODim = std::make_pair(ShokamEffect.Width, ShokamEffect.Height);

    EndScreen.Opacity = 0;
    InstructText.Text = "";

    SuperAttackName.Text = "";
    SuperAttackMask.Opacity = 0;
    
    Sprites = {&Shokam, &Player, &PlayerHeart};
    Game.StartSprites = Sprites.size();

    EMPTYLIST(MenuGUI);

    Mix_FadeOutMusic(0);
    PlayMusic("phase1");

    Game.isRewinding = false;
    Game.ScreenLimit = 0;
    Game.Running = true;
    Game.isEndScreen = false;

    Timer.ElapsedTime = 0;
    StartBossFight();
}

void SetStartScreen(){
    Game.Ending = std::nullopt;

    TopText.Opacity = 1;
    EMPTYLIST(MenuGUI)

    TopText.XCentered = TopText.YCentered = ScoreText.XCentered = BottomText.XCentered = true;
    InstructText.Position.Y = Settings.WindowWidth-InstructText.FontSize-10;
    TopText.Text = Settings.Title;
    BottomText.Text = ScoreText.Text = ""; 
    InstructText.Text = "Select: [ENTER], Toggle: [WASD/ARROWS]";

    if (!Mix_PlayingMusic()) 
        PlayMusic("menu");
    
    int w, h; 
    std::tie(w, h) = GetTextSize(TopText.Text, TopText.FontSize); 
    for (int i=0;i<(int)Game.SelectText.size();i++) { 
        MenuGUI.push_back(new TextObject(Game.SelectText.at(i), 0,0, Game.SelectFontSize)); 
        auto t = MenuGUI.at(MenuGUI.size()-1); 
        t->XCentered = true;
        t->Position.Y+=(Settings.WindowWidth-h)/2+h+(i+1)*Game.SelectFontSize+i*20;
    }
}

void ChangeSelect(int Step){
    PlayAudio("assets\\audio\\misc\\toggle.wav");
    Game.SelectState+=Step;
    Game.SelectState = (Game.SelectState<0 ? Game.SelectText.size()-1:(Game.SelectState>(int)Game.SelectText.size()-1 ? 0:Game.SelectState));
}

void AddText(std::vector<std::pair<std::string, int>> Texts, int Sep){
    TopText.Opacity = 0;
    std::for_each(MenuGUI.begin(), MenuGUI.end(), [](TextObject* t){t->Opacity = 0;});
    for (int i=0;i<(int)Texts.size();i++){
        auto T = new TextObject(
            Texts.at(i).first, 
            0,0,
            Texts.at(i).second
        );
        T->XCentered = true, T->Position.Y = Settings.WindowWidth/2+(i-(int)(Texts.size()/2))*T->FontSize+i*Sep;
        MenuGUI.push_back(T);
    }
    InstructText.Text = "Return: [A/<]";
}

void ChooseSelect(){
    std::function<void()> Functions[] = {
        Init, 
        [](){
            AddText({
                {"Controls", 50},
                {"Movement: [WASD/ARROWS]", 30}, 
                {"Pause/Resume: [SPACE]", 30}
            }, 20);
        }, 
        [](){
            AddText({
                {"Credits", 50},
                {"Game designer - WAP Industries", 30},
                {"Game programmer - WAP Industries", 30},
                {"Music consultant - La Fresco Tesco", 30},
                {"Lead design consultant - Zikavirus", 30},
                {"Assistant design consultant - Zachary", 30},
                {"Assistant design consultant - ReaperYK", 30}
            }, 10);
        }
    };
    Functions[Game.SelectState]();
}