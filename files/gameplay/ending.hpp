void EndDialogue(std::string Text, std::string Song, bool died){
    Mix_FadeOutMusic(2000);
    
    while ((int)Sprites.size()>Game.StartSprites){DELETESPRITE;};
    Threading::RunAsync(
        [Song](){
            Threading::Delay(3000);
            EndScreen.FadeIn(0.05, 50, Settings.EndScreenOpacity);
            Game.isEndScreen = true;    
            PlayMusic(Song);
            for (int i=Projectiles.size()-1;i>=0;i--) DestroyProjectile(i);
        }
    );
    TopText.Text = Text, BottomText.Text = INSTRUCT("ENTER", "restart");

    if (died) return;
    ScoreText.Text = "Time: "+Timer.Display.Text;
    int w, h; std::tie(w, h) = GetTextSize(TopText.Text, TopText.FontSize);
    ScoreText.Position.Y = (Settings.WindowWidth-h)/2+h;
}

void Victory(bool died){
    PlayAudio("assets\\audio\\ending\\victory.wav");
    ShrinkProjectiles();
    EndDialogue("You defeated Shokam Chai!", "win", died);
}

void Death(bool died){
    PlayAudio("assets\\audio\\ending\\death.wav");
    EndDialogue("You died!", "lose", died);
}

void EndGame(int Ending){
    Game.Ending = Ending;

    std::pair<std::function<void(bool)>, Sprite*> Endings[] = {
        {Death, &Player},
        {Victory, &Shokam}
    };
    Endings[Ending].first(!Ending);

    auto S = Endings[Ending].second;
    int X = RandInt(0+Settings.WindowWidth,Settings.WindowWidth),
        Y = !Ending*Settings.WindowWidth+(Ending ? -1:1)*(S->Height+20);
    Threading::RunAsync(
        [S, X, Y](){
            S->RotationRate = 0.2;
            S->Rotation = M_PI/180;
            S->Travel(X, Y);
        }
    );
}
