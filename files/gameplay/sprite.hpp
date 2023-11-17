struct Coord{
    float X, Y;
    Coord(float X, float Y): X(X), Y(Y){};
};


struct Sprite{
    std::string Image;
    Coord Position;
    int Width, Height;
    std::pair<int, int> ODim;
    float Opacity;

    int Speed;
    float Angle;
    Coord Velocity;

    float RotationRate = 0;
    float Rotation = 0;

    int Health;
    std::string Name;

    bool RenderFirst = false;

    Sprite(std::string Image, float X, float Y, int Width, int Height, float Opacity=1, std::string Name=""):
        Image(Image),
        Opacity(Opacity),
        Width(Width), Height(Height),
        Position(X, Y), Velocity(0,0), Name(Name){}

    virtual void Update(){
        this->Render();
        if (!Game.isPaused) this->Move();
    }

    virtual void Render(){
        if (
            this->Position.X+this->Width<=0 || 
            this->Position.Y+this->Height<=0 ||
            this->Position.X>Settings.WindowWidth || 
            this->Position.Y>Settings.WindowWidth ||
            this->Opacity<=0
        ) return;

        SDL_SetTextureBlendMode(Assets[this->Image], SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(Assets[this->Image], (int)(this->Opacity*255));
        SDL_FRect ImgBounds = {this->Position.X, this->Position.Y, (float)this->Width, (float)this->Height};
        if (this->Rotation>0){
            SDL_RenderCopyExF(Renderer, Assets[this->Image], nullptr, &ImgBounds, this->Rotation*180/M_PI, NULL, SDL_FLIP_NONE);
            if (!Game.isPaused) this->Rotation+=this->RotationRate;
        }
        else
            SDL_RenderCopyF(Renderer, Assets[this->Image], nullptr, &ImgBounds);
    }

    virtual void Move(){
        this->Position.X+=this->Velocity.X;
        this->Position.Y+=this->Velocity.Y;
    }

    void Travel(float X, float Y){
        CHECKPAUSE;
        float angle = atan2(Y-this->Position.Y, X-this->Position.X);
        Coord Velocity(
            cos(angle)*this->Speed,
            sin(angle)*this->Speed
        );
        while (
            abs(X-this->Position.X)>=this->Speed ||
            abs(Y-this->Position.Y)>=this->Speed
        ){
            if (Game.isEndScreen) return;
            this->Position.X+=Velocity.X;
            this->Position.Y+=Velocity.Y;
            Threading::Delay(10);
        }
    }

    void FadeIn(float Inc, int Delay, std::optional<int> Target=std::nullopt){
        while (this->Opacity<=(Target.has_value() ? Target.value():1)){
            this->Opacity+=Inc;
            Threading::Delay(Delay);
        }
    }

    void FadeOut(float Inc, int Delay){
        while (this->Opacity>=0){
            this->Opacity-=Inc;
            Threading::Delay(Delay);
        }
    }

    virtual ~Sprite(){}
};

struct Projectile: Sprite{
    int Speed;
    int ElapsedTime = 0;
    float Angle;
    bool Rebound = false, Rewind = false;
    Coord Origin;

    std::optional<int> ExplodeTime = std::nullopt, FreezeTime = std::nullopt; 
    std::function<void()> ExplodeFunction;

    bool FromPlayer;
    
    Projectile(std::string Image, float X, float Y, int Width, int Height, int Speed, float Angle, bool FromPlayer=false):
        Sprite(Image, X, Y, Width, Height), Speed(Speed), Angle(Angle), FromPlayer(FromPlayer), Origin(0,0){
            this->Image = "assets\\projectiles\\"+Image+".png";
            if (Game.isRewinding)
                this->Origin = Coord(X, Y);
        }

    virtual void Move() override{
        if (
            (this->FreezeTime.has_value() && this->ElapsedTime>=this->FreezeTime.value()) ||
            (Game.Ending.has_value() && !this->FromPlayer && Game.Ending.value())
        ) return;

        this->Velocity.X = (float)(sin(this->Angle))*this->Speed;
        this->Velocity.Y = (float)(cos(this->Angle))*this->Speed;

        this->Position.X+=this->Velocity.X;
        this->Position.Y+=this->Velocity.Y;
    }
};

struct TextObject: Sprite{
    std::string Text;
    int FontSize;
    float Opacity;
    bool XCentered=false, YCentered=false;

    TextObject(std::string Text, float X, float Y, int FontSize, float Opacity=1):
        Sprite("", X, Y, 0, 0), Text(Text), FontSize(FontSize), Opacity(Opacity){}

    virtual void Render() override{
        if (!this->Text.length() || this->Opacity<=0) return;

        TTF_Font* Font = TTF_OpenFont(Settings.GameFont, this->FontSize);
        SDL_Surface* Surface = 
            TTF_RenderText_Blended_Wrapped(
                Font, Text.c_str(), 
                SDL_Color{255, 255, 255, (Uint8)(this->Opacity*255)}, 
                Settings.WindowWidth
            );
        SDL_Texture* Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
        SDL_FRect Bounds = {this->Position.X, this->Position.Y, (float)Surface->w, (float)Surface->h};
        if (this->XCentered)
            Bounds.x = (Settings.WindowWidth-Bounds.w)/2;
        if (this->YCentered)
            Bounds.y = (Settings.WindowWidth-Bounds.h)/2;

        SDL_RenderCopyF(Renderer, Texture, NULL, &Bounds);

        SDL_DestroyTexture(Texture);
        SDL_FreeSurface(Surface);
        TTF_CloseFont(Font);
    }
};