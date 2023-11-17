#define PIRad 2*M_PI
#define SHOKAMCENTER Shokam.Position.X+(Shokam.Width-width)/2, Shokam.Position.Y+(Shokam.Height-width)/2
#define PCENTER(P) P->Position.X+(P->Width-width)/2, P->Position.Y+(P->Height-width)/2 
#define RANDANGLE std::rand()/(float)RAND_MAX*PIRad
#define DELETESPRITE delete Sprites.at(Sprites.size()-1); Sprites.pop_back()
#define SAFEDELETESPRITE CHECKATKEND; DELETESPRITE;
#define CLEARATTACK Game.CurrentAttack = std::nullopt; SuperAttackName.Text = ""; Game.ScreenLimit = 0; 
#define INITPARANG auto P = new Projectile( \
                        "12", \
                        PCENTER(S), \
                        width, height, \
                        3, 0 \
                    ); \
                    P->RotationRate = 0.1; \
                    P->Rotation = i*P->RotationRate+0.05;
#define RESUMETIME  SAFEDELETESPRITE; \
                    Threading::RunAsync( \
                        [](){ \
                            Threading::Delay(1000); \
                            CHECKATKEND; \
                            PlayAudio("assets\\audio\\misc\\resume.wav"); \
                            Threading::Delay(1500); \
                            std::for_each(Projectiles.begin(), Projectiles.end(), [](Projectile* p){p->FreezeTime = std::nullopt;}); \
                            CLEARATTACK; \
                        } \
                    );

void SetRebound(bool r){
    std::for_each(
        Projectiles.begin(), Projectiles.end(), 
        [r](Projectile* p){
            if (!p->FromPlayer) p->Rebound = r;
        }
    );
}

void SetRewind(bool r){
    std::for_each(
        Projectiles.begin(), Projectiles.end(), 
        [r](Projectile* p){
            if (!p->FromPlayer) {
                p->Rewind = r;
                if (r) p->Angle+=M_PI;
            }
        }
    );
}

std::pair<float, float> SpawnCircle(int x, int y, int radius){
    float angle = RANDANGLE;
    Game.ScreenLimit = radius;
    return std::make_pair(
        x+radius*(float)(cos(angle)),
        y+radius*(float)(sin(angle))
    );
}

void MoveBoss(std::optional<float> X=std::nullopt, std::optional<float> Y=std::nullopt){
    CHECKATKEND;
    SEEDRAND;

    int Count = std::count_if(
        std::filesystem::directory_iterator("assets\\audio\\attack"),
        std::filesystem::directory_iterator(), 
        [](const std::filesystem::directory_entry& entry){
            return entry.is_regular_file();
        }
    );

    int h = (int)(Count/2);
    PlayAudio("assets\\audio\\attack\\"+std::to_string(RandInt(1, h)+((ISPHASE2)*h))+".wav");
    Shokam.Travel(
        X.has_value() ? X.value() : RandInt(0, Settings.WindowWidth-Shokam.Width),
        Y.has_value() ? Y.value() : RandInt(0, Settings.WindowWidth/2-Shokam.Height)
    );
    Threading::Delay(1000);
}

void BurstAttack(
    std::string Image, 
    float X, float Y, float StartAngle, int BurstSize, int Radius, 
    std::variant<int, std::pair<int, int>> Width, int Speed){
    CHECKATKEND;

    float step = PIRad/BurstSize;

    for (int j=0;j<BurstSize;j++){
        float angle = j*step+StartAngle;
        int w;
        if (std::holds_alternative<int>(Width))
            w = std::get<int>(Width);
        else{
            auto p = std::get<std::pair<int, int>>(Width);
            w = RandInt(p.first, p.second);
        }

        Projectiles.push_back(
            new Projectile(
                Image,
                X+sin(angle)*Radius, Y+cos(angle)*Radius,
                w, w,
                Speed, angle
            )
        );
    }
}

void PierceAttack(
    std::string Image, 
    int BurstSize, int BurstLength, 
    float X, float Y, 
    int Speed, int Delay){
    for (int i=0;i<BurstLength;i++){
        CHECKATKEND;
        PlayAudio("assets\\audio\\projectile\\9.wav");
        BurstAttack(
            Image,
            X, Y, 0,
            BurstSize, 0,
            30, Speed
        );
        Threading::Delay(Delay);
    }
}

void FanAttack(
    std::string Image, 
    float Angle, int BurstSize, 
    float X, float Y, 
    int Radius, int Width, int Speed, 
    bool isWeapon){
    for (int j=0;j<BurstSize;j++){
        CHECKATKEND;
        Projectiles.push_back(
            new Projectile(
                Image,
                X+Radius*(float)(cos(Angle)),
                Y+Radius*(float)(sin(Angle)),
                Width, Width,
                Speed, (isWeapon ? -1:1)*Angle+j*M_PI/BurstSize
            )
        );
    }
}

void Attack1(){    
    Threading::RunAsync(
        [](){
            MoveBoss();
            int width = 30;
            for (int i=0;i<5;i++){
                CHECKATKEND;
                float sx, sy;
                std::tie(sx, sy) = SpawnCircle(SHOKAMCENTER, 50);
                PlayAudio("assets\\audio\\projectile\\2.wav");
                BurstAttack(
                    "5",
                    sx, sy, 0,
                    20,
                    50,
                    width, 3
                );
                Threading::Delay(700);
            }
            CLEARATTACK;
        }
    );
}

void Attack2(){
    Threading::RunAsync(
        [](){
            MoveBoss();
            int width = 30;
            for (int i=0;i<3;i++){    
                CHECKATKEND;
                float sx, sy;
                std::tie(sx, sy) = SpawnCircle(SHOKAMCENTER, 100);
                PlayAudio("assets\\audio\\projectile\\1.wav");
                BurstAttack(
                    "1",
                    sx, sy, 0,
                    15, 0,
                    width, 2
                );
                Threading::Delay(200);
            }

            Threading::Delay(2500);
            PierceAttack("2", 20,5,SHOKAMCENTER, 10, 50);
            Threading::Delay(2000);
            CLEARATTACK;
        }
    );
}

void Attack3(){
    Threading::RunAsync(
        [](){
            MoveBoss();
            int width = 30;
            for (int i=0;i<20;i++){
                CHECKATKEND;
                float sx, sy;
                std::tie(sx, sy) = SpawnCircle(SHOKAMCENTER, RandInt(30,50));
                PlayAudio("assets\\audio\\projectile\\10.wav");
                BurstAttack(
                    "2",
                    sx, sy,
                    RANDANGLE, 12, 0,
                    width, 2
                );
                Threading::Delay(200);
            }
            Threading::Delay(3000);
            CLEARATTACK;
        }
    );
}

void Attack4(){
    Threading::RunAsync(
        [](){
            MoveBoss();
            int width = 50;
            for (int i=0;i<30;i++){
                CHECKATKEND;
                PlayAudio("assets\\audio\\projectile\\11.wav");
                BurstAttack(
                    "1",
                    SHOKAMCENTER, i*0.25,
                    4, 70,
                    width, 5
                );
                Threading::Delay(200);
            }
            CLEARATTACK;
        }
    );
}

void Attack5(){
    Threading::RunAsync(
        [](){
            MoveBoss(Shokam.Position.X, Shokam.Position.Y);

            int width = 150;    
            for (int i=0;i<10;i++){
                CHECKATKEND;

                float angle = (RandInt(30,60))*M_PI/180;
                auto P = new Projectile(
                    "6",
                    -width, -width, width, width,
                    RandInt(3,5), angle
                ); 
                P->RotationRate = 0.1;
                P->Rotation = RANDANGLE;
                P->ExplodeTime = RandInt(70,100);
                P->ExplodeFunction = [P](){
                    PlayAudio("assets\\audio\\projectile\\7.wav");
                    BurstAttack(
                        "4",
                        P->Position.X+P->Width/2, P->Position.Y+P->Width/2, 0,
                        RandInt(10,15), 50,
                        std::make_pair(20,70), 4
                    );
                };

                Projectiles.push_back(P);
                Threading::Delay(RandInt(500,800));
            }
            CLEARATTACK;
        }
    );
}


void Attack6(){
    Threading::RunAsync(
        [](){
            SEEDRAND;
            int x = RandInt(0, 1)*(Settings.WindowWidth-Shokam.Width);
            MoveBoss(x);

            Sprite* Bag = new Sprite(
                "assets\\entities\\bag.png", 
                Shokam.Position.X+(!x ? 1:-1)*Shokam.Width,
                0,
                Shokam.Width*0.8, Shokam.Height*0.8
            );
            Bag->Position.Y = Shokam.Position.Y+Shokam.Height-Bag->Height;
            Bag->Speed = 20;
            Sprites.push_back(Bag);
            Threading::Delay(1000);
            
            Game.ScreenLimit = 150;
            Bag->Travel(Bag->Position.X, -Bag->Height-Bag->Speed);
            Threading::Delay(1000);
            PlayAudio("assets\\audio\\projectile\\14.wav");
            int width = 15;
            for (int i=0;i<10;i++){
                CHECKATKEND;
                BurstAttack(
                    "13",
                    PCENTER(Bag),
                    (i%2 ? 0.3:0), 30, 0,
                    width, 10
                );
                Threading::Delay(100);
            }
            
            Game.ScreenLimit = 0;
            DELETESPRITE;
            CLEARATTACK;
        }
    );
}

void Attack7(){
    Threading::RunAsync(
        [](){
            MoveBoss((Settings.WindowWidth-Shokam.Width)/2, 10);
            
            int width = 100, height = 30;
            Sprite* Broom = new Sprite(
                "assets\\entities\\broom.png",
                Shokam.Position.X+Shokam.Width-width/2,
                Shokam.Position.Y+Shokam.Height-height/2,
                width, height
            );
            Sprites.push_back(Broom);

            int pw = 25;
            Threading::Delay(1000);
            Broom->Rotation = 90*M_PI/180;
            for (int i=0;i<5;i++){
                PlayAudio("assets\\audio\\projectile\\2.wav");
                for (int j=0;j<5;j++){
                    FanAttack(
                        "5",
                        Broom->Rotation,
                        4,
                        Broom->Position.X+(Broom->Width-pw)/2,
                        Broom->Position.Y+(Broom->Height-pw)/2,
                        Broom->Width/2,
                        25, 7, true
                    );
                    SetRebound(true);
                    Threading::Delay(50);
                }
                Broom->Rotation+=i*0.2;
                Threading::Delay(200);
            }
            Threading::RunAsync(
                [](){
                    Threading::Delay(6000);
                    SetRebound(false);
                }
            );
            Threading::Delay(1000);
            
            int pn = 15;
            for (int j=0;j<5;j++){
                PlayAudio("assets\\audio\\projectile\\2.wav");
                Broom->Rotation = (j%2 ? 0.25:0);
                for (int i=0;i<pn;i++){
                    CHECKATKEND;
                    Broom->Rotation+=PIRad/pn;
                    auto P = new Projectile(
                        "5",
                        Broom->Position.X+(Broom->Width-pw)/2+Broom->Width/2*(float)(sin(Broom->Rotation)),
                        Broom->Position.Y+(Broom->Height-pw)/2+Broom->Width/2*(float)(cos(Broom->Rotation)),
                        pw, pw,
                        10-j, Broom->Rotation
                    );
                    P->FreezeTime = 10+j*0.5;
                    Projectiles.push_back(P);
                    Threading::Delay(20);
                }
            }
            RESUMETIME;
        }
    );
}

void Attack8(){
    Threading::RunAsync(
        [](){
            MoveBoss(Shokam.Position.X, Shokam.Position.Y);

            int width = 200;
            Sprite* Head = new Sprite(
                "assets\\entities\\head.png",
                Settings.WindowWidth+width, Settings.WindowWidth+width,
                width, width
            );
            Head->RenderFirst = true;
            Sprites.push_back(Head);
            Head->Speed = 10;
            Head->Travel(Settings.WindowWidth-Head->Width, Settings.WindowWidth-Head->Height);

            Threading::Delay(1000);
            for (int i=0;i<5;i++){
                PlayAudio("assets\\audio\\projectile\\7.wav");
                FanAttack(
                    "4", 
                    180*M_PI/180, 
                    10, 
                    Head->Position.X, Head->Position.Y+Head->Height, 
                    0, 50, 10, false
                );
                Threading::Delay(200);
            }

            Head->Travel(Settings.WindowWidth+width, Settings.WindowWidth+width);
            SAFEDELETESPRITE;
            CLEARATTACK;
        }
    );
}

void SuperAttack1(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\1.wav");
            SuperEffect2("2");
            ClearEffects(1300);

            std::vector<Coord> BurstPoints = {
                Coord(0,0),
                Coord(Settings.WindowWidth,0),
                Coord(0, Settings.WindowWidth),
                Coord(Settings.WindowWidth, Settings.WindowWidth)
            };

            for (int i=0;i<7;i++){
                for (auto Point: BurstPoints){
                    PierceAttack(
                        "3",
                        21,1,
                        Point.X, Point.Y,
                        15, 10
                    );
                }
            }
            CLEARATTACK;
        }
    );
}

void SuperAttack2(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\3.wav");
            SuperEffect1("1", 0);
            ClearEffects(1300);

            int width=150;
            for (int i=0;i<30;i++){
                CHECKATKEND;

                int rx = RandInt(0,1);
                if (!(i%3)) PlayAudio("assets\\audio\\projectile\\12.wav");
                Projectiles.push_back(
                    new Projectile(
                        "7",
                        rx*Settings.WindowWidth-!rx*width, RandInt(-Settings.WindowWidth, Settings.WindowWidth/2),
                        width, width,
                        RandInt(4, 7), RandInt(10,110)*M_PI/180*(rx ? -1:1)
                    )
                );
                Threading::Delay(250);
            }
            CLEARATTACK;
        }
    );
}

void SuperAttack3(){
    Threading::RunAsync(
        [](){
            Shokam.Travel((Settings.WindowWidth-Shokam.Width)/2, 0);

            PlayAudio("assets\\audio\\superattack\\2.wav");
            SuperEffect3("3", 0);
            ClearEffects(1000);

            std::vector<std::tuple<std::string, int, int, std::vector<int>, std::string, int, int, int, std::string>> GunInfo = {
                std::make_tuple(
                    "lasergun",
                    70,30,
                    std::vector<int>{1,15, 1},
                    "9",
                    50, 6, 
                    30, "4"
                ),
                std::make_tuple(
                    "shotgun",
                    100,30,
                    std::vector<int>{1, 25, 3},
                    "8",
                    20, 10, 
                    20, "6"
                ),
                std::make_tuple(
                    "ak47",
                    100,30,
                    std::vector<int>{2,40,12},
                    "10",
                    15, 15, 
                    5, "5"
                )
            };

            Sprite* Gun = new Sprite("", 0,0,0,0);
            Sprites.push_back(Gun);

            for (auto Info: GunInfo){
                Gun->Image = "assets\\entities\\"+std::get<0>(Info)+".png";
                Gun->Width = std::get<1>(Info), Gun->Height = std::get<2>(Info);
                Gun->Position.X = Shokam.Position.X+Shokam.Width-Gun->Width/2;
                Gun->Position.Y = Shokam.Position.Y+Shokam.Height-Gun->Height/2;

                float step = (float)(PIRad/std::get<3>(Info).at(1));
                bool isNormal = std::get<3>(Info).at(0)==1;

                for (int i=0;i<std::get<3>(Info).at(1);i++){
                    Gun->Rotation = i*step;
                    float angle = -Gun->Rotation+90*M_PI/180;

                    for (int j=0;j<(std::get<3>(Info).at(0)==2 ? std::get<3>(Info).at(2):1);j++){
                        float step = 0.5;
                        for (int k=0;k<(isNormal ? std::get<3>(Info).at(2):1);k++){
                            CHECKATKEND;

                            auto P = new Projectile(
                                std::get<4>(Info),
                                Gun->Position.X+(Gun->Width-std::get<5>(Info))/2+Gun->Width/2*(float)(cos(Gun->Rotation)),
                                Gun->Position.Y+(Gun->Height-std::get<5>(Info))/2+Gun->Width/2*(float)(sin(Gun->Rotation)),
                                std::get<5>(Info), std::get<5>(Info),
                                std::get<6>(Info), angle+(isNormal ? (k-(int)(std::get<3>(Info).at(2)/2))*step:0)
                            );
                            P->FreezeTime = std::get<7>(Info)+j*0.5;
                            Projectiles.push_back(P);
                        }
                        PlayAudio("assets\\audio\\projectile\\"+std::get<8>(Info)+".wav");
                    }
                    Threading::Delay(10);
                }
                Threading::Delay(500);
            }
            RESUMETIME;
        }
    );
}

void SLKAtk1(Sprite* S){
    CHECKATKEND;
    int width=60, height = width/3;
    int pn = 7;
    for (int j=0;j<2;j++){
        CHECKATKEND;
        PlayAudio("assets\\audio\\projectile\\13.wav");
        for (int i=0;i<pn;i++){
            INITPARANG;
            P->Angle = i*(PIRad/pn);
            Projectiles.push_back(P);
        }
        Threading::Delay(500);
    }
    SetRebound(true);
}

void SLKAtk2(Sprite* S){
    CHECKATKEND;
    int width = 60, height = width/3;
    int pn = 5;
    float inc = 0.4;

    for (int j=0;j<3;j++){
        CHECKATKEND;
        PlayAudio("assets\\audio\\projectile\\13.wav");
        for (int i=0;i<pn;i++){
            INITPARANG;
            P->Angle = (i-pn/2)*inc;
            Projectiles.push_back(P);
        }
        Threading::Delay(500);
    }
    SetRebound(true);
}

void SuperAttack4(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\4.wav");
            Shokam.Travel((Settings.WindowWidth-Shokam.Width)/2,100);

            SuperEffect1("6", 1);
            ClearEffects(1500);

            std::vector<std::tuple<std::string, std::function<void(Sprite*)>, std::string>> AtkInfo = {
                {"2", SLKAtk2, "2"},
                {"1", SLKAtk1, "1"}
            };

            int width = 100;
            auto SLK = new Sprite("", (Settings.WindowWidth-width)/2, 0, width, width); 
            SLK->Speed = 10;
            Sprites.push_back(SLK);
            for (auto Info: AtkInfo){
                SLK->Image = "assets\\entities\\slk"+std::get<0>(Info)+".png";
                SLK->Position.Y = -width;
                PlayAudio("assets\\audio\\misc\\slk"+std::get<2>(Info)+".wav");
                SLK->Travel(SLK->Position.X, Settings.WindowWidth/2-SLK->Height-SLK->Speed);
                std::get<1>(Info)(SLK);
                Threading::Delay(500);
                SLK->Travel(SLK->Position.X, -SLK->Height-SLK->Speed);
            }
            SAFEDELETESPRITE;

            Sprite* Gun = new Sprite(
                "assets\\entities\\ak47.png",
                0,0,
                100, 30
            );
            Gun->Position.X = Shokam.Position.X+Shokam.Width-Gun->Width/2;
            Gun->Position.Y = Shokam.Position.Y+Shokam.Height-Gun->Height/2;
            Sprites.push_back(Gun);

            int pw = 20;
            float inc = 0.2;
            for (int i=0;i<(int)(PIRad/inc);i++){
                CHECKATKEND;
                Projectiles.push_back(
                    new Projectile(
                        "8",
                        Gun->Position.X+(Gun->Width-pw)/2+Gun->Width/2*(float)(cos(Gun->Rotation)),
                        Gun->Position.Y+(Gun->Height-pw)/2+Gun->Width/2*(float)(sin(Gun->Rotation)),
                        pw, pw,
                        10, -Gun->Rotation
                    )
                );
                PlayAudio("assets\\audio\\projectile\\5.wav");
                Gun->Rotation+=inc;
                Threading::Delay(50);
            }
            SetRebound(false);

            SAFEDELETESPRITE;
            CLEARATTACK;
        }
    );
}

void SuperAttack5(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\5.wav");
            Shokam.Travel((Settings.WindowWidth-Shokam.Width)/2, 10);

            SuperEffect1("4", 0);
            ClearEffects(1500);

            int vh = 40;
            Sprite* Vape = new Sprite(
                "assets\\entities\\vape.png",
                Shokam.Position.X+Shokam.Width, Shokam.Position.Y+Shokam.Height-vh,
                vh/3, vh
            );
            Sprites.push_back(Vape);
            Threading::Delay(700);
            while (Vape->Opacity>0) {
                Vape->Opacity-=0.1;
                Threading::Delay(30);
            }
            SAFEDELETESPRITE;
            Threading::Delay(500);

            int pn = 10;
            float angle = 90*M_PI/180;
            int width = 20;
            for (int i=0;i<pn;i++){
                PlayAudio("assets\\audio\\projectile\\7.wav");
                Threading::RunAsync(
                    [width, angle](){
                        CHECKATKEND;
                        for (int k=0;k<5;k++){
                            for (int j=0;j<3;j++){
                                Projectiles.push_back(
                                    new Projectile(
                                        "1",
                                        Shokam.Position.X+(Shokam.Width-width)/2+sin(angle)*Shokam.Width/2,
                                        Shokam.Position.Y+(Shokam.Height-width)/2+cos(angle)*Shokam.Height/2,
                                        width, width,
                                        7, angle+j*0.15
                                    )
                                );
                            }
                            Threading::Delay(50);
                        }
                    }
                );
                angle-=(M_PI/180*180)/pn;
                Threading::Delay(150);
            }
            Threading::Delay(700);

            width = 15;
            PierceAttack(
                "7", 
                20, 5,
                SHOKAMCENTER,
                15, 50
            );
            Threading::Delay(200);

            for (int i=0;i<5;i++){
                PlayAudio("assets\\audio\\projectile\\7.wav");
                FanAttack(
                    "3",
                    -90*M_PI/180,
                    15, 
                    SHOKAMCENTER, 
                    0, 25, 10, false
                );
                Threading::Delay(50);
            }

            CLEARATTACK;
        }
    );
}

void SuperAttack6(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\6.wav");
            Shokam.Travel((Settings.WindowWidth-Shokam.Width)/2, 150);

            SuperEffect4("5");
            ClearEffects(1500);

            std::tuple<std::string, std::pair<int, int>, std::function<void(Sprite*)>> AtkInfo[] = {
                {
                    "1", {100,100},
                    [](Sprite* S){
                        int width = 20;
                        PlayAudio("assets\\audio\\projectile\\2.wav");
                        for (int i=0;i<3;i++){
                            BurstAttack(
                                "5",
                                PCENTER(S),
                                i*0.2, 15, 0,
                                width, 5
                            );
                            Threading::Delay(200);
                        }
                    }
                },
                {
                    "2", {-50,150},
                    [](Sprite* S){
                        int width = 20;
                        PierceAttack(
                            "7", 15, 10,
                            PCENTER(S),
                            10, 30
                        );
                    }
                },
                {
                    "3", {-200, -20},
                    [](Sprite* S){
                        PlayAudio("assets\\audio\\projectile\\7.wav");
                        int width = 50;
                        BurstAttack(
                            "4",
                            PCENTER(S),
                            0, 10, 0,
                            width, 5
                        );
                    }
                }
            };

            int width = 70, inc = 10;
            std::vector<Sprite*> Clones;
            for (auto Info: AtkInfo){
                Sprite* S = new Sprite(
                    "assets\\entities\\clone"+std::get<0>(Info)+".png",
                    SHOKAMCENTER,
                    0,0
                );
                S->Speed = 10, S->RenderFirst = true;
                Sprites.push_back(S);
                Clones.push_back(S);

                Threading::RunAsync(
                    [S, Info](){
                        S->Travel(S->Position.X+std::get<1>(Info).first, S->Position.Y+std::get<1>(Info).second);
                    }
                );
                Threading::RunAsync(
                    [width, S, inc](){
                        while (S->Width<width){
                            S->Width+=inc, S->Height+=inc;
                            Threading::Delay(20);
                        }
                    }
                );
            }
            
            int Timeout = 1000;
            Threading::Delay(Timeout);
            Game.ScreenLimit = std::nullopt;
            Game.isRewinding = true;
            for (int i=0;i<(int)Clones.size();i++){ 
                Clones.at(i)->Opacity = 0;
                Threading::RunAsync([AtkInfo, Clones, i](){std::get<2>(AtkInfo[i])(Clones.at(i));});
            }

            Threading::Delay(3000);
            PlayAudio("assets\\audio\\misc\\resume.wav");
            SetRewind(true);
            Game.ScreenLimit = std::nullopt;
            Game.isRewinding = false;
            
            while (std::count_if(Projectiles.begin(), Projectiles.end(), [](Projectile* p){return !p->FromPlayer;})>0);
            std::for_each(Clones.begin(), Clones.end(), [](Sprite* s){s->Opacity=1;});
            Threading::Delay(Timeout);
            for (auto C: Clones){
                Threading::RunAsync(
                    [C, width](){C->Travel(SHOKAMCENTER);}
                );
                Threading::RunAsync(
                    [C, inc](){
                        while (C->Width>0){
                            C->Width-=inc, C->Height-=inc;
                            Threading::Delay(20);
                        }
                    }
                );
            }
            while (std::count_if(Clones.begin(), Clones.end(), [](Sprite* c){return c->Width>0;})>0);
            Game.ScreenLimit = 0;
            for ([[maybe_unused]] auto C: Clones){SAFEDELETESPRITE;}
            CLEARATTACK;
        }
    );
}

void SuperAttack7(){
    Threading::RunAsync(
        [](){
            PlayAudio("assets\\audio\\superattack\\7.wav");
            SuperEffect3("7", 0);
            ClearEffects(1500);
            while(!!Effects.size());
            CHECKATKEND;

            Sprite* Domain = new Sprite(
                "assets\\entities\\domain.png",
                Settings.WindowWidth/2, Settings.WindowWidth/2,0,0
            );
            Sprite* Mask = new Sprite(
                "assets\\gui\\mask.png",
                0,0, Settings.WindowWidth, Settings.WindowWidth,
                0.5
            );
            Domain->RenderFirst = Mask->RenderFirst = true;
            Sprites.push_back(Domain), Sprites.push_back(Mask);

            int exp = 50;
            while (Domain->Width<Settings.WindowWidth){
                CHECKATKEND;
                Domain->Width+=exp, Domain->Height+=exp;
                Domain->Position.X-=exp/2, Domain->Position.Y-=exp/2;
                Threading::Delay(10);
            }

            int pw = 100, width = 0;
            std::vector<Coord> Pos = {
                Coord(0,0),
                Coord(Settings.WindowWidth-pw, Settings.WindowWidth/2),
                Coord(Settings.WindowWidth/2, Settings.WindowWidth-pw),
                Coord((Settings.WindowWidth-pw)/2, (Settings.WindowWidth-pw)/2)
            };
            for (auto P: Pos){
                CHECKATKEND;
                Sprite* Puiyi = new Sprite(
                    "assets\\entities\\puiyi.png",
                    P.X, P.Y, pw,pw
                );
                Puiyi->RenderFirst = true;
                Sprites.push_back(Puiyi);
                PierceAttack(
                    "5",
                    15, 5,
                    PCENTER(Puiyi),
                    10, 50
                );
                Threading::Delay(500);
                SAFEDELETESPRITE;
            }
            Threading::Delay(500);
            SAFEDELETESPRITE;
            Domain->FadeOut(0.2, 50);
            SAFEDELETESPRITE;
            CLEARATTACK;
        }
    );
}


struct Attack{
    std::function<void()> Function;
    bool isSuper=false;
    std::string Name;
};

std::vector<Attack> Attacks[]= {
    {
        Attack{Attack1, false, "atk1"}, 
        Attack{Attack2, false, "atk2"},
        Attack{Attack3, false, "atk3"},
        Attack{Attack4, false, "atk4"},
        Attack{Attack5, false, "atk5"},
        Attack{SuperAttack2, true, "Catastrophe Art [Meteor Rain]"},
        Attack{SuperAttack5, true, "[Poison Breath]"},
    },
    {
        Attack{Attack6, false, "atk6"}, 
        Attack{Attack7, false, "atk7"},
        Attack{Attack8, false, "atk8"},
        Attack{SuperAttack1, true, "Entrapment [Chaos Web]"},
        Attack{SuperAttack3, true, "Gunjutsu [Wheel of Time]"},
        Attack{SuperAttack4, true, "Divine Summon [Six-eyed Dragon]"},
        Attack{SuperAttack6, true, "Cycle of Agony [Bites the Dust]"},
        Attack{SuperAttack7, true, "Domain Expansion [Lust Hell]"}
    }
};