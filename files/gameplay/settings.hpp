struct {
    const std::string Title = "Shokam's Danmaku Destiny";
    const int WindowWidth = 600;
    const char* GameFont = "font\\font.ttf";

    const int MaxFPS = 60;
    const int FrameDelay = 1000/MaxFPS;

    const int PlayerSpeed = 5;
    const int ShokamSpeed = 5;

    const int BgScroll = 10;

    const std::pair<int, int> PlayerProjectileSize = std::make_pair(40,40);
    const int PlayerProjectileSpeed = 10;
    const int PlayerFireRate = 100;
    const int PlayerHealth = 5;
    const int HitBoxWidth = 8;

    const int ShokamHealth = 900;
    const std::pair<std::string, std::string> ShokamName = {
        "The Stranger", 
        "Malevolent Starchild, Shokam"
    };

    const float StartScreenOpacity = 0.5;
    const float EndScreenOpacity = 1;
} Settings;