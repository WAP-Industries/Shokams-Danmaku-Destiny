#define RIGHTXALIGN Settings.WindowWidth-ShokamEffect.Width
#define RIGHTYALIGN Settings.WindowWidth-ShokamEffect.Height
#define SETIMAGE ShokamEffect.Image = "assets\\effects\\"+src+".png"

void ClearEffects(int delay){
    Threading::Delay(delay);
    Threading::RunAsync(
        [](){
            int expand = 10;
            float ostep = 0.05;
            while (ShokamEffect.Opacity>0){
                CHECKPAUSE;
                ShokamEffect.Width+=expand, ShokamEffect.Height+=expand;
                ShokamEffect.Position.X-=expand/2, ShokamEffect.Position.Y-=expand/2;
                ShokamEffect.Opacity-=ostep;
                Threading::Delay(10);
            }
            ShokamEffect.Width = ShokamEffect.ODim.first, ShokamEffect.Height = ShokamEffect.ODim.second;
            Effects.clear();
        }
    );
}


void SuperEffect1(std::string src, int mode){
    SETIMAGE;
    ShokamEffect.Position.Y = (Settings.WindowWidth-ShokamEffect.Height)/2;
    ShokamEffect.Position.X = mode ? Settings.WindowWidth : -ShokamEffect.Width;
    ShokamEffect.Opacity = 0.9;
    Effects = {&ShokamEffect};

    Threading::RunAsync(
        [mode](){
            while ((mode ? (ShokamEffect.Position.X>Settings.WindowWidth-ShokamEffect.Width) : (ShokamEffect.Position.X<0))){
                CHECKPAUSE;
                ShokamEffect.Position.X+=20*(mode ? -1:1);
                Threading::Delay(5);
            }
        }
    );
}

void SuperEffect2(std::string src){
    SETIMAGE;
    ShokamEffect.Position.X = ShokamEffect.Position.Y = Settings.WindowWidth;
    ShokamEffect.Width = ShokamEffect.Height = 0;
    ShokamEffect.Opacity = 0.9;
    Effects = {&ShokamEffect};

    Threading::RunAsync(
        [](){
            float mul = 0;
            int deviation = 50;
            while (ShokamEffect.Height<Settings.WindowWidth*0.9){
                CHECKPAUSE;
                ShokamEffect.Width = ShokamEffect.ODim.first*mul;
                ShokamEffect.Height = ShokamEffect.ODim.second*mul;
                ShokamEffect.Position.X = RIGHTXALIGN+RandInt(0,deviation);
                ShokamEffect.Position.Y = RIGHTYALIGN;
                Threading::Delay(20);
                mul+=0.1;
            }
            ShokamEffect.Position.X = RIGHTXALIGN;
            ShokamEffect.Position.Y = RIGHTYALIGN;
        }
    );
}

void SuperEffect3(std::string src, int mode){
    SETIMAGE;
    ShokamEffect.Position.X = mode*(Settings.WindowWidth-ShokamEffect.Width);
    ShokamEffect.Position.Y = Settings.WindowWidth;
    ShokamEffect.Height = (int)(Settings.WindowWidth*0.8);
    ShokamEffect.Opacity = 0.9;
    Effects = {&ShokamEffect};

    Threading::RunAsync(
        [](){
            while (ShokamEffect.Position.Y+ShokamEffect.Height>Settings.WindowWidth-50){
                CHECKPAUSE;
                ShokamEffect.Position.Y-=25;
                Threading::Delay(3);
            }
        }
    );
}

void SuperEffect4(std::string src){
    SETIMAGE;
    ShokamEffect.Width = ShokamEffect.Height = Settings.WindowWidth*0.7;
    ShokamEffect.Position.X = (Settings.WindowWidth-ShokamEffect.Width)/2;
    ShokamEffect.Position.Y = (Settings.WindowWidth-ShokamEffect.Height)/2;
    ShokamEffect.Opacity = 0;
    Effects = {&ShokamEffect};

    Threading::RunAsync(
        [](){
            while (ShokamEffect.Opacity<0.6){
                CHECKPAUSE;
                ShokamEffect.Opacity+=0.1;
                Threading::Delay(20);
            }
        }
    );
}