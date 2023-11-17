#include "include.hpp"

struct {
    bool W=false, A=false, S=false, D=false;
} Keys;

Sprite Background1("assets\\background\\background.png", 0,0,Settings.WindowWidth, Settings.WindowWidth);
Sprite Background2 = Background1;
void RenderBackground(){
    std::vector<Sprite*> Backgrounds = {&Background1, &Background2};
    for (int i=0;i<(int)Backgrounds.size();i++){
        Backgrounds.at(i)->Render();
        if (!Game.isPaused) {
            if (Backgrounds.at(i)->Position.Y==0)
                Backgrounds.at(!i)->Position.Y = -Settings.WindowWidth+Settings.BgScroll;
            Backgrounds.at(i)->Position.Y+=Settings.BgScroll;
        }
    }
}

void GameLoop(){
    SDL_SetRenderDrawColor(Renderer, 0,0,0,255);
    SDL_RenderClear(Renderer); 
    if (!Game.Running){
        ScreenBackground.Render();
        RENDERALLGUI(ScreenGUI);
        for (int i=0;i<(int)Game.SelectText.size();i++){
            MenuGUI.at(i)->FontSize = Game.SelectFontSize;
            MenuGUI.at(i)->Text = (
                i==Game.SelectState ? 
                "> " + Game.SelectText.at(i) + " <" :
                Game.SelectText.at(i)  
            );
        }
        MenuGUI.at(Game.SelectState)->FontSize+=5;
        RENDERALLGUI(MenuGUI);
        RENDERSCREEN;
    }

    if (!Game.isPaused && !Game.isTransitioning){
        Timer.ElapsedTime++;
        Timer.Update();
    }

    if (!((GETMILLI(Timer.ElapsedTime))%(Settings.PlayerFireRate)))
        PlayerAttack();

    PlayerHeart.Position.X = Player.Position.X+(Player.Width-PlayerHeart.Width)/2;
    PlayerHeart.Position.Y = Player.Position.Y+(Player.Height-PlayerHeart.Height)/2;
    ShokamHealthBar.Position.X = (Settings.WindowWidth-ShokamHealthBar.Width)/2;

    RenderBackground();
    std::for_each(Sprites.begin(), Sprites.end(), [](Sprite* s){if (!s->RenderFirst)return; s->Update();});
    for (auto e: Effects) e->Render();
    for (auto h: Hearts) h->Render();
    RENDERALLGUI(GUI);
    std::for_each(Sprites.begin(), Sprites.end(), [](Sprite* s){if (s->RenderFirst)return; s->Update();});

    for (auto p: Projectiles){
        p->Update();
        if (Game.isPaused) continue;
        if (p->ExplodeTime.has_value() || p->FreezeTime.has_value()) p->ElapsedTime++;
    }
    RENDERALLGUI(SuperAttackGUI);

    if (Game.isPaused) {
        EndScreen.Opacity = 0.7;
        TopText.Text = "-- PAUSED --";
        BottomText.Text = INSTRUCT("SPACE", "resume");
        RENDERALLGUI(ScreenGUI);
        RENDERSCREEN;
    }
    if (Game.isTransitioning){
        EndScreen.Render();
    }
    if (Game.Ending.has_value()) {
        EndScreen.Render();
        if (EndScreen.Opacity>=Settings.EndScreenOpacity)
            TopText.Render(), ScoreText.Render(), BottomText.Render();
    }

    for (int i=Projectiles.size()-1;i>=0;i--){
        auto P = Projectiles.at(i);

        if (
            P->Rebound && (
                abs(P->Position.X)<=P->Speed ||
                abs(P->Position.Y)<=P->Speed ||
                abs(P->Position.X+P->Width-Settings.WindowWidth)<=P->Speed ||
                abs(P->Position.Y+P->Height-Settings.WindowWidth)<=P->Speed
            )
        ){
            P->Angle+=M_PI/2;
            if (P->RotationRate!=0) P->RotationRate*=-1;
            continue;
        }

        if (
            (
                Game.ScreenLimit.has_value() && (
                    P->Position.X<-P->Width-Game.ScreenLimit.value() || 
                    P->Position.Y<-P->Height-Game.ScreenLimit.value() ||
                    P->Position.X>Settings.WindowWidth+Game.ScreenLimit.value()  ||
                    P->Position.Y>Settings.WindowWidth+Game.ScreenLimit.value()
                )
            ) ||
            (
                P->Rewind &&
                abs(P->Position.X-P->Origin.X)<=P->Speed &&  
                abs(P->Position.Y-P->Origin.Y)<=P->Speed
            )
        ) {
            DestroyProjectile(i);
            continue;
        }

        std::vector<Sprite*> HSprites = {&PlayerHeart, &Shokam};
        for (int h=0;h<(int)HSprites.size();h++){
            if (Game.Ending.has_value()) break;
            auto S = HSprites.at(h);
            if (
                !Game.isTransitioning &&
                P->Position.X<=S->Position.X+S->Width &&
                P->Position.Y<=S->Position.Y+S->Height &&
                P->Position.X+P->Width>=S->Position.X &&
                P->Position.Y+P->Height>=S->Position.Y
            ){
                if (P->FromPlayer==h && S->Health>0){
                    DestroyProjectile(i);
                    PlayAudio("assets\\audio\\" + std::string((h ? "shokam":"player")) + "\\damage.wav");
                    S->Health--;

                    if (h==1){
                        ShokamHealthBar.Width = (int)((float)Shokam.Health/(float)Settings.ShokamHealth*Settings.WindowWidth*0.8);
                        if (ISPHASE2 && !Game.isTransitioning && Shokam.Image!="assets\\entities\\shokam2.png") Transition();
                    }
                    else
                        Hearts.pop_back();
                }
                if (S->Health<=0) return EndGame(h);
                continue;
            }
        }

        if (
            !Game.isTransitioning &&
            !Game.Ending.has_value() && 
            P->ExplodeTime.has_value() && 
            P->ElapsedTime>=P->ExplodeTime.value()
        ){
            P->ExplodeFunction();
            DestroyProjectile(i);
        }
    }

    Player.Velocity.X = Player.Velocity.Y = 0;
    if ((!Game.Ending.has_value() || Game.Ending.value()==1) && Game.Running){
        if (Keys.W && Player.Position.Y>0) Player.Velocity.Y = -Player.Speed; 
        if (Keys.A && Player.Position.X>0) Player.Velocity.X = -Player.Speed;
        if (Keys.S && Player.Position.Y<Settings.WindowWidth-Player.Width) Player.Velocity.Y = Player.Speed; 
        if (Keys.D && Player.Position.X<Settings.WindowWidth-Player.Height) Player.Velocity.X = Player.Speed;
    }

    RENDERSCREEN;
}

int main(int argc, char* argv[]){
    SEEDRAND;

    TTF_Init();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    SDL_Surface* Icon = IMG_Load("assets\\icon.png");
    SDL_SetWindowIcon(Window, Icon);
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    Uint32 FrameStart;
    int FrameTime;
    SDL_Event event;

    LoadAssets();
    EndScreen.Opacity = Settings.StartScreenOpacity;
    SetStartScreen();
    
    while (1){
        FrameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)){
            SDL_Keysym keysm = event.key.keysym;
            SDL_Keycode keycode = keysm.sym;

            if (event.type==SDL_QUIT){
                Close();
                return 0;
            }
            else if (
                event.type==SDL_KEYUP && 
                keycode==SDLK_SPACE &&
                !Game.isTransitioning &&
                !Game.Ending.has_value() &&
                !(EndScreen.Opacity>=Settings.EndScreenOpacity || EndScreen.Opacity==Settings.StartScreenOpacity)
            ) {
                std::function<void()> Funcs[] = {[](){Mix_PauseMusic();}, [](){Mix_ResumeMusic();}};
                Funcs[Game.isPaused]();
                Game.isPaused = !Game.isPaused;
                if (!Game.isPaused) EndScreen.Opacity = 0;
            }
            else if (event.type==SDL_KEYDOWN || event.type==SDL_KEYUP){
                switch (keycode){
                    case SDLK_w:
                    case SDLK_UP:
                        if (!Game.Running && event.type==SDL_KEYUP) ChangeSelect(-1);
                        Keys.W = event.type==SDL_KEYDOWN;
                    break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (!Game.Running) {
                            PlayAudio("assets\\audio\\misc\\toggle.wav");
                            SetStartScreen();
                        }
                        Keys.A = event.type==SDL_KEYDOWN;
                    break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (!Game.Running && event.type==SDL_KEYUP) ChangeSelect(1);
                        Keys.S = event.type==SDL_KEYDOWN;
                    break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        Keys.D = event.type==SDL_KEYDOWN;
                    break;
                    case SDLK_RETURN:
                        if (
                            (EndScreen.Opacity>=Settings.EndScreenOpacity || EndScreen.Opacity==Settings.StartScreenOpacity) && 
                            event.type==SDL_KEYUP
                        ){
                            if (Game.Running) {
                                EndScreen.Opacity = Settings.StartScreenOpacity;
                                Game.Running = false;
                                Mix_HaltMusic();
                                SetStartScreen();
                            }
                            else {
                                PlayAudio("assets\\audio\\misc\\" + std::string(!Game.SelectState ? "play":"toggle")+".wav");
                                ChooseSelect();
                            }
                        }
                    break;
                }
            }
        }
        GameLoop();

        FrameTime = SDL_GetTicks()-FrameStart;
        if (Settings.FrameDelay>FrameTime)
            SDL_Delay(Settings.FrameDelay-FrameTime);
    }
    return 0;
}