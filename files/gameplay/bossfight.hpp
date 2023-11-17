void ShrinkProjectiles(){
    Threading::RunAsync(
        [](){
            int rate = 5;
            while(
                !!std::count_if(Projectiles.begin(), Projectiles.end(), [](Projectile* p){return p->Width>0;})
            ){
                if (!Game.Ending.has_value() && !Game.isTransitioning) return;

                for (auto P: Projectiles){
                    if (P->FromPlayer || P->Width<=0) continue;
                    P->Width-=rate; P->Height-=rate;
                    P->Position.X+=rate/2; P->Position.Y+=rate/2;
                }
                Threading::Delay(50);
            }
            for (int i=Projectiles.size()-1;i>=0;i--)
                DestroyProjectile(i);
        }
    );
}

void Transition(){
    Mix_FadeOutMusic(2000);
    Game.isTransitioning = true;
    PlayAudio("assets\\audio\\misc\\transition1.wav");
    ShrinkProjectiles();
    Threading::RunAsync(
        [](){
            Threading::Delay(500);
            for (int i=0;;i++){
                if (EndScreen.Opacity>=1) break;
                Shokam.Opacity = (i%2 ? 0.5:1);
                Threading::Delay(300);
            }
            Shokam.Opacity = 1;
        }
    );
    Threading::RunAsync(
        [](){
            Threading::Delay(2000);
            EndScreen.FadeIn(0.05, 50);
            Threading::Delay(1000);
            
            Shokam.Image = "assets\\entities\\shokam2.png";
            ShokamNamebar.Text = Settings.ShokamName.second;
            EndScreen.FadeOut(0.05, 50);
            PlayAudio("assets\\audio\\misc\\transition2.wav");
            Threading::Delay(2000);
            Mix_HaltMusic();
            PlayMusic("phase2");
            Game.isTransitioning = false;
        }
    );
}

void SuperAttackIntro(std::string Name){
    SuperAttackName.Text = Name;
    SuperAttackName.Opacity = 0;
    SuperAttackName.FontSize = 40;
    SuperAttackName.Speed = 20;

    int w, h;
    std::tie(w, h) = GetTextSize(SuperAttackName.Text, SuperAttackName.FontSize);
    SuperAttackName.Position.X = (Settings.WindowWidth-w)/2;
    SuperAttackName.Position.Y = (Settings.WindowWidth-h)/2;

    while (SuperAttackName.Opacity<1){
        SuperAttackName.Opacity+=0.1;
        if (SuperAttackMask.Opacity<0.5) 
            SuperAttackMask.Opacity+=0.1;
        Threading::Delay(50);
    }
    SuperAttackMask.Opacity = 0;

    int fs = 25;
    Threading::RunAsync(
        [fs](){
            int width, height;
            std::tie(width, height) = GetTextSize(SuperAttackName.Text, fs);
            SuperAttackName.Travel(
                Settings.WindowWidth-width-10, 
                ShokamHealthBar.Position.Y+ShokamHealthBar.Height+15
            );
        }
    );
    Threading::RunAsync(
        [fs](){
            while (SuperAttackName.FontSize>fs){
                SuperAttackName.FontSize-=5;
                Threading::Delay(20);
            }
        }
    );
}

std::pair<bool, Attack> ChooseAttack(){
    SEEDRAND;
    auto Vec = Attacks[ISPHASE2];
    
    Attack Atk;
    do {
        Atk = Vec.at(RandInt(0, Vec.size()-1));
    } while(Game.LastAttack.has_value() && Atk.Name==Game.LastAttack.value());

    return std::make_pair(Atk.isSuper, Atk);
}

void StartBossFight(){
    Threading::RunAsync(
        [](){
            Threading::Delay(3000);
            while (1){
                CHECKEND;

                auto AtkInfo = ChooseAttack();
                bool isSuper = AtkInfo.first;
                Game.CurrentAttack = AtkInfo.second.Function;
                Game.LastAttack = AtkInfo.second.Name;

                while (!!std::count_if(Projectiles.begin(), Projectiles.end(), [](Projectile* P){return !P->FromPlayer;}));
                if (isSuper) 
                    SuperAttackIntro(AtkInfo.second.Name);
                Game.CurrentAttack.value()();
                while (Game.CurrentAttack.has_value()) Threading::Delay(1);
                Threading::Delay((isSuper ? 4:2)*1000);
            }
        }
    );
}