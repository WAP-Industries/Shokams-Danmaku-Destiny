Sprite EndScreen(
    "assets\\gui\\mask.png",
    0,0,
    Settings.WindowWidth, Settings.WindowWidth
);

Sprite ScreenBackground(
    "assets\\gui\\startscreen.png",
    0,0,
    Settings.WindowWidth, Settings.WindowWidth
);

TextObject TopText(
    "", 
    0,0,
    50
);
TextObject BottomText(
    "", 
    0, Settings.WindowWidth*3.5/5,
    30
);
TextObject ScoreText(
    "",
    0,0,
    40
);
TextObject InstructText(
    "", 
    10,0,
    25
);


Sprite ShokamHealthBar(
    "assets\\gui\\healthbar.png",
    0,10,
    Settings.WindowWidth*0.8, Settings.WindowWidth/40
);

TextObject SuperAttackName(
    "", 
    100,100,
    30,
    0.5
);

Sprite SuperAttackMask = EndScreen;

TextObject ShokamNamebar(
    "",
    10, ShokamHealthBar.Height+15,
    20
);


std::pair<int, int> GetTextSize(std::string Text, int FontSize){
    TTF_Font* Font = TTF_OpenFont(Settings.GameFont, FontSize);
    
    int width, height;
    TTF_SizeText(Font, Text.c_str(), &width, &height);

    TTF_CloseFont(Font);
    return std::make_pair(width, height);
}