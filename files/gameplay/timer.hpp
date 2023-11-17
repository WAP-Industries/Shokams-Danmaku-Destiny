typedef long long unsigned int Ticks;

struct GameTimer{
    Ticks ElapsedTime;
    TextObject Display;

    GameTimer(): Display("", 0,0,0){
        this->Display.FontSize = 30;
    }

    void Update(){
        if (Game.Ending.has_value() || Game.isPaused) return;

        Ticks TotalTime = GETMILLI(this->ElapsedTime);
        std::vector<Ticks> Time = {
            (Ticks) TotalTime/60000, 
            (Ticks) (TotalTime/1000)%60, 
            (Ticks) TotalTime%1000/10
        };
        std::vector<std::string> FTime(Time.size());
        std::transform(
            Time.begin(), Time.end(), FTime.begin(), 
            [&Time](const Ticks& T){
                return (&T==&Time[0] ? "":(T<10 ? "0":""))+std::to_string(T);
            }
        );
        this->Display.Text = FTime[0]+":"+FTime[1]+"."+FTime[2];

        int w, h;
        std::tie(w, h) = GetTextSize(this->Display.Text, this->Display.FontSize);
        this->Display.Position.X = Settings.WindowWidth-w-10;
        this->Display.Position.Y = Settings.WindowWidth-h;
    }
};

GameTimer Timer;